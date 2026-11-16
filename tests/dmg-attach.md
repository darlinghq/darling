# Attaching disk images

`dmg-attach.sh` checks `hdiutil attach` and the `libdarling-dmg` API (`dmg-api.c`) with
`fixtures/unmount-test.dmg` (volume "Unmount Test", see `unmount-disk-image.md`), and runs
`unmount-disk-image.sh` as a regression.

```sh
DARLING_SKILLS=<darling-skills checkout> tests/dmg-attach.sh
```

Needs a running Darling with the launcher under test first in `PATH` (and `DARLING_ROOTLESS=1` or
`DARLING_NONROOT=1` with a `DPREFIX` of its own for the unprivileged modes); `DARLING_ROOT` is the installed tree.

## Observed on macOS 26.7.3 (25G342), x86_64

| Case | stdout | stderr | exit |
|------|--------|--------|------|
| `hdiutil attach <missing file>` | | `hdiutil: attach failed - No such file or directory` | 1 |
| `hdiutil attach <a text file>` | | `hdiutil: attach failed - image not recognized` | 1 |
| `hdiutil attach -plist <image>` | plist; `system-entities` has a dict with `mount-point` = `/Volumes/Unmount Test`, `volume-kind` = `hfs` | | 0 |
| `sh -c "hdiutil attach -quiet <image>"`, then from the outer shell | the volume stays mounted after the attaching process exited | | 0 |

A plain `hdiutil attach` prints a device table (`/dev/disk5 GUID_partition_scheme`, `/dev/disk5s1 Apple_HFS
/Volumes/Unmount Test`); Darling prints only the mount point there for now (disk names come with DiskArbitration).

### Mount point names (same host)

| Case | Mount point |
|------|-------------|
| `unmount-test.dmg` copied to `renamed.dmg`, attached | `/Volumes/Unmount Test` (the volume's name, not the file's) |
| a second and a third image with the same volume name attached at once | `/Volumes/Unmount Test 1`, `/Volumes/Unmount Test 2` |
| `Unmount Test 1` detached, then a fourth one attached | `/Volumes/Unmount Test 1` (lowest free number) |
| image made with `hdiutil create -volname "A/B"` (`fixtures/slash-name.dmg`; `diskutil info` shows the name `A/B`) | `/Volumes/A:B` |
| `-mountroot <dir>`, with `<dir>/Unmount Test` taken (by a mounted volume, an empty directory or a file) | `<dir>/Unmount Test 2`; a third one `<dir>/Unmount Test 3` |

`fixtures/slash-name.dmg`: made on that host with `hdiutil create -size 2m -fs HFS+ -volname "A/B"` and
`hdiutil convert -format UDZO`; an empty HFS+ volume named `A/B`.

`hdiutil create -volname ""` stores the name `untitled`, so an empty volume name could not be observed; Darling
mounts a volume whose stored name is empty, `.` or `..` as `untitled` (same rules for taken names).

macOS removes a mount point it made in `/Volumes` when its volume goes away, even after a bare `unmount(2)`;
Darling has nothing doing that yet (DiskArbitration), so attach treats an empty directory in `/Volumes` with
nothing mounted on it as free. `unmount-disk-image.sh`, which attaches again after `unmount(2)`, depends on it.
