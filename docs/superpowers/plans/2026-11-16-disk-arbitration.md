# DiskArbitration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A real DiskArbitration for Darling. A `diskarbitrationd` launchd daemon keeps the disk model built from the
mount table. The DiskArbitration client library replaces today's dummy: lookups, descriptions, callbacks across
processes, unmount, eject and dissenters. `hdiutil` reports and detaches disks through it.

**Architecture:**
- **The file system tells the kernel layer its name and kind.** darling-dmg's FUSE file system publishes the volume
  name and kind as private xattrs on its root. Darling's `libsystem_kernel` reads them, so `getattrlist(ATTR_VOL_NAME)`
  and `statfs().f_fstypename` answer the way macOS's VFS does.
- **The daemon builds its model from public calls.** `diskarbitrationd` builds the disk model from `getfsstat`,
  `getattrlist` and `stat`, and rescans on kqueue `EVFILT_FS` events. It serves the client library over XPC as the
  Mach service `com.apple.DiskArbitration.diskarbitrationd`. It names disks, runs approvals across processes, unmounts
  with `unmount(2)` and removes mount points it no longer needs in `/Volumes`.
- **`hdiutil` sits above DiskArbitration.** It asks DiskArbitration for disk names after attaching, and detaches by
  unmounting and ejecting through it, as on macOS.

**Tech Stack:**
- C with CoreFoundation, libxpc, libdispatch and kqueue for the client library and the daemon.
- C++ for darling-dmg.
- C for darling-xnu `libsystem_kernel`.
- Guest test programs built with darling-skills `guest-cc.sh`; shell drivers run in all three launcher modes.

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md` ("How DiskArbitration is built", "Disk images",
"Unmount and DiskArbitration"); roadmap `docs/superpowers/plans/2026-11-16-finder-roadmap.md`, plan 3.

## Global Constraints

- **AGENTS.md applies throughout:**
  - documented contract first;
  - no app-specific behaviour;
  - `QUIRK:` comments carrying evidence and a test;
  - no debug leftovers;
  - a Provenance note on every PR.
- **Clean room.** Work only from Darling's in-tree public headers (`src/frameworks/DiskArbitration/include`), Apple's
  public documentation and black-box observation on the Mac (`ssh lubos@10.10.10.152`, macOS 26.7.3).
  - Apple's DiskArbitration project source is APSL 2.0, which is incompatible with Darling's GPL. Treat it as
    restricted (AGENTS.md §5 extension): never open it, quote it or reproduce it.
- **Behaviour observed on macOS 26.7.3 (25G342, x86_64).** These are the values the tests pin. Task 3 records them in
  `tests/diskarbitration.md`.

  *Lookups and descriptions*
  - **Unknown BSD name:** `DADiskCreateFromBSDName` returns a disk object even for an unknown name (`disk99`), and its
    `DADiskCopyDescription` is NULL. A `/dev/` prefix is accepted and dropped (`/dev/disk5s1` → `disk5s1`).
  - **Volume path:** `DADiskCreateFromVolumePath` returns NULL unless the URL is exactly a mount point, with or without a
    trailing slash. A file inside a volume, `/tmp` and a missing path all return NULL. `/` returns a disk.
  - **Identity:** two disks created for the same name are `CFEqual` but are different objects. Each
    `DADiskCopyDescription` call returns a new dictionary.
  - **Image volume description:**
    `DADeviceModel` "Disk Image", `DADeviceProtocol` "Virtual Interface", `DADeviceVendor` "Apple",
    `DAMediaBSDName` "disk5s1", `DAMediaBSDUnit` 5, `DAMediaBlockSize` 512,
    `DAMediaContent` "48465300-0000-11AA-AA11-00306543ECAC",
    `DAMediaEjectable` 1, `DAMediaRemovable` 1, `DAMediaWhole` 0, `DAMediaLeaf` 1, `DAMediaWritable` 0,
    `DAMediaKind` "IOMedia", `DAMediaName` "disk image",
    `DAVolumeKind` "hfs", `DAVolumeMountable` 1, `DAVolumeName` "Unmount Test", `DAVolumeNetwork` 0,
    `DAVolumePath` file:///Volumes/Unmount%20Test/ (a CFURL), `DAVolumeType` "Mac OS Extended",
    `DAVolumeUUID` (a CFUUID), `DAAppearanceTime` (a number).
  - **Whole disk "disk5":** the same device keys, with `DAMediaWhole` 1, `DAMediaLeaf` 0, `DAVolumeMountable` 0,
    `DAVolumeNetwork` 0, and no volume name, path or kind.

  *Callbacks*
  - **Existing disks:** an appeared callback is called for the disks already present once the session is scheduled.
  - **Unmount:** watchers get a description change for the leaf with the keys `[DAVolumePath]`, and the disk stays.
  - **Eject:** disappeared is called for the whole disk first, then the leaf.

  *Unmount and eject results*
  - **Unmount:**
    - an unknown leaf (`disk5s9`) → `kDAReturnBadArgument`;
    - a leaf that is not mounted → `kDAReturnNotMounted`;
    - `kDADiskUnmountOptionWhole` on a whole disk with nothing mounted → success.
  - **Eject:**
    - of a leaf → `kDAReturnUnsupported`;
    - of a whole disk with a volume still mounted → status `0xC010` (`0xC000 | EBUSY`);
    - of a gone disk → `kDAReturnBadArgument`;
    - success → the image is detached.
  - **Mount point:** it is removed when the volume is unmounted through DiskArbitration.

  *Dissenters*
  - **Busy dissent from another process:** an unmount or eject approval callback there returning `kDAReturnBusy` with
    the string "in use by test" makes the request finish with that status and string.
  - **Whole-disk unmount** asks approval for the leaf; **eject** asks for the whole disk.
  - **`kDADiskUnmountOptionForce`** still asks for approval but ignores the dissent.

  *`hdiutil`*
  - **Attach table:** each line is `printf("%-20s\t%-31s\t%s\n", dev, hint, mountPoint)`, the whole disk first, e.g.
    `/dev/disk5          \tGUID_partition_scheme          \t` and
    `/dev/disk5s1        \tApple_HFS                      \t/Volumes/Unmount Test`.
    - An APM image has the whole disk hint `Apple_partition_scheme`.
    - An image without a partition map prints one line, the volume on the whole disk with an empty hint.
  - **`attach -plist`:** `system-entities` lists the leaf first, then the whole disk.
    - Leaf keys: `content-hint` Apple_HFS, `dev-entry`, `mount-point`, `potentially-mountable` true,
      `unmapped-content-hint` 48465300-…, `volume-kind` hfs.
    - Whole disk keys: `content-hint` scheme, `dev-entry`, `potentially-mountable` false, `unmapped-content-hint`
      scheme.
  - **`detach`:**
    - accepts a mount point, `/dev/diskN` or `/dev/diskNsM`, and prints `"diskN" ejected.`;
    - `/dev/disk99` → stderr `hdiutil: detach failed - No such file or directory`, exit 1;
    - with an unmount dissenter → stdout the dissenter's string with no newline, stderr
      `hdiutil: couldn't unmount "disk5" - Resource busy`, exit 2;
    - `-force` with an eject dissenter → `hdiutil: couldn't eject "disk5" - Resource busy`, exit 2;
    - a busy file → exit 16 (Plan 1). So the exit status is the status's low byte.
- **Private install and run rules:**
  - Install into `~/darling-finder` only: `cmake -DCMAKE_INSTALL_PREFIX=$HOME/darling-finder`, then
    `make -C ~/Projects/darling/build install` or a per-target `cmake -P <dir>/cmake_install.cmake`.
  - Re-make the launcher setuid root after any install (`sudo chown root:root ~/darling-finder/bin/darling`,
    `sudo chmod 4755 …`).
  - Rootless runs use `~/darling-finder/rootless-bin/darling`.
  - Run the whole test list in rootless, non-root and root mode with
    `TESTS="…" $SCRATCH/helpers/matrix-plan2.sh <mode>`.
- **Branches and PRs:**
  - darling-xnu: branch `volume-attributes` from `unmount-syscall`.
  - darling-dmg: branch `disk-arbitration` from `attach-library`.
  - darling: `finder-app`.
  - Push the submodule branches before the main-repo bump. Open no PRs.

## Review Focus

1. **A volume vanishes mid-request.** The FUSE process is killed or something else unmounts the volume while an
   approval round is pending. The request must finish with a status (NotMounted or BadArgument), never hang or crash,
   and leave no stale disk. Test: `da-ops` "volume vanishes during approval" (Task 5).
2. **An approver dies or never answers.** A dissenter process that dies or hangs must not stall unmount or eject past
   the approval timeout (10 s), and a dead connection's registrations are dropped at once. Test: `da-ops` "dead
   approver" (Task 5).
3. **Mount-point cleanup racing an attach.** The daemon's cleanup must not remove a mount point that an attach has just
   created for a new volume of the same name. Test: `diskarbitration.sh` "unmount then attach at once, 20 rounds"
   (Task 3).
4. **Odd volume names.** A volume named "A/B" has `DAVolumeName` "A/B" and the path `/Volumes/A:B`. Names with `&`,
   `<` or non-ASCII survive the round trip through XPC. Test: `da-disks` with `slash-name.dmg` and `amp-name.dmg`
   (Task 3).
5. **The daemon restarts.** After launchd restarts `diskarbitrationd` (KeepAlive), an open session still gets appeared
   and disappeared callbacks for later attaches and ejects. Test: `da-callbacks` "daemon restart" (Task 4).

---

### Task 1: darling-dmg publishes the volume name and kind

**Files:**
- Modify: `src/external/darling-dmg/src/main-fuse.cpp` (getxattr/listxattr)
- Test: `tests/dmg-volume-xattr.sh` (host; builds the standalone darling-dmg and mounts with host FUSE)

**Interfaces:**
- Produces:
  - On the root directory of a darling-dmg mount, the xattr `darling.volume.name` is the HFS+ volume name in UTF-8,
    unterminated.
  - `darling.volume.kind` is `hfs`.
  - Neither appears in `listxattr`. Other paths return ENODATA for both. The `darling.` namespace is reserved for
    Darling's file system ↔ kernel-layer contract; guests only ever see `user.` xattrs, so they cannot see these.

- [ ] **Step 1: Write the failing test.** `tests/dmg-volume-xattr.sh`:
  - builds the standalone darling-dmg (`cmake -S src/external/darling-dmg -B $work/build && make -C $work/build`);
  - mounts `tests/fixtures/unmount-test.dmg` and `slash-name.dmg` on temporary directories (`darling-dmg <img> <dir>`)
    and checks with `getfattr`:
    - `getfattr --only-values -n darling.volume.name <mp>` = `Unmount Test` and `A/B`;
    - `darling.volume.kind` = `hfs`;
    - `getfattr -d -m - <mp>` does not list them;
    - `getfattr -n darling.volume.name <mp>/hello.txt` fails;
  - unmounts with `fusermount -u` and prints PASS/FAIL lines.
- [ ] **Step 2: Run it.** `tests/dmg-volume-xattr.sh` — Expected: FAIL (no such attribute).
  - If the host kernel refuses the `darling.` namespace before FUSE (getfattr reports "Operation not supported"
    without the daemon being asked), stop and ledger a ruling. The fallback is `trusted.darling.*` read by root only;
    re-plan Task 2 accordingly.
- [ ] **Step 3: Implement.** In `hfs_getxattr`, answer the two names for path `/` from the opened volume's
  `volumeName()` and the constant `hfs`. Return the size when `vlen` is 0 and ERANGE when it is too small. Leave
  `hfs_listxattr` unchanged.
- [ ] **Step 4: Run it.** Expected: all PASS. Run the host tests from Plan 2 (`dmg-mount-point-name`, `dmg-crafted-host`)
  — PASS.
- [ ] **Step 5: Commit** in darling-dmg (branch `disk-arbitration`): "fuse: publish the volume name and kind to the
  kernel layer". In darling: `git add -f tests/dmg-volume-xattr.sh`, commit with the submodule bump.

### Task 2: getattrlist ATTR_VOL_NAME and statfs f_fstypename from the file system

**Files:**
- Modify:
  - `src/external/xnu/darling/src/libsystem_kernel/emulation/include/xnu_syscall/bsd/helper/xattr/getattrlist_generic.c`
    (`ATTR_VOL_NAME`);
  - `.../src/xnu_syscall/bsd/helper/stat/guest_mount.c` and `.../include/.../guest_mount.h` (kind, volume name helper).
- Test:
  - `src/external/xnu/darling/tests/volume-name-kind.c` (guest program; also passes on macOS);
  - `tests/volume-attributes.sh` (darling; attaches images and runs it).

**Interfaces:**
- Consumes: Task 1 xattrs.
- Produces:
  - **`guest_mount_volume_name(const char* host_mount_point, const char* guest_name, char* out, size_t size)`** in
    `guest_mount.h`, in this order:
    1. the `darling.volume.name` xattr of the mount root;
    2. `Macintosh HD` for the root volume;
    3. the last component of `guest_name`.
  - **`f_fstypename`**: for a Linux type starting with `fuse`, a `darling.volume.kind` xattr (if any) replaces it, in
    `getfsstat`, `statfs` and `fstatfs` alike.
  - **`getattrlist`**: `ATTR_VOL_NAME` is supported on any object, naming the volume that holds it.

- [ ] **Step 1: Write the failing test.** `volume-name-kind.c <mount point> <expected name> <expected kind>` checks:
  - `getattrlist(ATTR_VOL_NAME)` on the mount point and on a file inside it = expected name;
  - `statfs(mp).f_fstypename` = expected kind, and the `getfsstat` entry for `mp` has the same kind;
  - `getattrlist(ATTR_VOL_NAME)` on `/` is non-empty;
  - `ATTRIBUTE_SET_T` `validattr` reports `ATTR_VOL_NAME`.

  `tests/volume-attributes.sh` attaches `unmount-test.dmg` twice, so the second mounts at `/Volumes/Unmount Test 1`,
  and `slash-name.dmg`. It runs the program with:
  - (`/Volumes/Unmount Test`, `Unmount Test`, `hfs`);
  - (`/Volumes/Unmount Test 1`, `Unmount Test`, `hfs`);
  - (`/Volumes/A:B`, `A/B`, `hfs`).

  Then it detaches the images.
- [ ] **Step 2: Run it** (root mode). Expected: FAIL. `ATTR_VOL_NAME` is not supported, and the kind is `fuse.darling-dm`.
- [ ] **Step 3: Implement.**
  - Add `guest_mount_volume_name` and the kind replacement in `guest_mount.c`, for both `list_visit` and
    `guest_mount_describe`. Use the Linux `lgetxattr` on the host mount point.
  - Add `ATTR_VOL_NAME` to `VOLUME_SUPPORTED` and emit it as an `attrreference` string, NUL-terminated, after the
    fixed part, as the other variable-length attributes are.
- [ ] **Step 4: Run it** in all three modes. Expected: PASS. Run xnu's `statfs-guest-volumes.c`,
  `getattrlist-volume-uuid.c` and `mount-options-user-id.c` too — PASS.
- [ ] **Step 5: Commit** in darling-xnu (branch `volume-attributes`): "getattrlist: ATTR_VOL_NAME; statfs: the
  kind a FUSE file system names". In darling: `git add -f tests/volume-attributes.sh`, commit with the bump.

### Task 3: diskarbitrationd and the disk lookups

**Files:**
- Create:
  - `src/frameworks/DiskArbitration/DAProtocol.h`: message keys shared by the library and the daemon;
  - `src/frameworks/DiskArbitration/DAClient.{c,h}`: the session's connection and synchronous requests;
  - `src/frameworks/DiskArbitration/diskarbitrationd/{CMakeLists.txt,main.c,DAModel.{c,h},DAServer.{c,h}}`;
  - `src/frameworks/DiskArbitration/diskarbitrationd/com.apple.diskarbitrationd.plist`.
- Modify:
  - `src/frameworks/DiskArbitration/{CMakeLists.txt,DADisk.c,DASession.c,README}`;
  - `src/frameworks/CMakeLists.txt` if subdirectories need adding.
- Test:
  - `tests/da-disks.c` (guest);
  - `tests/diskarbitration.sh` and `tests/diskarbitration.md` (the macOS observations above, case by case).

**Interfaces:**
- Consumes: `guest_mount` behaviour from Task 2, through `getfsstat` and `getattrlist`.
- Produces:
  - **The daemon.**
    - Installed at `libexec/darling/usr/libexec/diskarbitrationd`, with the plist in
      `libexec/darling/System/Library/LaunchDaemons`.
    - The plist sets `Label` com.apple.diskarbitrationd, `MachServices` {com.apple.DiskArbitration.diskarbitrationd},
      `KeepAlive` true and `RunAtLoad` true.
  - **The model (`DAModel.h`).** Disks are kept as `DADiskRecord`s (BSD name, whole/leaf, unit, mount point, image
    path, owner uid, mounted flag, description `CFMutableDictionaryRef`, appearance time).
    - `void DAModelRescan(DAModelEvents* events)` reconciles the model with `getfsstat` and reports what appeared,
      disappeared or changed (with the changed keys).
    - Recognised volumes:
      - the root volume `/`, as `disk0` (whole, not ejectable, not removable, internal) with `disk0s1`;
      - every volume whose `f_mntfromname` is an absolute path outside `/dev/`: an image-backed volume, whole `diskN`
        with leaf `diskNs1`, N being the lowest free unit from 1.
    - Other mounts get no disk.
    - Descriptions use the values listed under Global Constraints:
      - `DAVolumeName` from `getattrlist(ATTR_VOL_NAME)`;
      - `DAVolumeUUID` from `ATTR_VOL_UUID`;
      - `DAVolumeKind` from `f_fstypename`;
      - `DAVolumeType` "Mac OS Extended" only for kind `hfs`;
      - `DAMediaContent` the HFS+ GPT type GUID for kind `hfs`;
      - `DAMediaSize` = `f_blocks * f_bsize`;
      - `DAMediaWritable` = !`MNT_RDONLY`.
    - The IOKit paths (`DAMediaPath`, `DADevicePath`, `DABusPath`) are left out.
  - **Rescans.** The daemon rescans on start, on every kqueue `EVFILT_FS` event and before answering a lookup that
    misses.
  - **Mount-point cleanup.** When an image volume's mount goes away and its mount point is directly in `/Volumes`, is an
    empty directory and has nothing mounted on it, the daemon removes it. It holds `flock(LOCK_EX)` on `/Volumes` while
    checking and removing; the attach library takes the same lock.
  - **Protocol (`DAProtocol.h`).** XPC dictionaries with `"op"`:
    - `"describe"` {`"name"`} → {`"description"`: binary plist data} or {`"status"`: kDAReturnNotFound}. In the plist,
      `DAVolumePath` is a path string and the UUID keys are strings; the client converts them to `CFURL` and `CFUUID`.
    - `"lookup-path"` {`"path"`} → {`"name"`} or {`"status"`}.
  - **Client library.**
    - `DASessionCreate` opens an `xpc_connection_create_mach_service` connection on a private serial queue.
    - `DADiskCreateFromBSDName` (drops `/dev/`, never asks the daemon).
    - `DADiskCreateFromVolumePath` (`realpath`, then `"lookup-path"`; NULL on a miss).
    - `DADiskGetBSDName`.
    - `DADiskCopyDescription` (a fresh `"describe"` every call; NULL for an unknown disk).
    - `DADiskCopyWholeDisk` (strip `sN`; NULL if no such disk).
    - `CFEqual` and `CFHash` compare BSD names.

- [ ] **Step 1: Write the failing test.** `da-disks.c <image mp> <expected name> <slash mp> <amp mp>` checks:
  - `DADiskCreateFromVolumePath(mp)` gives a leaf `diskNs1`. The trailing-slash form gives the same name. A file
    inside the volume, `/nonexistent` and `/tmp` give NULL. `/` gives `disk0s1`.
  - The leaf description matches the Global Constraints values key by key, with `DAVolumeName` = expected name,
    `DAVolumePath` a CFURL equal to `file://` + mp + `/`, and `DAVolumeUUID` a CFUUID.
  - `DADiskCopyWholeDisk` gives `diskN` with `DAMediaWhole` true, `DAVolumeMountable` false and no volume keys.
  - `DADiskCreateFromBSDName("disk99")` is non-NULL with a NULL description; `/dev/diskNs1` gives `diskNs1`.
  - Two disks for one name are `CFEqual` but are different objects, and two descriptions are different objects.
  - The slash and amp volumes have `DAVolumeName` "A/B" and "Tom & Jerry <1>" (Review Focus 4).

  `tests/diskarbitration.sh` attaches `unmount-test.dmg`, `slash-name.dmg` and `amp-name.dmg`, runs `da-disks`, and
  detaches. Then two more cases follow:
  - "mount point removed after unmount(2)": `unmount(2)` the volume through a small `umount`-less C call, which may
    be `da-disks --unmount <mp>`; within 5 s `/Volumes/Unmount Test` is gone.
  - "unmount then attach at once, 20 rounds": `hdiutil detach` and an immediate `hdiutil attach`, 20 times; every
    attach succeeds at `/Volumes/Unmount Test`, which then holds `hello.txt` (Review Focus 3).
- [ ] **Step 2: Run it.** Expected: FAIL (the stub returns the URL as a "BSD name").
- [ ] **Step 3: Implement** the daemon, the plist, the protocol and the client functions above. Make the attach library
  hold its existing `MountRootLock` (the flock on the mount root) as it does now. The daemon's cleanup uses the same
  kind of lock, so nothing changes in darling-dmg.
- [ ] **Step 4: Run it** in all three modes, together with `tests/dmg-attach.sh`. Expected: PASS.
- [ ] **Step 5: Commit** in darling: "DiskArbitration: diskarbitrationd and disk lookups". Use `git add -f` for the
  test files.

### Task 4: sessions and appeared / disappeared / description-changed callbacks

**Files:**
- Modify: `src/frameworks/DiskArbitration/{DASession.c,DAClient.c,DAProtocol.h}`;
  `diskarbitrationd/DAServer.c`.
- Test: `tests/da-callbacks.c` (guest); cases in `tests/diskarbitration.sh`.

**Interfaces:**
- Consumes: Task 3 model events, protocol and client connection.
- Produces:
  - **Events.** The daemon sends every connection that said `"hello"` the events `"appeared"`, `"disappeared"` and
    `"changed"` ({`"name"`, `"description"`, and for `"changed"` also `"keys"`, an array of key strings}). Right after
    `"hello"` it sends `"appeared"` for every current disk, whole disks first.
  - **Ordering.** Appeared goes whole then leaf; disappeared goes whole then leaf.
  - **Delivery.** The client keeps the events in a per-session queue and delivers them only on the scheduled run loop
    (a version 0 `CFRunLoopSource`) or dispatch queue.
    - Matching: every key of a match dictionary must be present and `CFEqual` in the description.
    - A description-changed callback with a watch array runs only when a changed key is in it.
    - Registering an appeared callback queues appeared calls for the disks the session already knows.
  - **Reconnecting.** On `XPC_ERROR_CONNECTION_INTERRUPTED` the client says `"hello"` again and reconciles: disks it
    knew that are missing from the new snapshot get disappeared, new ones get appeared.
  - **API.** `DASessionScheduleWithRunLoop`, `DASessionUnscheduleFromRunLoop`, `DASessionSetDispatchQueue` (NULL
    unschedules), `DARegisterDiskAppearedCallback`, `DARegisterDiskDisappearedCallback`,
    `DARegisterDiskDescriptionChangedCallback` and `DAUnregisterCallback`.

- [ ] **Step 1: Write the failing test.** `da-callbacks.c <mode> …`. Cases:
  - `existing`: register appeared with match `kDADiskDescriptionMatchVolumeMountable`, schedule, run 2 s. The mounted
    image's leaf is reported exactly once and the whole disk not at all.
  - `attach-detach <image>`: register all three callbacks (no match), schedule on a dispatch queue, fork and exec
    `hdiutil attach -quiet`, then `hdiutil detach -quiet`. Expected order: appeared `diskN`, appeared `diskNs1`, then
    on detach changed `diskNs1` with keys containing `DAVolumePath`, disappeared `diskN`, disappeared `diskNs1`.
  - `unscheduled`: a registered but unscheduled session gets nothing in 2 s.
  - `unregister`: after `DAUnregisterCallback` nothing more is delivered.
  - `daemon restart`: `kill -TERM` the daemon's pid (from `launchctl list` or `pgrep`), wait 2 s, attach. The session
    still gets appeared for the new disk (Review Focus 5).
- [ ] **Step 2: Run it.** Expected: FAIL. Callbacks are never called.
- [ ] **Step 3: Implement** as in Interfaces.
- [ ] **Step 4: Run it** in all three modes with the Task 3 tests. Expected: PASS.
- [ ] **Step 5: Commit:** "DiskArbitration: sessions and disk callbacks".

### Task 5: unmount, eject, approvals and dissenters

**Files:**
- Modify:
  - `DADisk.c`: `DADiskUnmount`, `DADiskEject`, `DADiskMount` and `DADiskMountWithArguments`;
  - `DASession.c`: the approval registrations;
  - `DADissenter.c` (status strings across processes);
  - `DAProtocol.h`, `DAServer.c`, `DAModel.c`.
- Test: `tests/da-ops.c` (guest); cases in `tests/diskarbitration.sh`.

**Interfaces:**
- Consumes: Tasks 3–4.
- Produces:
  - **Protocol.**
    - `"unmount"` {`"name"`, `"options"`} and `"eject"` {`"name"`, `"options"`} → {`"status"`, optional `"string"`}.
    - `"approve"` request from the daemon {`"kind"`: `"unmount"`|`"eject"`, `"name"`, `"description"`} → reply
      {`"status"`, optional `"string"`}.
    - `"hello"` carries `"approvals"`, a flag saying the session has approval callbacks.
  - **Unmount (daemon).** Checks run in this order:
    1. an unknown name → `kDAReturnBadArgument`;
    2. a requester euid that is neither 0 nor the volume owner (`f_owner`) → `kDAReturnNotPrivileged`;
    3. a whole disk without `kDADiskUnmountOptionWhole` → `kDAReturnNotMounted`;
    4. a leaf that is not mounted → `kDAReturnNotMounted`;
    5. a whole disk with the Whole option and nothing mounted → success.

    Then the approval round runs once per mounted leaf. Every connection with approvals gets the request. The first
    dissent wins, unless `kDADiskUnmountOptionForce` is set, in which case the round still runs but dissent is ignored.
    The round times out after 10 s per round, and a connection that goes away counts as approval.

    The daemon then calls `unmount(2)`, with `MNT_FORCE` for Force. On failure the status is `0xC000 | errno`, marked
    `QUIRK:` with evidence `0xC010` for EBUSY (Mac, tier 2) and the test `da-ops busy-file`.

    On success:
    - the leaf stays as unmounted: `DAVolumePath` is removed and `"changed"` is sent with that key;
    - the mount point is cleaned up as in Task 3;
    - a later `EVFILT_FS` rescan does not turn this into a disappearance.
  - **Eject (daemon).** Checks run in this order:
    1. unknown → `kDAReturnBadArgument`;
    2. a leaf → `kDAReturnUnsupported`;
    3. not privileged → `kDAReturnNotPrivileged`;
    4. not ejectable (`disk0`) → `kDAReturnUnsupported`;
    5. a leaf still mounted → `0xC010`.

    Then an approval round runs for the whole disk, with no force. Success removes the records and sends disappeared
    whole then leaf. An image volume's FUSE process has already exited on unmount, so ejecting only forgets the disk.
  - **Mount.** `DADiskMount` and `DADiskMountWithArguments` complete with `kDAReturnUnsupported`. Re-mounting an
    ejected image needs the image layer, which DiskArbitration must not call.
  - **Client.**
    - `DADiskUnmount` and `DADiskEject` send asynchronously. The completion callback is delivered on the session's
      run loop or queue with NULL for success, otherwise a `DADissenterRef` carrying the status and string.
    - Approval requests run the matching callbacks of `DARegisterDiskUnmountApprovalCallback` or
      `DARegisterDiskEjectApprovalCallback` on the run loop or queue and reply with the first dissenter.
    - With no matching callback, or if the session is unscheduled, the client approves at once.

- [ ] **Step 1: Write the failing test.** `da-ops.c <mode> …` with modes:
  - `deny <seconds>`: register unmount and eject approval callbacks that print `approval asked for <name>` and dissent
    `kDAReturnBusy` "in use by test";
  - `hang`: the callbacks sleep 30 s;
  - `unmount <name> <options>`, `eject <name>`, `mount <name>`, `busy-file <mp>` (keeps a file open while unmounting
    the leaf without Force).

  The cases in `diskarbitration.sh` follow the Mac session that pinned the Global Constraints values:
  - leaf eject → `f8da000c`;
  - unknown `diskNs9` unmount → `f8da0003`;
  - whole eject while mounted → `c010`;
  - busy file → `c010`;
  - leaf unmount → ok, then the mount point is gone;
  - a second unmount → `f8da0007`;
  - whole unmount with option 1 when nothing is mounted → ok;
  - eject → ok;
  - a second eject → `f8da0003`;
  - with `deny` running:
    - unmount → `f8da0002` with the string "in use by test";
    - whole unmount with option 1 → the same;
    - eject → the same;
    - unmount with `0x80000` → ok, and `deny` logged a request for the leaf;
  - `mount` → `f8da000c`;
  - "dead approver" (Review Focus 2): `hang` running, its unmount finishes ok within 15 s; then `deny` killed with
    -KILL mid-session, and the next unmount finishes ok at once;
  - "volume vanishes during approval" (Review Focus 1): with `hang` running, start an unmount, then kill the volume's
    `darling-dmg-mount`. The unmount finishes with `f8da0007` or `f8da0003` within 15 s, and the disk is gone.
- [ ] **Step 2: Run it.** Expected: FAIL. The functions are missing; the link fails or the test reports FAILs.
- [ ] **Step 3: Implement** as in Interfaces.
- [ ] **Step 4: Run it** in all three modes with all earlier tests. Expected: PASS.
- [ ] **Step 5: Commit:** "DiskArbitration: unmount, eject and dissenters".

### Task 6: hdiutil reports and detaches through DiskArbitration

**Files:**
- Modify:
  - `src/external/darling-dmg/src/main-hdiutil.cpp`;
  - `include/darling-dmg/darling_dmg.h` and `src/darling_dmg.cpp` (partition scheme);
  - `CMakeLists.txt` (link DiskArbitration for hdiutil).
- Test:
  - `tests/dmg-attach.sh` and `tests/dmg-license.sh`: they take the mount point from the last tab-separated field
    instead of the whole last line;
  - new cases in `tests/dmg-attach.sh`;
  - fixtures `tests/fixtures/apm.dmg` (made on the Mac with `hdiutil create -size 2m -fs HFS+ -layout SPUD -volname
    APMv`) and `tests/fixtures/nolayout.dmg` (`-layout NONE -volname NoLayout`), recorded in `dmg-attach.md`.

**Interfaces:**
- Consumes: DiskArbitration (Tasks 3–5).
- Produces:
  - **`const char* darling_dmg_partition_scheme(darling_dmg_image*)`** returns `"GUID_partition_scheme"`,
    `"Apple_partition_scheme"` or `""` (no map).
  - **`hdiutil attach`** looks up the disks with `DADiskCreateFromVolumePath` and `DADiskCopyWholeDisk`.
    - It prints the table in the observed format: the whole disk with the scheme hint, then the leaf with `Apple_HFS`
      and the mount point.
    - `-plist` lists the leaf entry, then the whole-disk entry, with the observed keys.
    - If DiskArbitration does not know the volume, it prints the mount point alone, as now.
    - An image with no map still gets two lines (whole plus `s1`); that is a ruling.
  - **`hdiutil detach <mp|/dev/diskN|/dev/diskNsM>`**:
    - resolves the disk through DiskArbitration; unknown → `detach failed - No such file or directory`, exit 1;
    - runs `DADiskUnmount(whole, Whole | Force if -force)`, then `DADiskEject(whole)`;
    - on a failure prints the dissenter string to stdout with no newline, if there is one;
    - then prints `hdiutil: couldn't unmount|eject "diskN" - <strerror>` to stderr. The status maps to errno as
      follows: kDAReturnBusy → EBUSY, `0xC000|e` → e, BadArgument → EINVAL, NotPrivileged → EPERM, NotMounted →
      EINVAL, Unsupported → ENOTSUP, otherwise EIO. The exit status is `status & 0xFF`;
    - success prints `"diskN" ejected.`;
    - `-quiet` silences stdout and these messages (Plan 1 behaviour).

- [ ] **Step 1: Write the failing tests** in `dmg-attach.sh`:
  - "attach table": the last two lines match `^/dev/disk[0-9]+ {0,10}\tGUID_partition_scheme {10}\t$` and
    `^/dev/disk[0-9]+s1 {0,8}\tApple_HFS {22}\t/Volumes/Unmount Test$`, and the line widths match the observed format;
  - "apm scheme": `apm.dmg` gives the whole-disk hint `Apple_partition_scheme`;
  - "-plist dev entries": the leaf `dev-entry` equals `/dev/` + the BSD name `da-disks` reports for the mount point,
    and the whole entry comes second with `potentially-mountable` false;
  - "detach by device": `hdiutil detach /dev/diskN` prints `"diskN" ejected.`;
  - "detach unknown device": `/dev/disk99` → stderr `hdiutil: detach failed - No such file or directory`, exit 1;
  - "detach with a dissenter": `da-ops deny 20` in the background → stdout `in use by test`, stderr
    `hdiutil: couldn't unmount "diskN" - Resource busy`, exit 2;
  - "-force with a dissenter" → `hdiutil: couldn't eject "diskN" - Resource busy`, exit 2, and the volume is
    unmounted.

  Update `unmount-disk-image.sh` and `dmg-license.sh` only where they read the attach output, and keep their
  expectations otherwise.
- [ ] **Step 2: Run them.** Expected: FAIL (the old single-line output; detach does not know `/dev/` names).
- [ ] **Step 3: Implement.** Add the scheme API with a host unit test case in `dmg-crafted-host.cpp` or a new
  `tests/dmg-partition-scheme-host.cpp` for the three fixtures. Then make the `hdiutil` changes, deleting the
  hard-coded `/dev/disk1` output and `detachName`.
- [ ] **Step 4: Run** the full test list (`tests/dmg-attach.sh tests/dmg-license.sh tests/unmount-disk-image.sh
  tests/volume-attributes.sh tests/diskarbitration.sh`) in all three modes, and the host tests. Expected: PASS
  everywhere; rootless keeps its one documented SKIP.
- [ ] **Step 5: Commit** in darling-dmg: "hdiutil: disk names and detach through DiskArbitration". In darling, commit
  the tests, fixtures and bump. Push the darling-dmg, darling-xnu and darling branches. Mark plan 3 done in the
  roadmap.
