---
type: reference
updated: 2026-10-16
tags: [meta]
---

# Vault Conventions

How this vault is wired together, so future-you does not have to re-derive it.

## The idea

Every artifact — notes, schematics, code, drawings, exports — lives in this one tree.
Obsidian is the index over it. The app something is *edited* in does not matter; what
matters is that it is in the vault, in a format that can be linked and searched.

## Layout

| Folder | What goes there |
|---|---|
| `00-Inbox/` | Unsorted. Empty it periodically. |
| `10-Projects/` | One folder per project, with `Subsystems/`, `Hardware/`, `Firmware/`, `Diagrams/` |
| `20-Notes/` | Working notes not tied to one subsystem |
| `30-Reference/` | Datasheets, manuals, papers |
| `90-Meta/` | Templates, scripts, this file |

## Frontmatter

Every substantive note carries frontmatter so [[Subsystems]] can index it:

```yaml
type: subsystem | design-log | project | reference
project: Spartan Velocity
subsystem: VCU
status: draft | in-progress | validated | blocked
hardware: vcb-mk2
schematic: path/to/export.svg
firmware: path/to/source
updated: YYYY-MM-DD
tags: []
```

## Linking across tools

Both URI schemes are registered on `arc`, so these are clickable from inside Obsidian:

| To open | Write |
|---|---|
| A file in VS Code at a line | `vscode://file/mnt/lab-files/vault-mm/path/to/file.c:42` |
| A KiCad project | `file:///mnt/lab-files/vault-mm/path/to/board.kicad_pro` |
| Back into this vault from code | `obsidian://open?vault=vault-mm&file=<url-encoded path>` |

Note the missing slash: `vscode://file/mnt/...`, not `vscode://file//mnt/...`.

## Scripts

In `90-Meta/Scripts/`, with thin wrappers on `PATH` at `~/.local/bin` so they work
from anywhere (and fail with a clear message when the mount is down):

| Script | Does |
|---|---|
| `vault-sync` | Checks the mount is alive, commits, rebases, pushes. `-n` for a dry run. |
| `kicad-export` | Regenerates SVG / PDF / BOM from KiCad into `Hardware/exports/` |

Run `kicad-export` after changing a schematic, so notes embed something current.

## Storage

The vault lives on the `lab-files` Samba share on `nixos`, mounted at `/mnt/lab-files/vault-mm`
over Tailscale. Git is the sync mechanism between machines: the laptop writes to the
share, and phone and iPad reach the same content as clones of the GitHub repo.

**The link currently relays through DERP** — measured ~25 ms round trip and ~1.1 MB/s,
not a direct connection. Everything below follows from that.

## Rules that keep this from losing work

These are not style preferences. Each one maps to a specific failure.

1. **Enable the File Recovery core plugin.** Obsidian on a network drive has a
   documented failure where a disconnect-reconnect causes it to overwrite open notes
   with empty-editor contents, leaving 0-byte files. File Recovery keeps snapshots
   outside the vault, on local disk. This is the single most important item here.
2. **Close Obsidian before suspending the laptop** or before dropping Tailscale.
3. **Turn OFF Obsidian Git's auto-commit interval on every device.** Excalidraw files
   are rewritten whole on every save, at 1.6–2.2 MB each. Auto-commit every 10 minutes
   turns an afternoon of drawing into tens of MB of history, and within weeks the repo
   is too big for isomorphic-git to clone on the iPad. Commit deliberately instead.
4. **Never run the GVFS mount and the CIFS mount at the same time.** Two SMB clients
   with independent caches writing the same files invalidates the caching assumptions
   and can corrupt. If Nautilus has a `lab-files` bookmark, remove it.
5. **Push early and often.** Until the first push, the working tree and the git history
   both live on one disk on `nixos`. That is not a backup.
6. **Run `git gc` manually, with Obsidian closed, on a good link.** Automatic gc is
   disabled on purpose — a repack interrupted mid-rename over a flaky mount is bad.
7. **The mount uses `file_mode=0755`**, so everything on the share looks executable.
   That is deliberate: with `0644` nothing could run, which broke the vault's own
   scripts, git hooks, and `pio test` binaries alike. The cost is that you cannot tell
   from the mode bits which files are genuinely executable. git ignores this entirely
   because `core.fileMode=false`; to mark a file executable *in the repo*, use
   `git update-index --chmod=+x <path>`.
8. **`noserverino` is set.** Samba on `nixos` reports colliding inode numbers, which is
   what relocated a directory on 2026-10-16. The client no longer trusts the server's
   inode numbers, and `core.checkStat=minimal` keeps git from re-hashing the tree as a
   result. The real fix is server side: `vfs objects = fileid` with
   `fileid:algorithm = fsname` on the share.

## Worth fixing when you have time

Tailscale is relaying rather than connecting directly. Our side looks fine
(`netcheck` reports UDP works and the NAT mapping is stable), so the block is probably
on the `nixos` side — UDP 41641 outbound, or no UPnP/NAT-PMP on that router. A direct
path would take this from 1.1 MB/s to local-network speed and make every item above
less painful.
