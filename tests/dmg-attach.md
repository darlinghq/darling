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
