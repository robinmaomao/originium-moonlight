#include "controlpipe.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

namespace {
    constexpr int MAX_FRAME_SIZE = 1024 * 1024;

    quint32 readLength(const QByteArray& buffer)
    {
        const auto* data = reinterpret_cast<const unsigned char*>(buffer.constData());
        return static_cast<quint32>(data[0])
               | (static_cast<quint32>(data[1]) << 8)
               | (static_cast<quint32>(data[2]) << 16)
               | (static_cast<quint32>(data[3]) << 24);
    }

    QByteArray frame(const QJsonObject& message)
    {
        const QByteArray payload = QJsonDocument(message).toJson(QJsonDocument::Compact);
        const auto length = static_cast<quint32>(payload.size());
        QByteArray output;
        output.reserve(4 + payload.size());
        output.append(static_cast<char>(length & 0xFF));
        output.append(static_cast<char>((length >> 8) & 0xFF));
        output.append(static_cast<char>((length >> 16) & 0xFF));
        output.append(static_cast<char>((length >> 24) & 0xFF));
        output.append(payload);
        return output;
    }
}  // namespace

ControlPipe::ControlPipe(QString name, QObject* parent)
    : QObject(parent),
      m_Name(std::move(name))
{
}

bool ControlPipe::start()
{
    QLocalServer::removeServer(m_Name);
    if (!m_Server.listen(m_Name)) {
        qWarning() << "ControlPipe: failed to listen on" << m_Name << m_Server.errorString();
        return false;
    }

    connect(&m_Server, &QLocalServer::newConnection, this, &ControlPipe::onNewConnection);
    qInfo() << "ControlPipe: listening on" << m_Name;
    return true;
}

void ControlPipe::setStateProvider(std::function<QJsonObject()> provider)
{
    m_StateProvider = std::move(provider);
}

void ControlPipe::onNewConnection()
{
    while (m_Server.hasPendingConnections()) {
        QLocalSocket* socket = m_Server.nextPendingConnection();
        if (!socket) {
            continue;
        }

        connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
            processBuffer(socket);
        });
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    }
}

void ControlPipe::processBuffer(QLocalSocket* socket)
{
    QByteArray buffer = socket->property("originiumBuffer").toByteArray();
    buffer.append(socket->readAll());

    while (buffer.size() >= 4) {
        const quint32 length = readLength(buffer);
        if (length > MAX_FRAME_SIZE) {
            socket->disconnectFromServer();
            return;
        }
        if (buffer.size() < static_cast<int>(4 + length)) {
            break;
        }

        const QByteArray payload = buffer.mid(4, static_cast<int>(length));
        buffer.remove(0, static_cast<int>(4 + length));

        QJsonParseError parseError {};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            QJsonObject error;
            error["v"] = 1;
            error["ok"] = false;
            error["error"] = "BAD_REQUEST";
            sendMessage(socket, error);
            continue;
        }

        const QJsonObject request = document.object();
        const QJsonValue id = request.value("id");
        const QString command = request.value("cmd").toString();

        QJsonObject response;
        response["v"] = 1;
        if (!id.isUndefined()) {
            response["id"] = id;
        }

        if (command == "session.state") {
            response["ok"] = true;
            response["result"] = m_StateProvider ? m_StateProvider() : QJsonObject {{"state", "idle"}};
        } else if (command == "session.stop") {
            emit stopRequested();
            response["ok"] = true;
            response["result"] = QJsonObject {{"stopping", true}};
        } else {
            response["ok"] = false;
            response["error"] = "UNSUPPORTED";
        }

        sendMessage(socket, response);
    }

    socket->setProperty("originiumBuffer", buffer);
}

void ControlPipe::sendMessage(QLocalSocket* socket, const QJsonObject& message)
{
    if (!socket || socket->state() != QLocalSocket::ConnectedState) {
        return;
    }
    socket->write(frame(message));
    socket->flush();
}
