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

A plain `hdiutil attach` prints a device table; see "Disks and detach" below.

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
in Darling `diskarbitrationd` does that (see `diskarbitration.md`). Attach also treats an empty directory in
`/Volumes` with nothing mounted on it as free.

### Disks and detach

`hdiutil attach` prints a line per disk: the device padded to 20 characters, a tab, the content hint padded to 31,
a tab, and the mount point (empty for the whole disk). Shown with tabs as `>`:

| Image | Table |
|-------|-------|
| `unmount-test.dmg` (GUID map) | `/dev/disk5          >GUID_partition_scheme          >`<br>`/dev/disk5s1        >Apple_HFS                      >/Volumes/Unmount Test` |
| `apm.dmg` (Apple partition map) | `/dev/disk8 … Apple_partition_scheme`, `/dev/disk8s1 … Apple_partition_map`, `/dev/disk8s2 … Apple_HFS … /Volumes/APMv` |
| `nolayout.dmg` (no map) | one line: `/dev/disk9`, an empty hint, `/Volumes/NoLayout` |

`hdiutil attach -plist` lists the volume's entry first (`content-hint` `Apple_HFS`, `dev-entry` `/dev/disk5s1`,
`mount-point`, `potentially-mountable` true, `unmapped-content-hint` the HFS+ GPT type
`48465300-0000-11AA-AA11-00306543ECAC`, `volume-kind` `hfs`), then the whole disk's (`content-hint` and
`unmapped-content-hint` `GUID_partition_scheme`, `dev-entry` `/dev/disk5`, `potentially-mountable` false). In an APM
image the volume's `unmapped-content-hint` is `Apple_HFS`.

Darling's disks always have the volume on `s1`, so it prints two lines for every image: the whole disk with the
image's scheme (empty without a map), then `s1`. If DiskArbitration does not know the volume (a `-mountroot` outside
the guest's own tree, which `getfsstat` does not list), attach prints only the mount point, and detach unmounts it
without DiskArbitration, naming it by its mount point.

| `hdiutil detach …` | stdout | stderr | exit |
|--------------------|--------|--------|------|
| `/dev/disk5` | `"disk5" ejected.` | | 0 |
| `/dev/disk99`, or a path that is not a volume | | `hdiutil: detach failed - No such file or directory` | 1 |
| `/dev/disk5`, with `da-ops deny` dissenting | `in use by test` (no newline) | `hdiutil: couldn't unmount "disk5" - Resource busy` | 2 |
| `-force /dev/disk5`, with `deny` | `in use by test` | `hdiutil: couldn't eject "disk5" - Resource busy` (the volume is unmounted) | 2 |
| `-quiet /dev/disk5`, with `deny` | | | 2 |

The fixtures were made there with `hdiutil create -size 2m -fs HFS+ -layout SPUD -volname APMv` and
`-layout NONE -volname NoLayout` (UDRW, kept gzipped as `fixtures/apm-udrw.dmg.gz` and
`fixtures/nolayout-udrw.dmg.gz`), then `hdiutil convert -format UDZO` (`fixtures/apm.dmg`, `fixtures/nolayout.dmg`).
`dmg-partition-scheme-host.cpp` checks the scheme of all five on the host.

### More fixtures

- `fixtures/amp-name.dmg`: made on the same host with `hdiutil create -size 2m -fs HFS+ -volname "Tom & Jerry <1>"`
  and `hdiutil convert -format UDZO`. `hdiutil attach -plist` there reports
  `<string>/Volumes/Tom &amp; Jerry &lt;1&gt;</string>`.
- `fixtures/long-name-key.hfs.gz` and `fixtures/bad-blkx.dmg`: damaged images made by
  `fixtures/make-crafted-fixtures.py` (from a raw HFS+ image made there with `hdiutil create -layout NONE` and
  `hdiutil convert -format UDTO`, and from `unmount-test.dmg`); used by `dmg-crafted.c` (in Darling) and
  `dmg-crafted-host.cpp` (on the host, with AddressSanitizer).
- `dmg-mount-point-name.cpp`: host unit test of the mount point naming rules (build line in the file).

Two images with the same volume name attached at the same moment must get different mount points (the name is
chosen and the volume mounted while holding a lock on the mount root). When the volume cannot be mounted, hdiutil
prints what FUSE or fusermount said after the `attach failed` line (checked in non-root and root mode; rootless mode
mounts on a read-only directory without complaint).

### Attaching an image that is attached (macOS 26.7.3, 25G342)

- A second `hdiutil attach -nobrowse unmount-test.dmg`, also through a symbolic link to the file, prints the same
  devices and `/Volumes/Unmount Test` and exits 0; `hdiutil info` still lists the image once. A copy of the file
  attaches separately (`/Volumes/Unmount Test 1`).
- A second attach of an attached `sla-test.dmg` does not ask for the agreement: with stdin `/dev/null` it prints the
  devices and exits 0 (`dmg-license.sh`, "attached image asks nothing").

In Darling the image is identified by its real path, which is the source (`f_mntfromname`) of its volume, so a hard
link to an attached image attaches again. `dmg-attach.sh` "attach twice" and `dmg-api.c` check it; `dmg-api.c` does
it under `/Volumes`, because `getfsstat` does not list volumes mounted on the host side (under `/Volumes/SystemRoot`).
