# 0001 — Control channel v1

**Status:** design (not yet applied)

## Motivation

The application currently drives moonlight through CLI arguments and exit codes.
That cannot express live state, in-session actions or structured errors. A local
control channel removes CLI scraping and enables the HUD/overlay to be authoritative.

## Transport

- Windows named pipe: `\\.\pipe\originium-moonlight-{pid}`
- Framing: 4-byte little-endian length + UTF-8 JSON
- Lifetime: created on startup when `--control-pipe` is passed; removed on exit.

## Commands

```json
{ "v": 1, "id": "req-1", "cmd": "session.start", "params": { "host": "192.168.1.10", "app": "Desktop", "profile": { "width": 2560, "height": 1440, "fps": 120, "bitrateKbps": 80000 } } }
{ "v": 1, "id": "req-1", "ok": true, "result": { "sessionId": "s-1" } }
{ "v": 1, "id": "req-2", "cmd": "session.stop" }
{ "v": 1, "id": "req-3", "cmd": "session.state" }
```

Events (no `id`):

```json
{ "v": 1, "event": "session.state", "data": { "state": "streaming", "fps": 120, "bitrateKbps": 80000 } }
{ "v": 1, "event": "session.error", "data": { "code": "HOST_UNREACHABLE", "message": "…" } }
```

## Touched files

| File | Change |
|---|---|
| `app/cli/commandlineparser.cpp` | `--control-pipe` flag |
| `app/main.cpp` | create server, pass endpoint to `Session` |
| `app/streaming/session.cpp` | emit state transitions |
| `app/control/` | new pipe server + JSON-RPC dispatcher |
| `app/app.pro` | source list |

## Compatibility

Unknown commands return `UNSUPPORTED`; the app falls back to CLI mode when the
capability is absent.
