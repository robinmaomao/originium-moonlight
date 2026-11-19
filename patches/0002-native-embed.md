# 0002 — Native child-window embedding

**Status:** design (not yet applied)

## Motivation

Reparenting moonlight's SDL window from outside is fragile across display mode
changes and DPI transitions. Embedding must be implemented inside the client.

## Change

- New CLI flag: `--embed-parent <HWND>` (decimal/hex).
- On session start, create the video window with `SDL_CreateWindowFrom` semantics
  or reparent only once before the first frame, then handle
  `WM_DPICHANGED` / display changes internally.
- On stop, restore the original style and parent.
- Fallback: when the flag is absent, behavior is unchanged.

## Touched files

| File | Change |
|---|---|
| `app/cli/commandlineparser.cpp` | `--embed-parent` flag |
| `app/streaming/session.cpp` | pass parent HWND to renderer |
| `app/streaming/video/` | renderer child-window initialization |
| `app/app.pro` | source list |

## Capability

`embed-parent` in `components.lock.json`.

## Compatibility

Official builds ignore the app's embedding request; the app uses borderless
fullscreen plus HUD instead.
