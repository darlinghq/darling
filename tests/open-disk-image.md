<!-- Copyright (C) 2026 Lubos Dolezel -->

# Opening a disk image from the Linux desktop

`open-disk-image.sh` follows a disk image from the Linux desktop to Finder and back out: the desktop entry that
Darling installs (`share/applications/org.darlinghq.Darling.DiskImageMounter.desktop`, `Exec=... open %F`) is
launched on `fixtures/styled-sla-test.dmg` the way a file manager launches it; DiskImageMounter shows the image's
licence agreement, mounts it after Agree and asks LaunchServices to open the volume; Finder shows its styled window;
an application is dragged out of it; Command-E ejects it.

It runs on the host and calls `darling` from `PATH`, so the caller picks the launcher mode. It needs what
`finder.sh` needs (see its header and `finder.md`), plus `gio` and `setsid`. With the private install of the Finder
roadmap:

```sh
. build/tmp/helpers/mode.sh rootless   # or nonroot, root
DISPLAY=:97 DARLING_APPKIT_BACKEND=x11 bash tests/open-disk-image.sh
```

## Cases

| Case | What it shows |
|------|---------------|
| desktop entry | the installed entry's `Exec` names the installed launcher: `"<prefix>/bin/darling" open %F` |
| cold start | with the container stopped, the entry launched with `setsid` (no terminal, stdin `/dev/null`) on an image at `Dir With Späce/Ünï & 'q' $x.dmg` brings up the licence window within 180 s |
| agreement | Return (Agree) mounts the volume "Styled Test" and Finder opens its window |
| window layout | the window is styled from the image's `.DS_Store`: 640 points wide, 400 high plus at most a title bar, the background picture's blue in the bottom left corner |
| copy | Styled App.app dragged from that window into a folder opened with `darling open` is copied there whole |
| eject | Command-E in the volume's window closes it and unmounts the volume |
| two images | the entry launched on the licensed image and `unmount-test.dmg` at once (a file manager's selection): after Agree both are mounted and each volume has its window |
| command line | a relative path is taken from the launcher's working directory (in the root mode too, where the launcher has moved into the container's mount namespace by then); a symbolic link to an attached image opens its window without attaching it again; a missing file gives `The file <path> does not exist.` and no file at all `Usage: darling open <file>...`, both with exit status 1 |

The entry used by the run is a copy of the installed one whose `Exec` names the `darling` on `PATH`: the installed
launcher is setuid root, which would start the root mode whatever mode the run is in.

Mounts are counted with `df` in the guest (the guest has no `mount` command).

## Results

2026-11-23, `finder-app` branch: every case passes in the rootless, non-root and root modes, each on X11 (private
Xvfb) and on Wayland (private headless sway with the virtual pointer of cocotron's `tests/wayland-virtual-pointer.c`).
