# DiskImageMounter

`disk-image-mounter.sh` checks DiskImageMounter.app (`src/DiskImageMounter`), the handler of disk images: `open`
on an image in the guest attaches it, asks for its licence agreement first when it has one, and opens the volume
with the application that declares the volume type. A stand-in for Finder (`src/ae-receiver.m`, registered as
`com.apple.finder` with Finder's "Folder" and "Volume" document types) records the open event it gets.

It also runs two programs in process: `src/dim_image_opener.m` (the opener, with a recording stand-in for its
user interface and a failing attach) and `src/dim_license_window.m` (the licence window, with the
`sla-*.dmg` fixtures described in `dmg-license.md`).

```sh
DARLING_SKILLS=<darling-skills checkout> DISPLAY=<your own X display> tests/disk-image-mounter.sh
```

It needs a running Darling with the launcher under test first in `PATH` (and `DARLING_ROOTLESS=1` or
`DARLING_NONROOT=1` with a `DPREFIX` of its own for the unprivileged modes); `DARLING_ROOT` is the installed
tree. Export the display before the container starts (`darling shutdown` first): X11 with `DISPLAY` (and
`xdotool`), or Wayland with `WAYLAND_DISPLAY`, `XDG_RUNTIME_DIR`, `SWAYSOCK` and
`DARLING_APPKIT_BACKEND=wayland` (and `swaymsg`, `wtype`). The test types keys into DiskImageMounter's
windows, so use a display of your own.

## Cases

| Case | What it checks |
|------|----------------|
| opener | the opener in process: no licence, licence declined and agreed, attached already, not a disk image, a damaged image, a failing attach |
| license window (in process) | the window's languages and initial language, texts, prompts, button titles, defaults, RTF, and the replies of Agree, Disagree and closing |
| open | an image is attached, the stand-in Finder gets `odoc` for the volume, DiskImageMounter quits |
| open twice | the attached image is opened again, not attached again |
| odd path | spaces, `&` and non-ASCII characters in the image's path |
| bad image | an error alert; Return dismisses it, nothing is mounted, DiskImageMounter quits |
| license window | Escape declines (nothing mounted, it quits); opened again, Return agrees and the volume is opened |
| license twice | two opens of a licence image one second apart give one window and, after Return, one mount |
| kill during license | killing DiskImageMounter while the agreement shows leaves nothing mounted |
| attached license image | an attached image with a licence agreement is opened without asking again |

## Observed on macOS 26.7.3 (25G342), x86_64

**Directory types and handlers** (`NSURLTypeIdentifierKey`, `LSCopyDefaultApplicationURLForURL(url, kLSRolesAll)`):

| Path | Type | Handler |
|------|------|---------|
| a plain directory, or `odd.foo` with an undeclared extension | `public.folder` | Finder |
| `/`, `/Volumes/Unmount Test` (with or without a trailing `/`) | `public.volume` | Finder |
| `/Volumes` | `public.folder` | Finder |
| `x.rtfd` | `com.apple.rtfd` | TextEdit |
| `TextEdit.app` | `com.apple.application-bundle` | itself |

**OSType tags:**
- `UTTypeCopyPreferredTagWithClass(public.folder, com.apple.ostype)` is `fold`, and for `public.volume` it is `disk`.
- The reverse lookups give `public.folder` and `public.volume`.
- Finder's `Info.plist` declares a "Folder" document type (`CFBundleTypeOSTypes` `fold`) and a "Volume" document type (`disk`), both role Editor, with no `LSItemContentTypes`.

**Attaching an image that is already attached:**
- A second `hdiutil attach` of the same file, also through a symlink to it, prints the existing devices and mount point and exits 0. Nothing new is attached.
- A copy of the file attaches separately, as `Unmount Test 1`.
- A second attach of an attached image with a licence agreement does not ask again: with stdin `/dev/null` it prints the devices and exits 0.

**DiskImageMounter:**
- Its `Info.plist` has `LSUIElement` true, `NSPrincipalClass` `NSApplication`, and every document type has role Viewer.
- `open unmount-test.dmg` attaches and mounts it, and DiskImageMounter has quit 16 s later.
- A second `open` of the attached image attaches nothing more.
- `open bad-blkx.dmg`: nothing is attached, and DiskImageMounter is gone within 5 s. Where the error is shown could not be observed: there are no screenshots over ssh.
- `open sla-test.dmg`: DiskImageMounter keeps running with nothing attached until it is killed. Killing it leaves nothing attached.
- `open -a DiskImageMounter` with no document: it is still running 15 s later.
