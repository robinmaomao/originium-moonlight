# 0001 — Control channel v1

**Status:** implemented (partial: `session.state` + `session.stop`)

## Motivation

The application currently drives moonlight through CLI arguments and exit codes.
That cannot express live state or in-session actions. A local control channel
removes CLI scraping and enables the HUD/overlay to be authoritative.

## Transport

- Windows named pipe: the `--control-pipe <name>` value is the pipe name
  (`\\.\pipe\<name>`).
- Framing: 4-byte little-endian length + UTF-8 JSON.
- Created on startup when `--control-pipe <name>` is passed.

## Commands

```json
{ "v": 1, "id": "req-1", "cmd": "session.state" }
{ "v": 1, "id": "req-1", "ok": true, "result": { "state": "streaming" } }

{ "v": 1, "id": "req-2", "cmd": "session.stop" }
{ "v": 1, "id": "req-2", "ok": true, "result": { "stopping": true } }
```

- `session.start` currently returns `{"ok":false,"error":"UNSUPPORTED"}`; the app
  starts sessions via CLI and uses the pipe for state/stop.
- Implementation: `app/control/controlpipe.{h,cpp}`, wired in `app/main.cpp`.

## Touched files

| File | Change |
|---|---|
| `app/cli/commandlineparser.cpp` | `--control-pipe` flag |
| `app/main.cpp` | create server, wire stop/state |
| `app/control/` | new pipe server + JSON dispatcher |
| `app/app.pro` | source list |

## Compatibility

Unknown commands return `UNSUPPORTED`; the app falls back to CLI mode when the
capability is absent.

