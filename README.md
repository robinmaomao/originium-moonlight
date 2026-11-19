# originium-moonlight

Read-only fork of [moonlight-qt](https://github.com/moonlight-stream/moonlight-qt)
used by [Originium Stream](https://github.com/robinmaomao/originium-stream-webui).

## Policy

- **No pushes, no pull requests, no issues against upstream.** The `upstream`
  remote exists only for `git fetch` / `git rebase`.
- All work lives on the `originium` branch.
- Releases correspond 1:1 to source tags (GPL-3.0 compliance).

## Patches

| ID | Title | Capability | Status |
|---|---|---|---|
| 0001 | [Control channel v1](./patches/0001-control-channel.md) | `control-pipe` | implemented (state/stop) |
| 0002 | [Native child-window embedding](./patches/0002-native-embed.md) | `embed-parent` | design |

Official portable builds remain supported: the application falls back to CLI
arguments and borderless fullscreen when capabilities are absent.

## Build (Windows)

```powershell
git clone --recursive https://github.com/robinmaomao/originium-moonlight.git
cd originium-moonlight
# Qt 6.11+ MSVC, Visual Studio 2026 Build Tools
qmake6 moonlight-qt.pro
nmake
```

The portable folder requires `portable.dat` beside `moonlight.exe` (QSettings INI
mode). CI produces `Moonlight-Originium-x64.zip`.

## License

GPL-3.0-or-later. See [LICENSE](./LICENSE).
