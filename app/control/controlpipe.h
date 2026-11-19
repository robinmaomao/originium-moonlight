#pragma once

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>

#include <functional>

/**
 * @brief Local named-pipe control channel used by Originium Stream.
 *
 * Frames are 4-byte little-endian length prefixes followed by UTF-8 JSON.
 * Supports `session.state` and `session.stop` in protocol v1.
 */
class ControlPipe : public QObject
{
    Q_OBJECT

public:
    explicit ControlPipe(QString name, QObject* parent = nullptr);

    bool start();

    void setStateProvider(std::function<QJsonObject()> provider);

signals:
    void stopRequested();

private slots:
    void onNewConnection();

private:
    void processBuffer(QLocalSocket* socket);
    void sendMessage(QLocalSocket* socket, const QJsonObject& message);

    QString m_Name;
    QLocalServer m_Server;
    std::function<QJsonObject()> m_StateProvider;
};
