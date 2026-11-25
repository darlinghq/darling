# DiskArbitration

`diskarbitration.sh` checks the DiskArbitration library and `diskarbitrationd` with disk images. It also runs the
framework checks in `src/diskarbitration_match.c` and `src/diskarbitration_callbacks.c`.

```sh
DARLING_SKILLS=<darling-skills checkout> tests/diskarbitration.sh
```

It needs a running Darling with the launcher under test first in `PATH`. For the unprivileged modes it also needs
`DARLING_ROOTLESS=1` or `DARLING_NONROOT=1`, each with a `DPREFIX` of its own.

The C programs it builds (`da-*.c`) also pass on macOS.

## Observed on macOS 26.7.3 (25G342), x86_64

All cases use `fixtures/unmount-test.dmg` attached with `hdiutil attach`, unless they say otherwise.

### Lookups

| Call | Result |
|------|--------|
| `DADiskCreateFromVolumePath` of `/Volumes/Unmount Test`, with or without a trailing slash | the leaf `disk5s1` |
| `DADiskCreateFromVolumePath` of `/Volumes/Unmount Test/hello.txt`, `/tmp`, `/nonexistent` | NULL |
| `DADiskCreateFromVolumePath` of `/` | a disk |
| `DADiskCreateFromBSDName("disk99")` | a disk, whose `DADiskCopyDescription` is NULL |
| `DADiskCreateFromBSDName("/dev/disk5s1")` | `disk5s1` |
| two disks made for one name | `CFEqual`, but different objects |
| `DADiskCopyDescription` called twice | two different dictionaries |
| `DADiskCopyWholeDisk(disk5s1)` | `disk5` |

### Descriptions

**Leaf `disk5s1`:**

| Key | Value |
|-----|-------|
| `DADeviceModel` | "Disk Image" |
| `DADeviceProtocol` | "Virtual Interface" |
| `DADeviceVendor` | "Apple" |
| `DAMediaBSDName` | `disk5s1` |
| `DAMediaBSDUnit` | 5 |
| `DAMediaBlockSize` | 512 |
| `DAMediaContent` | `48465300-0000-11AA-AA11-00306543ECAC` |
| `DAMediaEjectable`, `DAMediaRemovable` | 1 |
| `DAMediaWhole` | 0 |
| `DAMediaLeaf` | 1 |
| `DAMediaWritable` | 0 |
| `DAMediaKind` | "IOMedia" |
| `DAMediaName` | "disk image" |
| `DAMediaSize` | 4153344, the volume's size as `statfs` and `diskutil` give it |
| `DAVolumeKind` | "hfs" |
| `DAVolumeMountable` | 1 |
| `DAVolumeName` | "Unmount Test" |
| `DAVolumeNetwork` | 0 |
| `DAVolumePath` | the CFURL `file:///Volumes/Unmount%20Test/` |
| `DAVolumeType` | "Mac OS Extended" |
| `DAVolumeUUID` | a CFUUID |
| `DAAppearanceTime` | a number |

macOS also gives some keys Darling leaves out, because no IOKit stands behind its disks:

- the IOKit paths `DAMediaPath`, `DADevicePath` and `DABusPath`;
- `DABusName`;
- `DADeviceRevision` and `DADeviceUnit`;
- `DAMediaBSDMajor` and `DAMediaBSDMinor`;
- `DAMediaIcon`;
- `DAMediaUUID`;
- `DAMediaEncrypted` and `DAMediaEncryptionDetail`.

**Whole disk `disk5`:** the same device and media keys, except:

| Key | Value |
|-----|-------|
| `DAMediaWhole` | 1 |
| `DAMediaLeaf` | 0 |
| `DAVolumeMountable` | 0 |
| `DAVolumeNetwork` | 0 |
| `DAMediaContent` | "GUID_partition_scheme" (Darling leaves it out: the daemon does not know the image's partition map) |

It has no volume name, path or kind.

The other images: `fixtures/slash-name.dmg` has `DAVolumeName` "A/B" at `/Volumes/A:B`, and `fixtures/amp-name.dmg`
has "Tom & Jerry <1>".

### Callbacks

Running `da-callbacks attach-detach` four times gave the same order every time:

1. attaching: appeared `disk5s1`, appeared `disk5`, then changed `disk5s1` with `DAVolumePath` (the volume mounts);
2. detaching: changed `disk5s1` with `DAVolumePath`, disappeared `disk5`, disappeared `disk5s1`.

The disks there already are come leaf first too. A session that is never scheduled gets no callbacks, and after
`DAUnregisterCallback` nothing more is delivered.

Darling's image volume is mounted before its disk appears, so its leaf appears with `DAVolumePath` already set and no
changed callback follows.

A session keeps working after `diskarbitrationd` restarts: its connection is interrupted, it says hello again and
catches up (`da-callbacks restart`, Darling only).

### Unmount, eject, mount and approvals

`da-ops` with the image attached, in this order (the statuses are what the completion callback's dissenter gives; 0
means no dissenter):

| Request | Status |
|---------|--------|
| eject the leaf `disk5s1` | `f8da000c` (kDAReturnUnsupported) |
| unmount `disk5s9`, which does not exist | `f8da0003` (kDAReturnBadArgument) |
| eject `disk5` while its volume is mounted | `c010` |
| unmount the leaf while a file in the volume is open | `c010` |
| mount the leaf while it is mounted | `f8da0002` (kDAReturnBusy) |
| with `da-ops deny` running: unmount the leaf; unmount `disk5` with kDADiskUnmountOptionWhole; eject `disk5` | `f8da0002` with the string "in use by test", each time |
| with `deny` running: unmount the leaf with kDADiskUnmountOptionForce | 0, and the mount point is gone |
| unmount the leaf again | `f8da0007` (kDAReturnNotMounted) |
| unmount `disk5` with kDADiskUnmountOptionWhole, nothing mounted | 0 |
| unmount `disk5` without options | `f8da0007` |
| eject `disk5` | 0 |
| eject it again | `f8da0003` |

The approver was asked about `disk5s1` for each leaf unmount, including the forced one (its dissent is then
ignored), and about `disk5` for each eject. An eject of a mounted disk with `deny` running gets the dissent, so the
approval comes before the check that the volume is unmounted. `c010` is 0xC000 with the errno EBUSY (16).

macOS mounts an unmounted leaf again when asked. Darling cannot (DiskArbitration does not drive the disk image
layer), so its mount request completes with `f8da000c`. A request also completes when an approver does not answer
for 10 s or has gone away, and when the volume disappears while the approval round runs.

`statfs` gives an attached image's `f_owner` as the user who attached it (501), and 0 for `/`; only root and that
user may unmount or eject the image.

## Darling's disks

Darling has no IOKit block devices. `diskarbitrationd` makes its disks from the mount table:

- the root volume is the whole disk `disk0` with the volume `disk0s1`;
- every volume whose source (`f_mntfromname`) is a file outside `/dev` is a disk image. It is the leaf `diskNs1` of
  a whole disk `diskN`, N being the lowest unit that is free.

macOS has more cases than that model: an image without a partition map has its volume on `diskN` itself, and an APM
image has it on `diskNs2`.

When an image's volume goes away, its mount point in `/Volumes` is removed, as on macOS. That also happens after a
bare `unmount(2)`. The daemon takes the same lock on `/Volumes` that the attach library holds while it names and
mounts a volume, so it never removes a mount point that an attach has just made.
