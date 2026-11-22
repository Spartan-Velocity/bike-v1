# Embedded Workflow in Obsidian

All firmware work for this project can be done without leaving Obsidian. The vault root is also the git repo, and `Firmware/vcu/` is the PlatformIO project.

## Plugins doing the work (all already installed)

| Plugin                                        | Role                                                        |
| --------------------------------------------- | ----------------------------------------------------------- |
| **Terminal**                                  | Integrated terminal + one-click PlatformIO profiles (below) |
| **open-vscode** / **vscode-editor**           | Open a file/folder in VS Code, or edit code inline          |
| **code-runner**                               | Run snippets (host-side scratch work)                       |
| **obsidian-git**                              | Stage / commit / push without a shell                       |
| **mermaid-view**, **excalidraw**, **tikzjax** | Wiring diagrams, schematics, timing sketches                |
| **xlsx-viewer**, **csv-lite**                 | BOMs and logs (e.g. Orion BMS CSVs)                         |

## One-click PlatformIO commands

The Terminal plugin exposes each profile as a command. Open the Command Palette
(`Ctrl/Cmd-P`) and search **"Terminal: Open terminal"** — these now appear:

- **PIO: Build** → `pio run`
- **PIO: Upload** → `pio run -t upload`  (Teensy plugged in)
- **PIO: Monitor** → `pio device monitor -b 115200`
- **PIO: Clean** → `pio run -t clean`

Each opens an integrated terminal already `cd`'d into `Firmware/vcu/`, runs the
command, prints `[exit <code>]`, and drops to a shell so you can keep going.

### Bind hotkeys (recommended)
Settings → Hotkeys → search "Terminal" → assign keys to the four profile commands,
e.g. `Ctrl-Shift-B` for Build, `Ctrl-Shift-U` for Upload, `Ctrl-Shift-M` for Monitor.

## Typical loop

1. Edit `Firmware/vcu/src/*.cpp` (inline, or "Open in VS Code" for heavy work).
2. **PIO: Build** — fix errors in the terminal pane.
3. **PIO: Upload** — flash the Teensy.
4. **PIO: Monitor** — watch serial. `Ctrl-C` to exit.
5. Commit via the obsidian-git sidebar.

## Notes
- The config lives in `.obsidian/plugins/terminal/data.json` (backup: `data.json.bak`).
- **NixOS:** there is no `/bin/bash`; the shell is `/run/current-system/sw/bin/bash`.
  All profiles (and the plugin's default integrated profile) use that path — if
  a system rebuild changes it, update `executable` in each profile.
- If `pio` isn't found, the login shell isn't sourcing the Nix profile — the
  profiles use `bash -lc` so `~/.nix-profile/bin` is on PATH; adjust if your
  shell init differs.
- Teensy upload needs the device present and udev rules in place.
