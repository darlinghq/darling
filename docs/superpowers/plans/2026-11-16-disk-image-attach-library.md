# Disk-Image Attach Library Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move disk-image attach/detach out of the `hdiutil` executable into a private C-callable library (plus a small FUSE helper process) that both `hdiutil` and the future DiskImageMounter.app use; mount volumes under their HFS+ volume name; read embedded licence agreements and make `hdiutil attach` honour them as macOS does.

**Architecture:** `libdarling-dmg.dylib` (darling-dmg repo, C API over the existing C++ reader classes) opens an image, reports its volume name and licence agreements, and attaches it by spawning `darling-dmg-mount`, a helper executable that runs the FUSE loop and reports readiness over an inherited pipe. The helper is its own session leader so the volume outlives the caller. `hdiutil attach`/`detach` become thin front ends over the library.

**Tech Stack:** C++11 (darling-dmg), C API header, libfuse 2 through `wrap_elf`, libxml2 (existing UDIF plist parsing), Darling `add_darling_executable`/`add_darling_library`.

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md` (section "Disk images: attach library, hdiutil, DiskImageMounter"); roadmap `docs/superpowers/plans/2026-11-16-finder-roadmap.md` (plan 2).

## Global Constraints

- AGENTS.md applies in full. The library is **Darling's own private interface**: no `DI*`/`DIHL*` names or anything modelled on Apple's private DiskImages framework; prefix every symbol `darling_dmg_`.
- Licence-agreement resource formats (`LPic`, `STR#`, `TEXT`, `RTF `) are implemented from public descriptions (e.g. Apple's public "SLAs for UDIFs" SDK documentation, dmgbuild's documentation) — paraphrased, no copied code; cite the sources in the PR's Provenance note.
- Every user-visible `hdiutil` behaviour asserted by a test is first observed on `ssh lubos@10.10.10.152` (macOS 26.7.3) and recorded in the test's `.md` (tier-2 evidence). Fixtures are made by us (on the Mac with `hdiutil create`/`udifrez` from resource files we write).
- Must work in all three launcher modes (root, rootless, non-root); use the plan-1 setup: install to `~/darling-finder`, prefixes `~/.darling-finder` (`DARLING_ROOTLESS=1`), `~/.darling-finder-nr` (`DARLING_NONROOT=1`), `~/.darling-finder-root` (root mode needs a temporary setuid launcher: ask the user first).
- Branches: darling-dmg `attach-library` (based on `detach-unmount`); main repo `finder-app`. Push branches, **open no PRs** (user rule for this project).
- Main-repo tests live flat in `tests/` and are force-added (`git add -f`; `/tests/*` is gitignored).
- The installed Darling under `/usr/local` belongs to another session: never install there.

## Review Focus

1. **Caller exits right after attach** (DiskImageMounter quits; a shell runs `hdiutil attach` and ends): the volume must stay mounted. → Task 1 test `attach survives caller`.
2. **Two images with the same volume name attached at once**: second gets a distinct mount point (macOS naming), neither shadows the other. → Task 2 test.
3. **Corrupt / non-image / truncated file**: attach fails with a clear error and no stale empty `/Volumes/<name>` directory and no orphan helper process. → Task 1 test `attach of a non-image`.
4. **Volume name with `/` or `:`** (legal in HFS+ names) or empty name: mount point must be a single safe path component. → Task 2 unit check.
5. **Licence agreement in a language other than the user's, malformed `LPic`, or `TEXT` in a non-Roman script encoding**: must not crash; falls back to the image's default language; text converted to UTF-8. → Task 3 tests.

---

### Task 1: Library + FUSE helper; `hdiutil` attach/detach on top

**Files:**
- Create: `src/external/darling-dmg/include/darling-dmg/darling_dmg.h` (public C API of the private library)
- Create: `src/external/darling-dmg/src/darling_dmg.cpp` (API implementation)
- Create: `src/external/darling-dmg/src/main-mount.cpp` (helper: FUSE loop + readiness pipe; reuses `main-fuse.cpp`)
- Modify: `src/external/darling-dmg/src/main-hdiutil.cpp` (attach/detach through the API)
- Modify: `src/external/darling-dmg/CMakeLists.txt` (Darling branch: `libdarling-dmg.dylib` → `/usr/lib/darling`, `darling-dmg-mount` → `/usr/libexec`, `hdiutil` links the library)
- Create: `tests/dmg-attach.sh`, `tests/dmg-attach.md`, `tests/dmg-api.c` (main repo)

**Interfaces:**
- Produces (C, `darling_dmg.h`; all functions return 0 or an errno value, never throw):
  ```c
  typedef struct darling_dmg_image darling_dmg_image;
  int  darling_dmg_open(const char* path, darling_dmg_image** out);   // ENOENT, EACCES, EFTYPE (not a disk image / unreadable format)
  void darling_dmg_close(darling_dmg_image* image);
  const char* darling_dmg_volume_name(darling_dmg_image* image);       // UTF-8; Task 1: file name without extension; Task 2: HFS+ name
  struct darling_dmg_attach_options {
      const char* mount_root;      // NULL = "/Volumes"
      const char* mount_point;     // NULL = mount_root/<volume name> (made unique in Task 2)
  };
  int  darling_dmg_attach(darling_dmg_image* image, const struct darling_dmg_attach_options* options,
                          char* mount_point, size_t mount_point_size);  // fills the mount point actually used
  int  darling_dmg_detach(const char* mount_point, int force);          // unmount() (+ MNT_FORCE), then rmdir of a directory attach created
  ```
- Helper contract (private to the library): `darling-dmg-mount <image> <mount point> <ready-fd>`; writes `"0\n"` to `ready-fd` from FUSE's `init` callback once the mount is live, or `"<errno> <message>\n"` and exits non-zero on failure; closes `ready-fd` either way; runs as a session leader (`POSIX_SPAWN_SETSID`). FUSE options: `-o ro,subtype=darling-dmg,fsname=<image path>` plus today's (`nonempty`, `-s`, foreground).

- [ ] **Step 1: Observe macOS attach behaviour** for the cases the tests pin, with `tests/fixtures/unmount-test.dmg`: `hdiutil attach <missing>`, `hdiutil attach <a text file>`, `hdiutil attach -plist <ok image>` (record which keys exist; Darling must keep emitting at least `system-entities[].mount-point`), and that the volume stays after the attaching shell exits. Record stdout/stderr/exit codes in `tests/dmg-attach.md`.

- [ ] **Step 2: Write the failing tests**
  - `tests/dmg-api.c` (guest program, links `-L$DARLING_ROOT/usr/lib/darling -ldarling-dmg`): `darling_dmg_open(<missing>)` → `ENOENT`; open of a text file → `EFTYPE`; open fixture → 0, `darling_dmg_volume_name` non-empty; attach with `mount_root` = a fresh temp dir → 0, `hello.txt` readable at the returned path; `statfs(mount point).f_fstypename` contains `"fuse"` or equals what Darling reports for the subtype (print it; assert the mount point matches `f_mntonname`); `darling_dmg_detach(mp, 0)` → 0 and the directory is gone; detach again → `EINVAL`.
  - `tests/dmg-attach.sh` (host driver, same style as `tests/unmount-disk-image.sh`): the Step 1 cases with the observed outputs; `attach survives caller` (attach in one `darling shell`, read `hello.txt` from a second one); two different images attached at once; `attach of a non-image` leaves no new directory under `/Volumes` and no `darling-dmg-mount` process (`ps` in the guest); then `tests/unmount-disk-image.sh` as a regression.

- [ ] **Step 3: Run them to verify they fail** (rootless prefix). Expected: `dmg-api` fails to link (no library); `dmg-attach.sh` fails at least the non-image cleanup check.

- [ ] **Step 4: Implement** the library, helper and CMake targets; rewrite `doAttach`/`doDetach` over the API keeping their option parsing and output formats (`-plist`, `-puppetstrings`, plain, `-quiet`, `-mountpoint`, `-mountroot`, `-mountrandom`). `doDetach` keeps plan-1 messages; the unmount moves into `darling_dmg_detach`. Remove the old in-process `daemon()` + `main_fuse` path from `hdiutil`.

- [ ] **Step 5: Run the tests in rootless and non-root modes** (`tests/dmg-attach.sh`, `dmg-api`, `tests/unmount-disk-image.sh`). Expected: all PASS.

- [ ] **Step 6: Commit** (darling-dmg `attach-library`; main repo tests force-added).

### Task 2: Mount under the HFS+ volume name

**Files:**
- Modify: `src/external/darling-dmg/src/darling_dmg.cpp` (`darling_dmg_volume_name`, default mount point)
- Modify: `src/external/darling-dmg/src/HFSVolume.{h,cpp}` (`std::string volumeName()` from the root folder's thread record, UTF-8)
- Modify: `tests/dmg-api.c`, `tests/dmg-attach.sh`, `tests/dmg-attach.md`

**Interfaces:**
- Consumes: Task 1 API.
- Produces: `std::string HFSVolume::volumeName()`; `darling_dmg_volume_name` returns it; default mount point = `<mount_root>/<sanitised name>`, made unique by the observed macOS rule (Step 1). Sanitising: `/` → `:` (macOS shows HFS `/` as `:` in POSIX paths and vice versa), empty name → `"Untitled"` (confirm on Mac).

- [ ] **Step 1: Observe on macOS:** attach `tests/fixtures/unmount-test.dmg` copied under another file name → mount point uses the volume name; attach two copies at once → record the second mount point exactly (e.g. `/Volumes/Unmount Test 1`); create an image whose volume name contains `/` (`hdiutil create -volname "A/B"`) and record its mount point. Record in `tests/dmg-attach.md`.
- [ ] **Step 2: Write failing tests:** `dmg-api.c` — fixture opened as `renamed.dmg` reports `"Unmount Test"`; `dmg-attach.sh` — the three Step 1 cases with the observed paths.
- [ ] **Step 3: Run to verify they fail** (today: file-name based mount point).
- [ ] **Step 4: Implement** `volumeName()` and the naming rule.
- [ ] **Step 5: Run all disk-image tests** (rootless, non-root). Expected: PASS.
- [ ] **Step 6: Commit.**

### Task 3: Read licence agreements

**Files:**
- Create: `src/external/darling-dmg/src/LicenseAgreement.{h,cpp}` (parse `LPic`, `STR#`, `TEXT`/`RTF ` from the UDIF plist `resource-fork` dict, which `DMGDisk` already loads with libxml2)
- Modify: `src/external/darling-dmg/include/darling-dmg/darling_dmg.h`, `src/darling_dmg.cpp`
- Create: `tests/fixtures/sla-test.dmg`, `tests/fixtures/sla-test.r.xml` (resource source we wrote), `tests/dmg-license.c`, `tests/dmg-license.md`

**Interfaces:**
- Produces:
  ```c
  struct darling_dmg_license {
      const char* language;        // BCP-47 tag derived from the Mac region code, e.g. "en", "de", "ja"
      int is_default;              // the image's default language (LPic)
      int is_rtf;                  // text is RTF (from an 'RTF ' resource) rather than plain UTF-8
      const char* text;            // UTF-8 (TEXT converted from the region's Mac script encoding via ICU/iconv)
      const char* language_name;   // STR# entries, UTF-8; NULL if the image has none
      const char* agree;
      const char* disagree;
      const char* print;
      const char* save;
      const char* message;
  };
  size_t darling_dmg_license_count(darling_dmg_image* image);           // 0 = no agreement
  const struct darling_dmg_license* darling_dmg_license_at(darling_dmg_image* image, size_t index);
  const struct darling_dmg_license* darling_dmg_license_for_languages(darling_dmg_image* image,
      const char* const* preferred, size_t count);                        // first match, else the default
  ```
- [ ] **Step 1: Make the fixture on macOS:** write `sla-test.r.xml` (resource-file XML with an `LPic` naming English as default plus German, an English `TEXT`, a German `TEXT` with non-ASCII characters in Mac Roman, and `STR#` button sets for both), `hdiutil udifrez -xml sla-test.r.xml` onto a copy of the unmount fixture, and confirm macOS presents it (`hdiutil imageinfo` or an interactive `hdiutil attach` in a pty via `script`). Record what the agreement looks like in `tests/dmg-license.md`.
- [ ] **Step 2: Write failing test** `dmg-license.c`: unmount fixture → count 0; SLA fixture → count 2, default is `"en"`, German text contains the exact non-ASCII string from the resource file as UTF-8, buttons match; `for_languages({"de"})` → German; `for_languages({"fr"})` → English default. Malformed-input cases (Review Focus 5): images built in the test by truncating the `LPic` data in a copy of the plist (driver-side), expecting count 0 and no crash.
- [ ] **Step 3: Run to verify it fails** (symbols missing).
- [ ] **Step 4: Implement `LicenseAgreement`** (region code → language tag and Mac encoding table limited to the regions the public SDK lists; unknown region → skip that entry).
- [ ] **Step 5: Run tests** (rootless). Expected: PASS.
- [ ] **Step 6: Commit.**

### Task 4: `hdiutil attach` honours licence agreements

**Files:**
- Modify: `src/external/darling-dmg/src/main-hdiutil.cpp`
- Modify: `tests/dmg-license.md`; Create: `tests/dmg-license.sh`

**Interfaces:**
- Consumes: Task 3 API.
- Produces: `hdiutil attach` behaviour for images with an agreement, exactly as observed on macOS in Step 1 (prompt text, pager use, accepted answers, behaviour with stdin not a terminal, with `-plist`, with `-quiet`, any pre-accepting option). Undocumented parts carry a `QUIRK:` comment citing the observation and the test.

- [ ] **Step 1: Observe on macOS** with `tests/fixtures/sla-test.dmg`: `hdiutil attach` in a pty answering `Y`, answering `N`; `hdiutil attach </dev/null`; `yes | hdiutil attach`; `hdiutil attach -plist </dev/null`; `hdiutil attach -quiet </dev/null`. Record stdout/stderr/exit status/whether mounted in `tests/dmg-license.md`.
- [ ] **Step 2: Write failing test** `tests/dmg-license.sh` reproducing each Step 1 case in Darling (pty via `script -qc` on the host side of `darling shell` where a terminal is needed) and asserting the observed outputs, exit statuses and mount state; plus "declined → nothing under `/Volumes`, no helper process".
- [ ] **Step 3: Run to verify it fails** (today the agreement is ignored and the image mounts).
- [ ] **Step 4: Implement** the prompt flow in `doAttach` before calling `darling_dmg_attach`.
- [ ] **Step 5: Mode matrix:** all disk-image tests (`unmount-disk-image.sh`, `dmg-attach.sh`, `dmg-api`, `dmg-license.sh`) in rootless, non-root and (with the user's OK for the temporary setuid) root mode. Expected: all PASS.
- [ ] **Step 6: Commit and push** darling-dmg `attach-library` and the main-repo bump on `finder-app` (no PR).
