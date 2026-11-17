# Volumes, eject, trash, icons and reveal (Foundation / AppKit) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the volume, eject, trash, icon and reveal APIs work for any app: the Foundation and CoreFoundation ones, and NSWorkspace's. Finder (plan 7) and DiskImageMounter (plan 6) are built on them.

**Architecture:**
- **What each layer owns, following the macOS dependency graph** (observed with `dyld_info -dependents` on macOS 26.7.3):
  - CoreFoundation links neither DiskArbitration nor IOKit, so CF's volume resource keys come from `statfs`, its mount flags and `getattrlist`.
  - Foundation and AppKit both link DiskArbitration.
  - AppKit links CoreServices.
- **Flags:** darling-xnu's `statfs`/`getfsstat` gain the mount flags CF reads: `MNT_LOCAL`, `MNT_ROOTFS`, `MNT_DONTBROWSE`, `MNT_REMOVABLE`.
- **Foundation:** enumerates volumes with `getfsstat`, and trashes into per-volume trash directories.
- **AppKit's NSWorkspace:**
  - builds its volume calls on Foundation;
  - ejects through DiskArbitration;
  - turns DiskArbitration callbacks into the mount notifications;
  - draws icons from types and volumes;
  - reveals files by sending the file viewer (`com.apple.finder`, or the `NSFileViewer` default) the `aevt`/`odoc` event macOS 26 sends for a reveal (bookmark-data items plus `'aapd'` = true), through a new `LSOpenApplication` that supports `initialEvent`.

**Tech Stack:** C (darling-xnu, CoreServices), Objective-C (CoreFoundation's NSURL.m, Foundation, AppKit/cocotron), shell test drivers.

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md` (section "NSWorkspace / Foundation").

## Global Constraints

- AGENTS.md governs everything, in particular:
  - layering (§2): CoreFoundation must not call DiskArbitration, and nothing above `libsystem_kernel` parses the mount table;
  - `QUIRK:` comments with evidence and a test (§3);
  - no verbatim copying (§4);
  - tests from the documented contract (§8);
  - a Provenance note in every commit (§7).
- Anti-goal: any app-specific logic: no checks on bundle IDs, app names, DMG names or specific background images. The one bundle ID named here, `com.apple.finder`, is the receiver of the reveal event (overridable by the `NSFileViewer` default), not a check on the caller.
- Anti-goal: re-implementing lower-layer functionality privately in a higher layer.
- All three launcher modes (root, rootless, non-root) must support mount, eject and the end-to-end flow. Every guest test here runs in all three.
- Build tree `~/Projects/darling/build`. Install only into `~/darling-finder`. Re-apply setuid to `~/darling-finder/bin/darling` after an install (`chown root:root`, `chmod 4755`).
- Push branches. **No pull requests** until the whole roadmap is done.
- Branches:
  - CoreFoundation, Foundation and cocotron: `finder-volumes`, created from their current HEAD.
  - xnu: keep using `volume-attributes`.
  - darling: `finder-app`. It records the submodule bumps; never `git add -A`.
- Values a test expects that the documentation doesn't pin come from black-box runs on macOS 26.7.3. They are recorded in `tests/volumes.md` (tier-2 evidence).
- Artwork:
  - Apple artwork is never used.
  - New generic icons follow AppKit's precedent: rendered at 32×32 from the Adwaita icon theme, listed in `AppKit/SystemImages/COPYING.Adwaita`.

## Observed on macOS 26.7.3 (the probes behind this plan)

**Volume keys:**

| Volume | Name | Ejectable / Removable | ReadOnly | Local | Internal | Browsable | RootFS | FormatDescription | TypeName | MountFromLocation |
|--------|------|-----------------------|----------|-------|----------|-----------|--------|-------------------|----------|-------------------|
| `/Volumes/Unmount Test` (UDZO) | "Unmount Test" | YES / YES | YES | YES | nil | YES | NO | "Mac OS Extended" | "hfs" | "disk5s1" |
| `/` | "System" | NO / NO | YES | YES | YES | YES | YES | "APFS" | "apfs" | "disk2s4s1" |

- A path inside a volume answers with its volume's values.
- `NSURLVolumeURLKey` is the mount point as a directory URL, and `NSURLVolumeIdentifierKey` is 8 bytes of data.

**`statfs` flags:**

| Volume | Flags |
|--------|-------|
| a disk image | `MNT_REMOVABLE\|MNT_LOCAL` (plus `MNT_RDONLY` when read-only) |
| `/` | `MNT_LOCAL\|MNT_ROOTFS` |
| `/System/Volumes/VM` | `MNT_DONTBROWSE` |
| an autofs mount | `MNT_DONTBROWSE\|MNT_AUTOMOUNTED`, no `MNT_LOCAL` |

**Enumeration:**
- `mountedVolumeURLsIncludingResourceValuesForKeys:options:0` lists every mounted volume, root first.
- With `NSVolumeEnumerationSkipHiddenVolumes` only `/` and the image are left.
- `nil` keys are accepted.

**NSWorkspace volume calls:**
- `mountedLocalVolumePaths` is every volume path from that list (it even includes autofs).
- `mountedRemovableMedia` gives only the image.
- `getFileSystemInfoForPath:`:

| Path | removable | writable | unmountable | description, type |
|------|-----------|----------|-------------|-------------------|
| the image | 1 | 0 | 1 | "hfs", "hfs" |
| `/` | 0 | 0 | 0 | "apfs", "apfs" |
| a folder in `/tmp` | 0 | 1 | 0 | "apfs", "apfs" |

**`unmountAndEjectDeviceAtURL:error:`:**

| Target | Result |
|--------|--------|
| `/nonexistent` | NO, `NSOSStatusErrorDomain` −36 |
| `/tmp`, `/` | NO, −47 |
| the image, with a dissenter running | NO, −47, empty userInfo |
| a file inside the volume | YES: its volume is ejected |

`unmountAndEjectDeviceAtPath:` returns the same answer as a BOOL.

**Notifications** go to `NSWorkspace.sharedWorkspace.notificationCenter`, on the main thread. The object is the workspace. The userInfo has:
- `NSDevicePath`: the mount point as a string;
- `NSWorkspaceVolumeURLKey`: the mount point as a directory URL;
- `NSWorkspaceVolumeLocalizedNameKey`: the volume name.

They arrive as follows:

| Event | Notifications |
|-------|---------------|
| `hdiutil attach` | DidMount |
| `hdiutil detach` | WillUnmount, then DidUnmount |
| our own synchronous eject, on the main thread | WillUnmount, then DidUnmount, both before the call returns |
| an eject that is dissented | WillUnmount only |

**Trash:**
- `URLForDirectory:NSTrashDirectory inDomain:NSUserDomainMask appropriateForURL:nil` is `~/.Trash`.
- For a URL on another volume it is `<mount point>/.Trashes/<uid>`. If that directory doesn't exist yet, the answer is `NSFeatureUnsupportedError` (3328) with an underlying POSIX ENOENT. A read-only volume gives 3328 with an underlying EROFS.
- `trashItemAtURL:` errors:

| Item | Error |
|------|-------|
| an item on another volume | none: it goes to that volume's `.Trashes/<uid>` |
| a missing file | `NSFileNoSuchFileError` (4) |
| a file on a read-only volume | 3328 |
| an `http:` URL | `NSFileReadUnsupportedSchemeError` (262) |

- macOS renames a colliding item `name HH-MM-SS-mmm.ext`. Darling keeps its documented-safe `name N.ext` (see Task 3).
- `recycleURLs:completionHandler:` calls its handler on the main thread. `newURLs` lists only the items that moved. When one item fails, the error has code 4 (that failure's code) and `NSUnderlyingErrors`.

**Icons** (all 32×32; compared by pixels):

| Item | Icon |
|------|------|
| a missing path, `""`, an unknown extension, no extension | the `public.data` type icon |
| a folder | the `public.folder` icon |
| `a.txt` | the `txt` icon |
| a `.dmg` (`com.apple.disk-image-udif`, which conforms to `public.disk-image`) | its own icon |
| an `.app` without an icon | a generic application icon |
| `/` | an internal-disk icon |
| the image's mount point | a removable-volume icon, different from `/` |

`iconForFiles:` gives:
- `@[]`: nil;
- one file: the same pixels as `iconForFile:`;
- two files: `NSImageNameMultipleDocuments`.

### Reveal (user-supplied macOS 26 observation; see the spec's "Reveal requests")

| Call | Event the file viewer gets |
|------|----------------------------|
| `selectFile:inFileViewerRootedAtPath:` | `aevt`/`odoc`, a list of 1 bookmark-data item, `'aapd'` true |
| `activateFileViewerSelectingURLs:` | one `aevt`/`odoc` with all items in order, `'aapd'` true |
| `selectFile:nil inFileViewerRootedAtPath:<dir>` | `aevt`/`rapp`, no items |

The root path is never sent. The file viewer is found by bundle ID: the `NSFileViewer` default, else `com.apple.finder`.

## Review Focus

1. **Volume names that need escaping** (`fixtures/slash-name.dmg` is "A/B" at `/Volumes/A:B`; `fixtures/amp-name.dmg` is "Tom & Jerry <1>"):
   - NSURLVolumeNameKey gives the real name.
   - The notifications' `NSDevicePath` is the mount point.
   - Their localized name is the volume name.
   - Tests: Task 2 and Task 5.
2. **A volume that goes away between enumeration and a key lookup:** a fresh NSURL for its old mount point answers from the volume underneath (`/`), and nothing crashes (Task 2).
3. **An app ejecting synchronously on its own main thread** gets its own WillUnmount and DidUnmount before the call returns, and the call takes well under the 10 s approval timeout (Task 5).
4. **Trash collisions** for a name without an extension, a dot file and a folder: each gets a fresh name, and the original stays in the trash (Task 3).
5. **Ejecting while another process dissents:**
   - NO with −47;
   - the volume stays mounted;
   - no DidUnmount;
   - a later eject without the dissenter works (Task 4 for the result, Task 5 for the notifications).

---

## Files

| File | Responsibility |
|------|----------------|
| xnu `…/bsd/helper/stat/guest_mount.c` | the Darling mount flags in `statfs`/`fstatfs`/`getfsstat` |
| xnu `tests/volume-flags.c` (new) | prints a path's statfs flags |
| corefoundation `NSURL.m` | the volume resource keys |
| corefoundation `tests/nsurl-volume-keys.m` (new) | prints the volume keys for its arguments |
| foundation `include/Foundation/NSPathUtilities.h`, `src/NSFileManager.m` | `NSTrashDirectory`, volume enumeration, per-volume trash |
| foundation `tests/filemanager-volumes.m`, `tests/filemanager-trash-volumes.m` (new) | enumeration; trash |
| CoreServices `src/LaunchServices/LaunchServices.c`, `src/LaunchServices/LSOpen.m` | `LSOpenApplication` with `initialEvent` |
| darling `tests/src/launchservices_open_application.m`, `tests/src/ae-receiver.c` (new) | the LS test; a test bundle that records the Apple Events it receives |
| cocotron `AppKit/MacWorkspace.m` | volume calls, eject, icons, recycle, reveal |
| cocotron `AppKit/NSWorkspaceVolumeEvents.{h,m}` (new) | the DiskArbitration session that posts mount notifications |
| cocotron `AppKit/include/AppKit/NSWorkspace.h` | the missing declarations |
| cocotron `AppKit/CMakeLists.txt` | DiskArbitration dependency; new images |
| cocotron `AppKit/SystemImages/` | `NSVolumeInternal.png`, `NSVolumeRemovable.png`, `NSDiskImage.png`, `NSMultipleDocuments.png`; `COPYING.Adwaita` rows |
| cocotron `tests/nsworkspace-volumes.m`, `tests/nsworkspace-mount-notifications.m`, `tests/nsworkspace-icon-kinds.m`, `tests/nsworkspace-recycle.m`, `tests/nsworkspace-reveal.m` (new) | AppKit tests |
| darling `tests/volumes.sh`, `tests/volumes.md` (new; `git add -f`) | the driver for every guest case in this plan; the macOS observations above |

**The driver** `tests/volumes.sh` copies the structure of `tests/diskarbitration.sh`: `guest`, `rc`, `outputs`, `pass`, `fail`, `$work`, `$gw`, and a `build <name> <source> <frameworks…>` with guest-cc.sh. It finds the submodule tests through `$here/../src/external/<repo>/tests/`. Each task adds its cases to it. Every case attaches its fixtures itself and detaches them at the end.

Run it in one mode with `TESTS=tests/volumes.sh <scratch>/helpers/matrix-plan2.sh <mode>`, and in all modes with `TESTS=tests/volumes.sh <scratch>/helpers/all-modes.sh`. The plan writes these as `run-volumes <mode>` and `run-volumes all`.

---

### Task 1: Mount flags in darling-xnu

**Files:**
- Modify: xnu `darling/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/helper/stat/guest_mount.c` (`guest_mount_describe`, `list_visit`)
- Create: xnu `tests/volume-flags.c`
- Test: darling `tests/volumes.sh` (created here), `tests/volumes.md` (created here, with the "Observed" section above)

**Interfaces:**
- Produces, for every `statfs`, `fstatfs`, `getfsstat` and `getmntinfo` entry:

| Flag | Set for |
|------|---------|
| `MNT_LOCAL` (0x1000) | every volume, except Linux network types: `nfs`, `nfs4`, `cifs`, `smb3`, `9p`, `ceph`, `fuse.sshfs` |
| `MNT_ROOTFS` (0x4000) | the guest root volume (`/`) |
| `MNT_DONTBROWSE` (0x100000) | every volume that is neither `/` nor mounted directly in `/Volumes` |
| `MNT_REMOVABLE` (0x200) | every volume whose type comes from its `darling.volume.kind` marker, i.e. a disk image's, the same rule `diskarbitrationd` uses |

  Plan 3's flags (RDONLY, NOSUID, NODEV, NOEXEC, SYNCHRONOUS) stay as they are.
- `tests/volume-flags.c`: `volume-flags <path>…` prints one line per path, `<path>: <f_mntonname> <hex flags> <names>`. The names are the set flags among `LOCAL ROOTFS DONTBROWSE REMOVABLE RDONLY`, in that order. It also builds on macOS.

- [ ] **Step 1: Write the failing test.** Create `tests/volumes.sh` and `tests/volumes.md`, plus the case "mount flags":
  - attach `fixtures/unmount-test.dmg`;
  - run `volume-flags / '/Volumes/Unmount Test' /proc`;
  - expect `/` to have `LOCAL ROOTFS` and no `REMOVABLE`/`DONTBROWSE`;
  - expect the image to have `LOCAL REMOVABLE RDONLY` and no `ROOTFS`/`DONTBROWSE`;
  - expect `/proc` to have `DONTBROWSE`;
  - detach.
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: `FAIL: mount flags`, with `/` showing none of the new flags.
- [ ] **Step 3: Implement it.** Add `static unsigned int guest_mount_flags(const struct guest_root* root, const char* mnton, const char* guest_name, const char* linux_fstype, int has_kind)`. It returns the bits from the table, which both `guest_mount_describe` and `list_visit` OR into `f_flags`. `has_kind` is whether `guest_mount_kind` replaced the type: make it return 1 when it did.
- [ ] **Step 4: Run it to see it pass.** Rebuild `system_kernel`, run `cmake -P src/external/xnu/darling/src/libsystem_kernel/cmake_install.cmake`, then `run-volumes all`. Expected: `PASS: mount flags` in all three modes. Also run `TESTS="tests/volume-attributes.sh tests/diskarbitration.sh" <scratch>/helpers/all-modes.sh`. Expected: no new FAIL.
- [ ] **Step 5: Commit.**
  - In xnu, on `volume-attributes`: "statfs: local, root, browse and removable mount flags".
  - In darling: the bump plus `git add -f tests/volumes.sh tests/volumes.md`.
  - Both with a Provenance note: macOS statfs observations, no Apple code.

### Task 2: NSURL volume resource keys (CoreFoundation)

**Files:**
- Modify: corefoundation `NSURL.m` (the `CFURLCreatePropertyForKey` key switch, ~:437-811)
- Create: corefoundation `tests/nsurl-volume-keys.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: Task 1's flags.
- Produces: the keys Tasks 3–6 read. A path answers for the volume that contains it (`statfs` of the path, `getattrlist` of `f_mntonname`):

| Key | Source |
|-----|--------|
| `NSURLVolumeNameKey`, `NSURLVolumeLocalizedNameKey` | `ATTR_VOL_NAME` |
| `NSURLVolumeUUIDStringKey` | `ATTR_VOL_UUID` (absent when the UUID is all zeros) |
| `NSURLVolumeIsRemovableKey`, `NSURLVolumeIsEjectableKey` | `MNT_REMOVABLE` (the ejectable inference is ruled below) |
| `NSURLVolumeIsReadOnlyKey` | `MNT_RDONLY` |
| `NSURLVolumeIsLocalKey` | `MNT_LOCAL` |
| `NSURLVolumeIsInternalKey` | YES when local and not removable, otherwise nil |
| `NSURLVolumeIsBrowsableKey` | not `MNT_DONTBROWSE` |
| `NSURLVolumeIsRootFileSystemKey` | `MNT_ROOTFS` |
| `NSURLVolumeIsAutomountedKey` | `MNT_AUTOMOUNTED` |
| `NSURLVolumeLocalizedFormatDescriptionKey` | "Mac OS Extended" for `hfs` (with " (Journaled)" when `MNT_JOURNALED`), "APFS" for `apfs`, otherwise `f_fstypename` |
| `NSURLVolumeTypeNameKey` | `f_fstypename` |
| `NSURLVolumeSubtypeKey` | `f_fssubtype` |
| `NSURLVolumeMountFromLocationKey` | `f_mntfromname` without a leading `/dev/` |
| `NSURLVolumeURLKey` | `f_mntonname` as a directory file URL |
| `NSURLVolumeIdentifierKey` | the 8 bytes of `f_fsid` |
| `NSURLIsVolumeKey` | YES when the path's resolved path equals `f_mntonname` |

  Booleans are `kCFBooleanTrue`/`kCFBooleanFalse`. A URL that isn't a file URL, or whose path doesn't exist, keeps today's behaviour (error or nil, as the switch already does).
- `tests/nsurl-volume-keys.m`: `nsurl-volume-keys <path>…` prints `key = value` for each of these keys, nil as `nil`, sorted by key name. It also builds on macOS.

- [ ] **Step 1: Write the failing test.** Add "volume keys" to `tests/volumes.sh`:
  - attach `unmount-test.dmg`, `slash-name.dmg` and `amp-name.dmg`;
  - run the program for `/`, `/Volumes/Unmount Test/`, `/Volumes/Unmount Test/hello.txt`, `/Volumes/A:B` and `/Volumes/Tom & Jerry <1>`;
  - compare against the Interfaces table and the Observed table.

  The values for `/Volumes/Unmount Test`:

| Key | Value |
|-----|-------|
| Name | "Unmount Test" |
| Ejectable, Removable, ReadOnly, Local, Browsable | 1 |
| RootFileSystem | 0 |
| Internal | nil |
| FormatDescription | "Mac OS Extended" |
| TypeName | "hfs" |
| URL | `file:///Volumes/Unmount%20Test/` |
| IsVolume | 1 (and 0 for `hello.txt`, which otherwise matches the volume) |

  The other volumes:
  - `/`: Ejectable 0, Internal 1, RootFileSystem 1, IsVolume 1.
  - `/Volumes/A:B`: Name "A/B".
  - `/Volumes/Tom & Jerry <1>`: Name "Tom & Jerry <1>".

  Review Focus 2 ("volume keys after detach"): detach the image, then run the program again for `/Volumes/Unmount Test` (now missing) and for `/Volumes`. Expected: no crash (exit 0); `/Volumes` reports `/`'s volume (URL `file:///`).
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: `FAIL: volume keys`, with Name `nil`.
- [ ] **Step 3: Implement the keys** as new branches in the switch. They share one helper, `static Boolean volumeInfoForURL(CFURLRef url, struct statfs* outStat, char* outName, size_t nameSize, uuid_t outUUID)`. Fill the empty `NSURLVolumeURLKey`, `NSURLVolumeIdentifierKey` and `NSURLIsVolumeKey` branches. Use `kCFURLVolume*Key` constants that already exist (CFURLConstants.c), and the NSURL.m strings for the newer keys.
- [ ] **Step 4: Run it to see it pass.** Rebuild the `CoreFoundation_*` targets, run `cmake -P src/external/corefoundation/cmake_install.cmake`, then `run-volumes all`. Expected: PASS in all three modes. The existing `corefoundation/tests/nsurl-resource-key-constants.m` still passes in the guest.
- [ ] **Step 5: Commit.**
  - corefoundation `finder-volumes`: "NSURL: volume resource keys from statfs and getattrlist".
  - darling bump and the driver.
  - Provenance: the public NSURL key documentation, plus the macOS observations.

  Ruling recorded in the commit message: Ejectable equals Removable, because every removable Darling volume is a disk image, which DiskArbitration reports as ejectable, and CoreFoundation cannot ask DiskArbitration.

### Task 3: Foundation: mounted volumes, NSTrashDirectory, per-volume trash

**Files:**
- Modify: foundation `include/Foundation/NSPathUtilities.h` (add `NSTrashDirectory = 102`), `src/NSFileManager.m` (`mountedVolumeURLsIncludingResourceValuesForKeys:options:` :361, `URLForDirectory:…` :525, `URLsForDirectory:inDomains:` :509, `trashItemAtURL:resultingItemURL:error:` :1561)
- Create: foundation `tests/filemanager-volumes.m`, `tests/filemanager-trash-volumes.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: Task 2's keys (prefetching only; enumeration uses `getfsstat` directly).
- Produces:
  - **`mountedVolumeURLsIncludingResourceValuesForKeys:(NSArray*)keys options:(NSVolumeEnumerationOptions)options`:**
    - Returns one directory file URL per `getfsstat(MNT_NOWAIT)` entry, in that order.
    - With `NSVolumeEnumerationSkipHiddenVolumes`, entries flagged `MNT_DONTBROWSE` are skipped.
    - `NSVolumeEnumerationProduceFileReferenceURLs` is accepted and ignored.
    - Each URL's `keys` are fetched with `resourceValuesForKeys:error:`, so they are cached on it. `nil` keys are allowed.
    - Returns `nil` only when `getfsstat` fails.
  - **`URLForDirectory:NSTrashDirectory inDomain:NSUserDomainMask appropriateForURL:create:error:`:**
    - With no URL, or one on the home volume (same `f_fsid` as `NSHomeDirectory()`): `~/.Trash`, created with mode 0700 when `create` is YES.
    - Otherwise: `<f_mntonname>/.Trashes/<getuid()>`. With `create`, it makes `.Trashes` (mode 01333) and the uid directory (0700).
    - When the directory is missing and not created, or can't be created: nil with `NSFeatureUnsupportedError` (3328) and an `NSUnderlyingErrorKey` POSIX error (ENOENT, EROFS, …).
    - `URLsForDirectory:NSTrashDirectory inDomains:NSUserDomainMask` gives `@[~/.Trash]`.
  - **`trashItemAtURL:resultingItemURL:error:`** moves the item into the trash `URLForDirectory:…appropriateForURL:item create:YES` gives. It keeps today's collision naming and its error mapping (missing item → 4). New cases:
    - a non-file URL → `NSFileReadUnsupportedSchemeError` (262);
    - a trash that can't be had → that 3328 error, with `NSURLErrorKey` set to the item.

- [ ] **Step 1: Write the failing tests.**
  - **`filemanager-volumes.m`**, with no arguments:
    - with options 0: the list starts with `file:///` and contains `file:///Volumes/Unmount%20Test/`;
    - with SkipHidden: the same two, without `file:///proc/`;
    - with options 0: `/proc` is present;
    - `nil` keys work;
    - prefetching `NSURLVolumeNameKey` leaves the name in `-[NSURL getResourceValue:…]` for the image.
  - **`filemanager-trash-volumes.m <home-volume dir> <other-volume dir> <read-only volume file>`:** the driver passes `$HOME/<tmp dir>` in the guest, a fresh directory under `/Volumes/SystemRoot$work` (the host file system is another volume in every mode), and `/Volumes/Unmount Test/hello.txt`. It checks:
    - `NSTrashDirectory` is `~/.Trash`;
    - before anything is trashed there, the other volume's trash URL with `create:NO` is 3328 with underlying ENOENT;
    - trashing a file on the other volume gives `<mount point>/.Trashes/<uid>/<name>`, and the source is gone;
    - the read-only file gives 3328;
    - `http://example.com/x` gives 262;
    - a missing file gives 4.

    Review Focus 4: two files called `noext`, two called `.hidden` and two folders called `a folder`, each trashed into the home trash. The second of each gets a different name than the first, both exist in the trash, and the result URL is the moved item. The program removes what it trashed.
  - The driver removes `<host mount point>/.Trashes/<uid>` after the case, and `.Trashes` too if it created it. It records the host mount point it used in the case output.
- [ ] **Step 2: Run them to see them fail.** Run `run-volumes rootless`. Expected: `FAIL: mounted volumes` (nil list) and `FAIL: trash on other volumes` (EXDEV error).
- [ ] **Step 3: Implement** the three methods per the Interfaces. Volume of an item: `statfs` of the item (`lstat` first, so a missing item gives error 4 before any volume lookup).
- [ ] **Step 4: Run them to see them pass.** Rebuild the `Foundation_*` targets, run `cmake -P src/external/foundation/cmake_install.cmake`, then `run-volumes all`. Expected: PASS in all three modes. The existing `foundation/tests/filemanager-*.m` trash tests (if any; `grep -l trashItem foundation/tests`) still pass.
- [ ] **Step 5: Commit.**
  - foundation `finder-volumes`: "NSFileManager: mounted volumes, NSTrashDirectory, trash on other volumes".
  - darling bump and driver.
  - Provenance: documentation and the macOS observations.

  Ruling recorded in the commit message: collision names stay `name N.ext`, which the documentation allows ("may have a different name"). macOS's `name HH-MM-SS-mmm.ext` is an undocumented detail no caller can depend on, so it is not a needed quirk.

### Task 4: NSWorkspace volume calls and eject

**Files:**
- Modify: cocotron `AppKit/MacWorkspace.m` (`mountedLocalVolumePaths`, `mountedRemovableMedia`, `getFileSystemInfoForPath:…`, `unmountAndEjectDeviceAtPath:`), `AppKit/include/AppKit/NSWorkspace.h` (declare `unmountAndEjectDeviceAtURL:error:`), `AppKit/CMakeLists.txt` (add `DiskArbitration` to DEPENDENCIES)
- Create: cocotron `tests/nsworkspace-volumes.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: Task 3's enumeration and Task 2's keys. DiskArbitration (plan 3):
  - `DASessionCreate`, `DADiskCreateFromVolumePath`, `DADiskCopyWholeDisk`;
  - `DADiskUnmount(whole, kDADiskUnmountOptionWhole, …)`, `DADiskEject`;
  - `DASessionScheduleWithRunLoop`.
- Produces:
  - **`mountedLocalVolumePaths`:** every path of `mountedVolumeURLsIncludingResourceValuesForKeys:nil options:0`.
  - **`mountedRemovableMedia`:** those whose `NSURLVolumeIsRemovableKey` is YES.
  - **`getFileSystemInfoForPath:isRemovable:isWritable:isUnmountable:description:type:`:**
    - NO for nil or a path that doesn't exist;
    - otherwise removable = IsRemovable, writable = not IsReadOnly, unmountable = IsRemovable, description = type = NSURLVolumeTypeNameKey.
  - **`- (BOOL)unmountAndEjectDeviceAtURL:(NSURL*)url error:(NSError**)error`:**
    1. A path that can't be resolved (`realpath` fails) gives `NSOSStatusErrorDomain` −36 (`ioErr`).
    2. Otherwise take the containing volume (NSURLVolumeURLKey) and its disk (`DADiskCreateFromVolumePath`), then its whole disk.
    3. Unmount the whole disk with `kDADiskUnmountOptionWhole`, then eject it.
    4. Any missing disk or dissenter gives −47 (`fBsyErr`) with empty userInfo.
    5. It waits by running the current run loop in the private mode `NSWorkspaceEjectRunLoopMode`, where its session is scheduled. Task 5 adds its notification session to the same mode, so use a file-static `NSString* const`.
  - **`unmountAndEjectDeviceAtPath:`** returns `[self unmountAndEjectDeviceAtURL:[NSURL fileURLWithPath:path] error:NULL]`.
- `tests/nsworkspace-volumes.m` prints the calls' results in the Observed format, then takes these modes:
  - `eject <path>`: prints `eject <path>: <0|1> <domain> <code>`;
  - `eject-path <path>`: prints the BOOL.

- [ ] **Step 1: Write the failing test.** Add the case "workspace volumes" to `tests/volumes.sh`:
  - with the image attached: `mountedLocalVolumePaths` contains `/` and `/Volumes/Unmount Test`, and `mountedRemovableMedia` is exactly `/Volumes/Unmount Test`;
  - `getFileSystemInfoForPath:` for the image gives `1 1 0 1 hfs hfs`, and `1 0 x 0 …` for `/`. "writable" for `/` follows the guest, so it isn't checked.
  - eject results:

| Target | Expected |
|--------|----------|
| `/nonexistent` | `0 NSOSStatusErrorDomain -36` |
| `/` | `0 NSOSStatusErrorDomain -47`, and `/` still works |
| `/Volumes/Unmount Test/hello.txt` | `1`, and `/Volumes/Unmount Test` is gone within 5 s |
| `eject-path` of the re-attached image | 1 |

  Review Focus 5: re-attach, start `tests/da-ops deny 15` (built as in `diskarbitration.sh`), then eject. Expected: `0 NSOSStatusErrorDomain -47` and still mounted. Then stop `deny` and eject again. Expected: 1.
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: `FAIL: workspace volumes`, with empty lists and no symbol `unmountAndEjectDeviceAtURL:error:`.
- [ ] **Step 3: Implement** per the Interfaces.
- [ ] **Step 4: Run it to see it pass.** Rebuild `AppKit`, then run `make install` of cocotron's AppKit (`cmake -P src/external/cocotron/cmake_install.cmake`, or the AppKit subdirectory's script) and `run-volumes all`. Expected: PASS in all three modes.
- [ ] **Step 5: Commit.**
  - cocotron `finder-volumes`: "NSWorkspace: volume paths, file system info and eject through DiskArbitration".
  - darling bump.
  - Provenance: documentation, the macOS observations of error codes.

### Task 5: Mount notifications

**Files:**
- Create: cocotron `AppKit/NSWorkspaceVolumeEvents.h`, `AppKit/NSWorkspaceVolumeEvents.m`; `tests/nsworkspace-mount-notifications.m`
- Modify: cocotron `AppKit/NSWorkspace.m` (`notificationCenter` starts the events once), `AppKit/MacWorkspace.m` (the eject also schedules the events session in its mode), `AppKit/CMakeLists.txt`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes:
  - Task 4's `NSWorkspaceEjectRunLoopMode`.
  - DiskArbitration: `DARegisterDiskAppearedCallback`, `DARegisterDiskDisappearedCallback`, `DARegisterDiskDescriptionChangedCallback` (watching `kDADiskDescriptionVolumePathKey`), `DARegisterDiskUnmountApprovalCallback`, and `DAAppearanceTime` in descriptions.
- Produces:
  - **`void NSWorkspaceVolumeEventsStart(NSWorkspace* workspace)`:**
    - It is idempotent and thread-safe (`dispatch_once`).
    - It creates one session, scheduled on `CFRunLoopGetMain()` in `kCFRunLoopCommonModes`, and records the start time.
    - It posts to `workspace.notificationCenter` on the main thread, with object `workspace` and the userInfo keys from the Observed section:

| Event | Posts |
|-------|-------|
| a disk appears with a volume path, and its `DAAppearanceTime` is later than the start (older disks are catch-up, not mounts); or its description changes from no path to a path | `NSWorkspaceDidMountNotification` |
| the unmount approval callback runs for a disk with a volume path (it always approves: returns NULL) | `NSWorkspaceWillUnmountNotification` |
| a description change from a path to none; or a disk that disappears while it had a path | `NSWorkspaceDidUnmountNotification` |

    - It keeps a mount-point set, so each mount gives exactly one DidMount and one DidUnmount.
  - **`void NSWorkspaceVolumeEventsSchedule(CFStringRef mode, BOOL schedule)`:** adds the session to the main run loop in `mode`, or removes it. Task 4's eject calls it around its wait when it runs on the main thread, so the app's own approval callback answers at once.
  - `-[NSWorkspace notificationCenter]` calls `NSWorkspaceVolumeEventsStart(self)` before returning the centre.
- `tests/nsworkspace-mount-notifications.m <image> <mount point>` prints `<name> <NSDevicePath> <localized name> main=<0|1>` for each mount notification as it comes, while it works through these steps:
  1. `hdiutil attach` (NSTask), then run the loop for 3 s;
  2. `hdiutil detach`, 3 s;
  3. attach, then `unmountAndEjectDeviceAtURL:` on the main thread, printing the time it took and `posted-before-return=<0|1>`;
  4. attach, start `da-ops deny 15`, eject (expect NO), stop deny, eject (expect YES).

- [ ] **Step 1: Write the failing test.** Add the case "mount notifications" to `tests/volumes.sh`. The expected lines, in order (DidMount, WillUnmount and DidUnmount, with the device path, the name and `main=1` each time):
  1. attach: DidMount;
  2. detach: WillUnmount, DidUnmount;
  3. attach: DidMount. Our eject: WillUnmount, DidUnmount, `posted-before-return=1`, elapsed < 5 s (Review Focus 3);
  4. attach: DidMount. Dissented eject: WillUnmount and no DidUnmount (Review Focus 5). Second eject: WillUnmount, DidUnmount.

  A notification for a volume that was already mounted when the program started (`/`) must not appear.

  Review Focus 1: run steps 1–2 with `slash-name.dmg`. Expected: device path `/Volumes/A:B`, localized name `A/B`.
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: `FAIL: mount notifications`, with no lines.
- [ ] **Step 3: Implement** per the Interfaces.
- [ ] **Step 4: Run it to see it pass.** Rebuild and install AppKit, then `run-volumes all`. Expected: PASS in all three modes, and Task 4's case still passes.
- [ ] **Step 5: Commit.**
  - cocotron: "NSWorkspace: mount notifications from DiskArbitration".
  - darling bump.
  - Provenance: documentation, the macOS observations of order, thread and userInfo.

### Task 6: Icons for files, folders, volumes and disk images

**Files:**
- Modify: cocotron `AppKit/MacWorkspace.m` (`iconForFile:` :99, `iconForFiles:` :148, add `iconForContentType:`), `AppKit/include/AppKit/NSWorkspace.h`, `AppKit/CMakeLists.txt` (resources :507-518), `AppKit/SystemImages/COPYING.Adwaita`, `tests/nsworkspace-iconforfile.m` (the `strict` expectations)
- Create: `AppKit/SystemImages/NSVolumeInternal.png` (Adwaita `devices/drive-harddisk`), `NSVolumeRemovable.png` (`devices/drive-removable-media`), `NSDiskImage.png` (`mimetypes/package-x-generic`), `NSMultipleDocuments.png` (two `mimetypes/application-x-generic`, the second offset 6 px right and down, on a 32×32 canvas); `tests/nsworkspace-icon-kinds.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: Task 2's `NSURLIsVolumeKey` and `NSURLVolumeIsRemovableKey`, plus the existing `iconForFileType:`.
- Produces:
  - **`iconForFile:`**, in this order:

| Item | Icon |
|------|------|
| a bundle with its own icon | that icon (today's behaviour) |
| a volume's mount point | `NSVolumeRemovable` when removable, else `NSVolumeInternal` |
| an application bundle (`.app` or `CFBundlePackageType` `APPL`) without an icon | `iconForFileType:@"com.apple.application-bundle"` |
| another directory | the folder icon |
| a missing path or `""` | `iconForFileType:@"public.data"` |
| a file | `iconForFileType:` of its extension (no extension → `public.data`) |

    A type conforming to `public.disk-image` gives `NSDiskImage`, and `public.volume` gives `NSVolumeInternal`, so `iconForFileType:` returns them too.
  - **`iconForFiles:`:** `@[]` → nil; one path → `iconForFile:`; more → `[NSImage imageNamed:NSImageNameMultipleDocuments]`. That name maps to `NSMultipleDocuments.png`; add the constant if it is missing.
  - **`- (NSImage*)iconForContentType:(UTType*)type`** returns `[self iconForFileType:type.identifier]`.
- Images: render with `rsvg-convert -w 32 -h 32`; compose `NSMultipleDocuments` with `magick`. Add one row per image to `COPYING.Adwaita`'s table.

- [ ] **Step 1: Write the failing tests.**
  - **`nsworkspace-icon-kinds.m <image mount point>`** compares icons by PNG pixels, as the Observed probe does:
    - missing path, `""`, `b.zzqq`, `noext` → `public.data`;
    - folder → `public.folder`;
    - `a.txt` → `txt`;
    - `c.dmg` → `public.disk-image` and different from `public.data`;
    - `Bare.app` (Info.plist without an icon) → `com.apple.application-bundle`, different from `public.data`;
    - `/` → `public.volume`;
    - the image mount point → different from `/` and from the folder;
    - `iconForFiles:@[]` → nil;
    - one file → the same pixels as `iconForFile:`;
    - two files → `NSImageNameMultipleDocuments`;
    - `iconForContentType:` of the `public.disk-image` UTType → the same as `c.dmg`;
    - every icon is 32×32.
  - In `nsworkspace-iconforfile.m`, change the `strict` cases to the macOS behaviour: a bundle without an icon gives the generic application icon, and a plain file gives its type icon.
  - Add the case "icon kinds" to `tests/volumes.sh`; it also runs `nsworkspace-iconforfile strict` and `appkit-workspace-type-icons`.
- [ ] **Step 2: Run them to see them fail.** Run `run-volumes rootless`. Expected: `FAIL: icon kinds` (nil for the plain file) and `FAIL: iconforfile strict`.
- [ ] **Step 3: Implement** per the Interfaces, and add the images.
- [ ] **Step 4: Run them to see them pass.** Rebuild and install AppKit, then `run-volumes all`. Expected: PASS in all three modes.
- [ ] **Step 5: Commit.**
  - cocotron: "NSWorkspace: icons for files, folders, volumes and disk images".
  - darling bump.
  - Provenance: documentation, the macOS observations, and the Adwaita images (LGPL-3/CC-BY-SA-3.0, attributed).

  Ruling in the commit message: `iconForFile:nsworkspace-iconforfile strict` expected nil before, which was Darling-only behaviour. It now follows macOS, so this is a correction of a test, not a weakening.

### Task 7: recycleURLs:completionHandler:

**Files:**
- Modify: cocotron `AppKit/MacWorkspace.m`, `AppKit/include/AppKit/NSWorkspace.h`
- Create: cocotron `tests/nsworkspace-recycle.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: Task 3's `trashItemAtURL:resultingItemURL:error:`.
- Produces: `- (void)recycleURLs:(NSArray<NSURL*>*)URLs completionHandler:(void (^)(NSDictionary<NSURL*, NSURL*>* newURLs, NSError* error))handler`.
  - It trashes each URL on a background queue.
  - Then it calls `handler` once, on the main queue, with:
    - `newURLs`: the items that moved, original → new;
    - `error`: nil when all moved. Otherwise an `NSCocoaErrorDomain` error with the first failure's code, `NSLocalizedDescriptionKey` "The file “<name>” could not be moved to the trash.", and `NSUnderlyingErrorsKey` holding every failure.
  - A nil handler is allowed.

- [ ] **Step 1: Write the failing test** `nsworkspace-recycle.m <dir>`:
  - recycle a file plus a missing one. Expected: `newURLs` has 1 entry, error code 4, 1 underlying error, handler on the main thread;
  - recycle a file alone. Expected: error nil, the new URL exists, the old one doesn't;
  - recycle with a nil handler. Expected: no crash, the file moves within 5 s.

  Add the case "recycle" to `tests/volumes.sh`.
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: FAIL (unrecognized selector).
- [ ] **Step 3: Implement** per the Interfaces.
- [ ] **Step 4: Run it to see it pass.** Run `run-volumes all`. Expected: PASS in all three modes.
- [ ] **Step 5: Commit.** cocotron "NSWorkspace: recycleURLs:completionHandler:" and the darling bump, with Provenance.

### Task 8: LSOpenApplication with an initial event

**Files:**
- Modify: darling `src/frameworks/CoreServices/src/LaunchServices/LaunchServices.c` (next to `LSOpenFromURLSpec`), `src/LaunchServices/LSOpen.m` (remove the commented-out stub at :167)
- Create: darling `tests/src/ae-receiver.c` (the test bundle's executable), `tests/src/launchservices_open_application.m`
- Test: `tests/volumes.sh`

**Interfaces:**
- Consumes: the existing launch path inside `LSOpenFromURLSpec`, launchservicesd's queued-first-event delivery (`send_event(event, pid, …, queue)`), and `LSRegisterURL`.
- Produces: `OSStatus LSOpenApplication(const LSApplicationParameters* appParams, ProcessSerialNumber* outPSN)`.
  - It launches `appParams->application` (an FSRef; `FSRefMakePath`), honouring `kLSLaunchNewInstance`, `kLSLaunchDontSwitch` and `kLSLaunchAndHide` as `LSOpenFromURLSpec` does.
  - When `initialEvent` is set:
    - a new process gets it as its first event, instead of `kAEOpenApplication`;
    - a running instance (when `kLSLaunchNewInstance` is not set) gets it at once, instead of `kAEReopenApplication`.
  - `argv` and `environment` are passed to a new process.
  - `outPSN` (if given) gets `{ 0, pid }`.
  - Errors: a NULL `appParams` or `application`, or `version` ≠ 0 → `paramErr`; a path that isn't an application → `kLSApplicationNotFoundErr` (−10814).
- `tests/src/ae-receiver.c`: a bundle executable.
  - It installs handlers for every event (`typeWildCard`/`typeWildCard`).
  - For each event it appends `<class> <id>[ aapd]<items>` to `<bundle>/../events.txt`: ` aapd` when the event has an `'aapd'` parameter that is true; then, for each direct-object item, ` <url>` for a `typeFileURL` and ` bmrk:<url>` for `typeBookmarkData` (resolved with `CFURLCreateByResolvingBookmarkData`).
  - It exits 20 s after the last event.
  - The test builds two bundles around it: `Receiver.app` (`org.darlinghq.test.ae-receiver`), and later `Finder.app` (`com.apple.finder`, for Task 9).

- [ ] **Step 1: Write the failing test** `launchservices_open_application.m <dir with Receiver.app>`:
  - register the bundle;
  - `LSOpenApplication` with an `initialEvent` `'aevt'`/`'odoc'` whose direct object is a list of two `typeFileURL`s. Expected: noErr, a pid > 0, and `events.txt` gets `aevt odoc <url1> <url2>` as its first line within 10 s;
  - call it again while the receiver runs. Expected: the same pid, and a second `aevt odoc` line;
  - a NULL `application` → `paramErr`;
  - a plain directory → −10814.

  Add the case "LSOpenApplication" to `tests/volumes.sh`. It removes the bundles afterwards; lookups skip removed bundles (`tests/src/launchservices_removed_bundles.m`).
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: FAIL (undefined symbol `_LSOpenApplication`).
- [ ] **Step 3: Implement** per the Interfaces.
- [ ] **Step 4: Run it to see it pass.** Rebuild `LaunchServices` and `launchservicesd` (as needed) and install `src/frameworks/CoreServices`, then `run-volumes all`. Expected: PASS in all three modes.
- [ ] **Step 5: Commit** in darling: "LaunchServices: LSOpenApplication with an initial event". Provenance: the public LSOpenDeprecated.h contract.

### Task 9: Reveal in the file viewer

**Files:**
- Modify: cocotron `AppKit/MacWorkspace.m` (`selectFile:inFileViewerRootedAtPath:` ~:400, `activateFileViewerSelectingURLs:` ~:562); darling `src/frameworks/CoreServices/include/AE/AEDataModel.h` (add `typeBookmarkData = 'bmrk'`, public on macOS)
- Create: cocotron `tests/nsworkspace-reveal.m`
- Test: `tests/volumes.sh` (reuses Task 8's `ae-receiver.c`)

**Interfaces:**
- Consumes:
  - Task 8's `LSOpenApplication`;
  - `URLForApplicationWithBundleIdentifier:` (MacWorkspace.m:582);
  - Foundation's `NSAppleEventDescriptor`, for building the event; `-[NSURL bookmarkDataWithOptions:...]` for the items.
- Produces:
  - The file viewer's bundle ID: the `NSFileViewer` user default when it is a non-empty string, else `com.apple.finder`.
  - **`activateFileViewerSelectingURLs:(NSArray<NSURL*>*)urls`:**
    - builds one `'aevt'`/`'odoc'` event addressed to the file viewer, whose direct object is a list of the URLs' bookmark data (`typeBookmarkData`), in order, and which has the parameter `'aapd'` = `typeBoolean` true;
    - `'aapd'` is undocumented: a `QUIRK:` comment cites the macOS 26 observation (tier 2) and `nsworkspace-reveal.m`;
    - calls `LSOpenApplication` with the event as `initialEvent` (no `kLSLaunchDontSwitch`);
    - does nothing for an empty array, a URL without bookmark data, or when the file viewer is not found.
  - **`selectFile:(NSString*)path inFileViewerRootedAtPath:(NSString*)root`:**
    - `root` is not sent;
    - a non-empty `path` → the same event for `[path]`;
    - a nil or empty `path` → `'aevt'`/`'rapp'` with no items;
    - returns YES once the event is handed to LaunchServices, NO when the file viewer is not found or the launch fails.

- [ ] **Step 1: Write the failing test** `nsworkspace-reveal.m <dir>`:
  - Before the test registers a file viewer: `selectFile:@"/tmp" inFileViewerRootedAtPath:@""` returns NO, because the guest has no Finder yet (plan 7).
  - Then the driver installs the `com.apple.finder` receiver bundle and registers it.
  - `selectFile:<dir>/a.txt inFileViewerRootedAtPath:@"/"` returns YES, and `aevt odoc aapd bmrk:file://<dir>/a.txt` is the receiver's first event.
  - `activateFileViewerSelectingURLs:@[a, sub/b]` gives `aevt odoc aapd bmrk:<a> bmrk:<sub/b>`.
  - `selectFile:nil inFileViewerRootedAtPath:<dir>` gives `aevt rapp`.
  - With `NSFileViewer` set (in the test's argument domain) to `org.darlinghq.test.ae-receiver` and `Receiver.app` registered, the reveal goes to that bundle's `events.txt`.

  Add the case "reveal" to `tests/volumes.sh`. It removes the receiver bundles afterwards.
- [ ] **Step 2: Run it to see it fail.** Run `run-volumes rootless`. Expected: FAIL (NO where YES is expected).
- [ ] **Step 3: Implement** per the Interfaces.
- [ ] **Step 4: Run it to see it pass.** Rebuild and install AppKit, then `run-volumes all`. Expected: every case of `tests/volumes.sh` passes in all three modes. Also run `TESTS="tests/diskarbitration.sh tests/dmg-attach.sh tests/volume-attributes.sh"` in all modes. Expected: no new FAIL.
- [ ] **Step 5: Commit.**
  - cocotron: "NSWorkspace: reveal in the file viewer with the event macOS sends"; darling: the `typeBookmarkData` constant.
  - darling bump.
  - Mark plan 4 done in `docs/superpowers/plans/2026-11-16-finder-roadmap.md`, with the Repos cell: "xnu, corefoundation, foundation, cocotron, darling (CoreServices)".
  - Provenance: the NSWorkspace contract, the public Apple Event constants, and the macOS 26 black-box observation of the reveal events (user-supplied).
