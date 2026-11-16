# unmount(2) and `hdiutil detach` on a disk image

`unmount-disk-image.sh` attaches `fixtures/unmount-test.dmg` (as `Unmount Test.dmg`, so the mount point has a
space in it), runs `unmount-disk-image.c` on the volume, then checks `hdiutil detach`.

```sh
DARLING_SKILLS=<darling-skills checkout> tests/unmount-disk-image.sh
```

It prints one `PASS`/`FAIL` line per check and exits non-zero on any failure. It needs a running Darling and the
`darling` launcher of the install under test first in `PATH` (set `DARLING_ROOTLESS=1` or `DARLING_NONROOT=1`
and a `DPREFIX` of its own for the unprivileged modes).

## Fixture

`fixtures/unmount-test.dmg` was made for this test on macOS 26.7.3 (25G342):

```sh
hdiutil create -size 4m -fs HFS+ -volname "Unmount Test" ut.dmg
hdiutil attach ut.dmg -mountpoint /tmp/utmnt
mkdir /tmp/utmnt/sub; echo hello > /tmp/utmnt/hello.txt
hdiutil detach /tmp/utmnt
hdiutil convert ut.dmg -format UDZO -o ut-ro.dmg
```

## Expected behaviour (observed on macOS 26.7.3, x86_64)

`unmount-disk-image.c` passes unchanged on macOS with the image attached, also with `ignore-sigchld`.

`hdiutil detach`:

| Case | stdout | stderr | exit |
|------|--------|--------|------|
| success | `"disk5" ejected.` | | 0 |
| volume busy (a file open on it) | | `hdiutil: couldn't unmount "disk5" - Resource busy` | 16 |
| volume busy, `-quiet` | | | 16 |
| volume busy, `-force` | `"disk5" ejected.` | | 0 |
| path with no image mounted (missing, or a plain directory) | | `hdiutil: detach failed - No such file or directory` | 1 |

Darling has no disk names for images yet, so its messages name the mount point where macOS names the disk; the
test matches any name.

## Results

| Launcher mode | Result |
|---------------|--------|
| root (setuid launcher) | all pass |
| rootless (`DARLING_ROOTLESS=1`) | all pass |
| non-root (`DARLING_NONROOT=1`) | all pass |
