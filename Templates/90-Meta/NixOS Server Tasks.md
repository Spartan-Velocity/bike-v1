---
type: reference
updated: 2026-10-17
tags: [meta, nixos, server]
---

# NixOS Server Tasks

For the Claude Code instance running on `nixos`. Written from the laptop (`arc`) side,
so everything here is what that machine needs but cannot see from here.

## Context

`nixos` exports `/srv/lab-files` over Samba. The laptop mounts it at `/mnt/lab-files`
with a kernel CIFS mount, and the Obsidian vault plus its git repo live in
`/srv/lab-files/vault-mm`. Reachable only over Tailscale (`100.65.170.20`), currently
a direct connection, ~27 ms.

## 1. Authorize the laptop's SSH key

SSH from `arc` currently fails on both publickey and password. The key exists already:

```
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAID8YcX6nqHpC1GcR4PT7OR7zxEhUA8rWwXWJDHwvob5M ex@arc
```

Declaratively, in `configuration.nix`:

```nix
users.users.ex.openssh.authorizedKeys.keys = [
  "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAID8YcX6nqHpC1GcR4PT7OR7zxEhUA8rWwXWJDHwvob5M ex@arc"
];
```

Then `sudo nixos-rebuild switch`. Verify from `arc`: `ssh ex@100.65.170.20 hostname`.

## 2. Fix the Samba inode collisions — the real bug

On 2026-10-16 the laptop's kernel logged, against this share:

```
CIFS: VFS: Inode number collision detected
CIFS: VFS: Autodisabling the use of server inode numbers on \\100.65.170.20\lab-files
CIFS: VFS: The server doesn't seem to support them properly or the files might be
           on different servers (DFS)
```

Samba is handing out inode numbers that are not unique across the share. The usual
cause is the share spanning more than one filesystem — a bind mount, a separate
subvolume, or an overlay under `/srv/lab-files`.

Diagnose:

```
findmnt -T /srv/lab-files
df -T /srv/lab-files
findmnt | grep srv
testparm -s 2>/dev/null | sed -n '/lab-files/,/^\[/p'
```

Standard fix — give Samba a consistent inode source for the share:

```
vfs objects = fileid
fileid:algorithm = fsname
```

On NixOS that goes in `services.samba.settings."lab-files"`.

Once this is fixed and verified, `noserverino` can be dropped from the client mount
(see below), which restores hardlink detection.

## 3. Optional: WebDAV for mobile sync

Verified working from the laptop against rclone 1.75.2 — the CORS preflight returns
the right headers and an authenticated `PROPFIND` returns `207`, which is what
Obsidian on iOS needs:

```
rclone serve webdav /srv/lab-files/vault-mm \
  --addr 0.0.0.0:8099 \
  --user <user> --pass <pass> \
  --allow-origin "app://obsidian.md"
```

`--allow-origin` takes a single origin. iPad and iPhone are both iOS so one value
covers them; confirm whether the installed Obsidian sends `app://obsidian.md` or the
older `capacitor://localhost`. Bind to the Tailscale interface rather than `0.0.0.0`
if you want it reachable only over the tailnet. Run it as a systemd service.

Pair it with the **Remotely Save** plugin on each device. Note it is file-level sync:
conflicts produce duplicate files rather than merges, and it keeps no history. git on
the laptop covers history; these two do not fight, because git's `.git` directory is
not something Obsidian sync touches.

## 4. Client-side mount, for reference

What `arc` currently uses, in `/etc/fstab`:

```
//100.65.170.20/lab-files /mnt/lab-files cifs
  credentials=/etc/samba/creds-labfiles,
  uid=1000,gid=1000,forceuid,forcegid,file_mode=0755,dir_mode=0755,
  vers=3.1.1,sec=ntlmssp,nounix,noserverino,mfsymlinks,mapposix,
  cache=strict,actimeo=30,echo_interval=10,
  soft,nofail,_netdev,x-systemd.automount,x-systemd.mount-timeout=20s,
  x-systemd.requires=tailscaled.service,x-systemd.after=tailscaled.service 0 0
```

Why these, briefly: `soft` because `hard` parks Electron in uninterruptible sleep when
the server stops answering; `file_mode=0755` because `0644` makes nothing on the share
executable, which silently breaks scripts, git hooks and test binaries; `noserverino`
because of the bug in section 2; no `idle-timeout`, because unmounting under a live
Obsidian is what produces 0-byte notes.

`actimeo=30` assumes **one writer**. If anything starts editing `/srv/lab-files`
directly on the server, say so — that assumption has to change.
