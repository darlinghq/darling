# Real unmount(2) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `unmount(2)` in Darling actually unmount (including FUSE-backed disk images in every launcher mode), and make `hdiutil detach` use it instead of spawning `fusermount` itself.

**Architecture:** `sys_unmount` in xnu's `libsystem_kernel` emulation translates the guest path (vchroot), translates BSD flags, and calls Linux `umount2`. When Linux refuses with `EPERM`/`EACCES` and the target is a FUSE mount owned by the calling user, it runs the host's `fusermount -u` through the ELF bridge — so the only host-specific unmount code lives at the lowest layer. `hdiutil detach` (darling-dmg) then just calls `unmount()`.

**Tech Stack:** C (libsystem_kernel emulation, no libc above it: use `LINUX_SYSCALL`, `vchroot_expand`, `elfcalls()`), C++ (darling-dmg `hdiutil`), shell for the integration test.

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md` (section "Unmount and DiskArbitration"); roadmap `docs/superpowers/plans/2026-11-16-finder-roadmap.md`.

## Global Constraints

- AGENTS.md applies in full; no app-specific logic; no debugging leftovers; Provenance note in each PR.
- Must work in all three launcher modes: root, rootless (`DARLING_ROOTLESS=1`), non-root (`DARLING_NONROOT=1`).
- Errnos follow the macOS `unmount(2)` man page; values the man page does not pin are confirmed by black-box observation on `ssh lubos@10.10.10.152` (macOS Tahoe) before being asserted.
- `libsystem_kernel` sits below malloc and libc: no libc calls other than the `extern` helpers already used in that tree (`strlcpy`, `strcmp`, …); buffers on the stack or `sys_mmap`.
- Not in `VARIANT_DYLD` builds: the fusermount fallback is compiled out there (return the Linux error).
- Build tree `~/Projects/darling/build`; runtime testing per darling-skills `darling-guest-testing` (clone `https://github.com/darlinghq/darling-skills` to `$DARLING_SKILLS` and read `darling-guest-testing/SKILL.md` first).
- xnu changes on branch `unmount-syscall` in `src/external/xnu`; darling-dmg changes on branch `detach-unmount` in `src/external/darling-dmg`; main-repo changes on `finder-app`. Never `git add -A` in the main repo (it has unrelated submodule modifications).

## Review Focus

1. **Busy volume** (a file open on it): `unmount(path, 0)` must fail with `EBUSY` and leave the volume mounted; `MNT_FORCE` must unmount. → pinned in Task 3's integration test.
2. **Path that is inside, but not the root of, a mount** (e.g. `/Volumes/X/sub`): must fail `EINVAL`, never unmount the containing volume. → Task 1 test + Task 3 test.
3. **Another user's FUSE mount** / FUSE mount without `user_id=` option: must return `EPERM` without spawning `fusermount`. → Task 2 unit-level check via the options parser test.
4. **Mount point path with spaces or non-ASCII** (`/Volumes/My App 1`): fallback must pass the host path as one argv element. → Task 3 fixture volume name contains a space.
5. **Trailing slash / symlink to a mount point** (`/Volumes/X/`, a symlink pointing at `/Volumes/X`): macOS resolves the path; behaviour confirmed on macOS in Task 1 and asserted. → Task 1 test.

---

### Task 1: Error-path contract test and Linux `umount2` path

**Files:**
- Create: `src/external/xnu/darling/tests/unmount-errors.c`
- Modify: `src/external/xnu/darling/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/impl/mount/unmount.c`

**Interfaces:**
- Produces: `long sys_unmount(const char* path, int flags)` — returns 0 or a negative BSD errno (existing signature, header unchanged).

- [ ] **Step 1: Write the failing test** `unmount-errors.c` — a guest C program printing one `PASS`/`FAIL:` line per check, exit 1 on any failure. Checks (each `unmount()` call returns -1 and sets `errno`):
  - `unmount(<fresh mkdtemp dir under /tmp>, 0)` → `EINVAL` (not a mount point)
  - `unmount("/nonexistent-unmount-test", 0)` → `ENOENT`
  - `unmount(NULL, 0)` → `EFAULT`
  - `unmount(<mkdtemp dir>/file, 0)` where `file` is a regular file → `EINVAL` (or `ENOTDIR`; see Step 2)
  - `unmount("/", 0)` as a non-root user → `EPERM`
  - `unmount(<symlink in /tmp pointing at the mkdtemp dir>, 0)` → `EINVAL`

- [ ] **Step 2: Pin the values on macOS**

Run: `scp unmount-errors.c lubos@10.10.10.152:/tmp/ && ssh lubos@10.10.10.152 'cc -o /tmp/unmount-errors /tmp/unmount-errors.c && /tmp/unmount-errors'`
Expected: every line `PASS`. If a line fails, change that expectation to what macOS returned, add a one-line comment `// observed on macOS 26 (Tahoe), arm64` above it, and re-run until all PASS. Record the OS version (`sw_vers`) in the test's header comment.

- [ ] **Step 3: Run in Darling to verify it fails**

Run (from the build tree, after `$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh -o /tmp/unmount-errors unmount-errors.c`):
`timeout -s KILL 60 darling shell /Volumes/SystemRoot/tmp/unmount-errors </dev/null > /tmp/ue.out 2>&1; cat /tmp/ue.out`
Expected: `FAIL:` lines (the stub returns `EBUSY` for everything).

- [ ] **Step 4: Implement the `umount2` path in `sys_unmount`**

`path == NULL` → `-EFAULT`. Expand with `vchroot_expand` (flags `VCHROOT_FOLLOW`, `dfd = LINUX_AT_FDCWD`, as `fstatat.c` does), returning `errno_linux_to_bsd` on failure. BSD `MNT_FORCE` (`0x00080000`) → Linux `MNT_FORCE` (`1`); all other BSD bits ignored. Call `LINUX_SYSCALL(__NR_umount2, vc.path, linux_flags)`; on error return `errno_linux_to_bsd(ret)`, except `ENOTDIR`/`EINVAL` mapping adjusted to what Step 2 pinned. Leave a clearly marked call site for Task 2's fallback when the Linux error is `EPERM` or `EACCES`.

- [ ] **Step 5: Build, install, run test to verify it passes**

Run: `cd ~/Projects/darling/build && make -j$(nproc) && sudo make install` (libsystem_kernel is widely linked: full install per the skill), then `$DARLING_SKILLS/darling-guest-testing/scripts/restart.sh` and the Step 3 command.
Expected: all `PASS`.

- [ ] **Step 6: Commit** (in `src/external/xnu`, branch `unmount-syscall`)

```bash
git add darling/tests/unmount-errors.c darling/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/impl/mount/unmount.c
git commit -m "unmount: unmount with Linux umount2 and report macOS errors"
```

### Task 2: FUSE fallback through the host's fusermount

**Files:**
- Modify: `src/external/xnu/darling/src/libsystem_kernel/emulation/include/xnu_syscall/bsd/impl/stat/mount_table.h`
- Modify: `src/external/xnu/darling/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/impl/stat/mount_table.c`
- Modify: `src/external/xnu/darling/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/impl/mount/unmount.c`
- Create: `src/external/xnu/darling/tests/mount-table-options.c` (host-side unit test, see Step 1)

**Interfaces:**
- Consumes: `sys_unmount` from Task 1; existing `mount_table_enumerate`, `elfcalls()->dlsym`, `elfcalls()->get_errno`.
- Produces:
  - `long mount_table_mount_at(const char* host_mount_point, char* fstype, size_t fstype_size, char* options, size_t options_size)` — exact match on the mount point column of `/proc/self/mounts` (the topmost mount there); returns 0 and fills both buffers, `-EINVAL` if nothing is mounted exactly there, other negative BSD errno if the table can't be read. The options column is needed, so the table walker must expose it (extend the private parser; keep `mount_table_visitor`'s signature unchanged for existing callers).
  - `int mount_options_fuse_user_id(const char* options, unsigned int* uid)` — parses `user_id=<n>` from a comma-separated option string; returns 1 and sets `*uid` if present, 0 otherwise. Pure function, in `mount_table.c`, declared in the header so the unit test can call it.

- [ ] **Step 1: Write the failing unit test** `mount-table-options.c`: compiled on the host against `mount_table.c`'s parser function only (copy-free: `#include` the `.c` with the libsystem externs stubbed, or move the pure parser into its own small `.c` — implementer's call). Assertions:
  - `"rw,nosuid,nodev,relatime,user_id=1000,group_id=1000"` → returns 1, uid 1000
  - `"rw,user_id=0"` → 1, uid 0
  - `"rw,nosuid"` → 0
  - `"rw,xuser_id=5"` → 0 (must match a whole option)
  - `""` → 0
  - `"user_id="` → 0; `"user_id=12a"` → 0

- [ ] **Step 2: Run it to verify it fails**

Run: `cc -std=gnu11 -Wall -Werror -o /tmp/mto <test sources> && /tmp/mto`
Expected: build failure (`mount_options_fuse_user_id` undefined).

- [ ] **Step 3: Implement `mount_options_fuse_user_id` and `mount_table_mount_at`**

- [ ] **Step 4: Run the unit test to verify it passes**

Run: Step 2 command. Expected: all `PASS`.

- [ ] **Step 5: Implement the fallback in `sys_unmount`** (`#ifndef VARIANT_DYLD`)

When `umount2` fails with `EPERM`/`EACCES`:
1. `mount_table_mount_at(vc.path, …)`; not found → `-EINVAL`.
2. `fstype` does not start with `"fuse"` → return the original error (`EPERM`).
3. No `user_id=` or it differs from the caller's real uid (`LINUX_SYSCALL(__NR_getuid)`) → `-EPERM` (fusermount would refuse anyway; don't spawn it).
4. Spawn the host's `fusermount` with argv `{"fusermount", "-u", ["-z" if MNT_FORCE,] vc.path, NULL}` using `posix_spawnp` obtained via `elfcalls()->dlsym(NULL, "posix_spawnp")`, with `PATH` extended by `:/Volumes/SystemRoot/bin:/Volumes/SystemRoot/usr/bin` the same way `darling-dmg/src/main-hdiutil.cpp:addFusermountIntoPath` does (setenv on the ELF side, via `elfcalls()->dlsym(NULL, "setenv")`). If `fusermount` is not found, try `fusermount3`. Wait with `LINUX_SYSCALL(__NR_wait4, pid, &status, 0, NULL)`, retrying on `EINTR`.
5. Exit status 0 → return 0. Non-zero → re-check with `mount_table_mount_at`: still mounted → `-EBUSY`; gone → 0. Spawn failure → `-EPERM`.

`-z` for `MNT_FORCE` is a mapping choice (lazy detach is the closest the unprivileged path offers to a forced unmount), not a quirk; say so in a comment.

- [ ] **Step 6: Commit** (xnu, branch `unmount-syscall`)

```bash
git add darling/src/libsystem_kernel/emulation darling/tests/mount-table-options.c
git commit -m "unmount: unmount the caller's FUSE mounts through the host's fusermount"
```

### Task 3: Disk-image integration test and `hdiutil detach` on `unmount()`

**Files:**
- Create: `tests/fixtures/unmount-test.dmg` (main repo; made on macOS, see Step 1)
- Create: `tests/unmount-disk-image.c` (guest program)
- Create: `tests/unmount-disk-image.sh` (host driver)
- Create: `tests/unmount-disk-image.md` (how to run; provenance of the fixture)
- Modify: `src/external/darling-dmg/src/main-hdiutil.cpp` (`doDetach`, option table)

**Interfaces:**
- Consumes: `unmount()` from Tasks 1–2 (through libSystem); `hdiutil attach` as it exists.
- Produces: `hdiutil detach [-force] [-quiet] <mount-path>` built on `unmount()`; exit 0 on success, 1 on failure with a message on stderr matching macOS's wording (pinned in Step 2).

- [ ] **Step 1: Make the fixture on macOS**

```bash
ssh lubos@10.10.10.152 'cd /tmp && rm -f ut.dmg ut-ro.dmg &&
  hdiutil create -size 4m -fs HFS+ -volname "Unmount Test" ut.dmg &&
  hdiutil attach ut.dmg -mountpoint /tmp/utmnt && mkdir /tmp/utmnt/sub &&
  echo hello > /tmp/utmnt/hello.txt && hdiutil detach /tmp/utmnt &&
  hdiutil convert ut.dmg -format UDZO -o ut-ro.dmg'
scp lubos@10.10.10.152:/tmp/ut-ro.dmg tests/fixtures/unmount-test.dmg
```
Expected: a file of a few KB. The volume name contains a space (Review Focus 4).

- [ ] **Step 2: Pin `hdiutil detach` failure output on macOS**

Run on macOS: attach the fixture, `cd` into the volume in a second shell (or hold `exec 3</Volumes/Unmount\ Test/hello.txt`), run `hdiutil detach "/Volumes/Unmount Test"; echo rc=$?`, then `hdiutil detach -force …; echo rc=$?`.
Record exact stderr text and exit codes in `tests/unmount-disk-image.md`.

- [ ] **Step 3: Write the guest program** `unmount-disk-image.c` taking the mount point as `argv[1]`, printing `PASS`/`FAIL:` per check:
  - `unmount("<mp>/sub", 0)` → -1 `EINVAL`, and `<mp>/hello.txt` still readable
  - open `<mp>/hello.txt`, keep the fd: `unmount(mp, 0)` → -1 `EBUSY`, file still readable
  - `unmount(mp, MNT_FORCE)` → 0
  - `statfs(mp)` → `f_fstypename` is no longer the disk image's (compare with the value read before)
  - second `unmount(mp, 0)` → -1 `EINVAL`

- [ ] **Step 4: Write the driver** `unmount-disk-image.sh`: builds the guest program with `$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh`, copies the fixture into the guest (`darling shell cp /Volumes/SystemRoot/<abs>/tests/fixtures/unmount-test.dmg /Users/$USER/`), runs `hdiutil attach` inside the guest, runs the guest program on `"/Volumes/Unmount Test"`, then attaches again and checks `hdiutil detach` (busy → rc 1 with the Step 2 message; `-force` → rc 0; mount point directory removed). Uses `timeout -s KILL` and `</dev/null` on every `darling shell`, output to files. Exit non-zero on any `FAIL:`.

- [ ] **Step 5: Run it to verify the detach part fails**

Run: `tests/unmount-disk-image.sh`
Expected: guest-program checks PASS (Tasks 1–2 done); `hdiutil detach` busy-case checks FAIL (current `doDetach` ignores `fusermount`'s status and always returns 0, and has no `-force`).

- [ ] **Step 6: Rewrite `doDetach` on `unmount()`**

Add `-force` to the option table (→ `MNT_FORCE`). Call `unmount(argv[optind], flags)`; on failure print the Step 2 message format with `strerror(errno)` and return 1; on success `rmdir` the mount point and return 0. Remove the `posix_spawnp`/`fusermount` code from `doDetach`; `addFusermountIntoPath()` stays (libfuse's mount path still needs it in `doAttach`).

- [ ] **Step 7: Rebuild hdiutil and run the integration test**

Run: `$DARLING_SKILLS/darling-guest-testing/scripts/rebuild.sh hdiutil && tests/unmount-disk-image.sh`
Expected: exit 0, all `PASS`.

- [ ] **Step 8: Mode matrix**

Run the driver in each mode with its own prefix, shutting down between:
- root: `darling shutdown; tests/unmount-disk-image.sh`
- rootless: `DARLING_ROOTLESS=1 DPREFIX=~/.darling-rootless …` with a non-setuid launcher
- non-root: `DARLING_NONROOT=1 DPREFIX=~/.darling-nonroot …` with a non-setuid launcher
The unprivileged modes need a non-setuid `darling` (see `tests/launcher/README.md`); **ask the user** how they run those on this machine before this step. Expected: all PASS in all three. Record per-mode results in `tests/unmount-disk-image.md`.

- [ ] **Step 9: Commit**

```bash
# darling-dmg, branch detach-unmount
git add src/main-hdiutil.cpp
git commit -m "hdiutil: detach through unmount() and support -force"
# main repo, branch finder-app
git add tests/fixtures/unmount-test.dmg tests/unmount-disk-image.c tests/unmount-disk-image.sh tests/unmount-disk-image.md
# Both submodules were already ahead of the recorded commit before this work
# (unrelated changes). Before adding them, check `git -C src/external/xnu log
# <recorded>..HEAD` and ask the user whether the bump may include those commits.
git add src/external/xnu src/external/darling-dmg   # submodule bumps only
git commit -m "tests: unmount and hdiutil detach of a disk image"
```
