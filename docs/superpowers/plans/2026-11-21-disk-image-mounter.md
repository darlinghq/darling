# DiskImageMounter.app Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Opening a `.dmg` in Darling (`open x.dmg`, or any app opening it through LaunchServices) runs DiskImageMounter. It asks for the image's licence agreement when there is one, attaches the image through libdarling-dmg, and opens the mounted volume through LaunchServices, which hands it to the file viewer. Then it quits.

**Architecture:**
- DiskImageMounter is a new in-tree ObjC app at `src/DiskImageMounter`. It is installed as `/System/Library/CoreServices/DiskImageMounter.app` and declares the disk-image types, so LaunchServices sends it `.dmg` files.
- Two lower layers get what it needs, at the layer where macOS has it:
  - **libdarling-dmg:** attaching an image that is already attached returns its existing mount point, as `hdiutil attach` does on macOS.
  - **CoreFoundation and LaunchServices:** a mount root's type is `public.volume`, and a folder or volume is opened by the application that declares that type. Finder declares it through the `'fold'`/`'disk'` OSTypes. Without such an application, today's fallback to the host file manager stays.
- Finder itself is plan 7. Here a stand-in bundle with Finder's bundle ID and document types receives the open event.

**Tech Stack:** Objective-C (AppKit and Foundation through cocotron), C++ (darling-dmg), CoreFoundation C, sqlite (launchservicesd), shell test harnesses (xdotool on X11, wtype and swaymsg on Wayland).

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md` (sections "Disk images: attach library, hdiutil, DiskImageMounter" and "Apps: structure"; roadmap `docs/superpowers/plans/2026-11-16-finder-roadmap.md`, row 6).

## Global Constraints

- AGENTS.md governs everything:
  - clean room: never disassemble Apple binaries; the Macs are for black-box observation only;
  - no app-specific logic;
  - `QUIRK:` comments carry evidence and a test;
  - no debug leftovers;
  - every commit has a Provenance note;
  - fix at the right layer.
- Language: DiskImageMounter is Objective-C only. Framework and system work is in the language of the component it lands in.
- Bundle identity: `com.apple.DiskImageMounter`, `/System/Library/CoreServices/DiskImageMounter.app`, `LSUIElement` true. Its `Info.plist` has `CFBundleDocumentTypes` for disk images (`com.apple.disk-image-udif` and related, role Viewer). It exports `com.apple.disk-image-udif`: conforms to `com.apple.disk-image` and `public.disk-image`; extensions `dmg`, `udif`, `img`; MIME type `application/x-apple-diskimage`.
- DiskImageMounter attaches without `-nobrowse` semantics. After attaching, it opens the volume's root through `NSWorkspace`/LaunchServices, then quits. It has no Dock-visible windows except the licence agreement and error alerts. Images stay mounted when it quits.
- Licence: show a window with the agreement, a language popup if several are present, and Agree / Disagree. Disagree means nothing is mounted. Mounting an image with a licence agreement without the user's acceptance is an anti-goal.
- All artwork is our own. Icons follow AppKit's precedent: rendered from the Adwaita icon theme with `rsvg-convert`, and listed in a `COPYING.Adwaita` next to them.
- Build tree: `~/Projects/darling/build` (`-DCMAKE_INSTALL_PREFIX=$HOME/darling-finder`). Install only into `~/darling-finder`.
- After any install:
  - `sudo -n chown root:root ~/darling-finder/bin/darling && sudo -n chmod 4755 ~/darling-finder/bin/darling`;
  - refresh the plain copy `~/darling-finder/rootless-bin/darling` (mode 755).
- Temp files go in `~/Projects/darling/build/tmp`. Run every test with `TMPDIR` there (`build/tmp/helpers/mode.sh` sets it). Never use `/tmp`.
- GUI tests:
  - X11: the private Xvfb `:97`, with `unset WAYLAND_DISPLAY SWAYSOCK`. Never `:99`, `:96`, `:98` or the user's desktop.
  - Wayland: the private sway (`XDG_RUNTIME_DIR=$HOME/Projects/darling/build/tmp/wl`, `SWAYSOCK=…/wl/sway-ipc.sock`, `WAYLAND_DISPLAY=…/wl/wayland-1`).
- Modes: every runtime test passes in rootless, non-root and root mode (`build/tmp/helpers/all-modes.sh`).
- Branches:
  - corefoundation: `disk-image-mounter`, from `finder-volumes` HEAD;
  - darling-dmg: `disk-image-mounter`, from `disk-arbitration` HEAD;
  - darling: `finder-app`. It records the submodule bumps; never `git add -A` or `commit -a`.
- Push branches, open no pull requests.
- Commits end with:
  - `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  - `Claude-Session: https://claude.ai/code/session_01CRFMixwsFg4yEYzqovZF1D`
- Do not touch the i386 `NSStringDrawingContext` regression; another agent owns it.

## Observed on macOS 26.7.3 (the probes behind this plan)

These were recorded on 10.10.10.152 while writing the plan. Task 6 writes them into `tests/disk-image-mounter.md` as tier-2 evidence.

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

## Rulings in this plan

1. **No Print or Save buttons.**
   - Why: the spec names only the language popup, Agree and Disagree.
   - Cost if wrong: two buttons added later.
2. **The agreement is shown as libdarling-dmg returns it**, with line breaks kept and word wrap in the text view. RTF is shown as attributed text.
   - Why: hdiutil's reflow exists for terminals.
   - Cost if wrong: a cosmetic difference.
3. **Licence window title and keys:**
   - The title is the image's file name.
   - Return is Agree (the default button) and Escape is Disagree.
   - Closing the window counts as Disagree.
   - Why: our design; nothing could be observed (no screenshots).
   - Cost if wrong: cosmetic, or a key that does something different.
4. **Strings the image doesn't provide** have our own English defaults: "Agree", "Disagree", and the prompt "To use the contents of this disk image, you must agree to its license."
5. **Errors:** an `NSAlert` with the message "“<file name>” could not be opened.". Its informative text is:
   - "The file is not a disk image this system can read." for `EFTYPE`;
   - otherwise `strerror`, followed by the lines of `darling_dmg_attach_messages` when there are any.
   - Why: our wording; we can't see the Mac's.
6. **No documents:** without documents, DiskImageMounter quits once launching is done (macOS keeps it running).
   - Why: a Darling container should not keep an idle UI process alive.
   - Cost if wrong: none for users; a later `open` launches it again.
7. **Image identity is the file's real path.** An image counts as already attached when a darling-dmg mount (`f_fstypename` `fuse.darling-dmg` or `darling-dmg`, as the kernel layer reports it) has `f_mntfromname` equal to `realpath` of the image.
   - Why: this matches the symlink observation.
   - Cost if wrong: a hard link to an attached image attaches again.
8. **Several images in one open event** are handled one after another, in order, each with its own licence window. DiskImageMounter quits after the last one.
9. **Attaching runs on a background dispatch queue**, so a licence or error window never blocks it. Results come back on the main queue.
10. **The folder and volume handler is found by type, never by bundle ID.** This plan does not hard-code `com.apple.finder` in LaunchServices. The stand-in Finder of the tests declares the same OSTypes as macOS's Finder.

## Review Focus

1. **Image paths with spaces, non-ASCII characters or `&`, and images under `/Volumes/SystemRoot/...` (the host).** Expected: attached and opened like any other.
   - Task 4 runs `open` on `$gw/Dir With Space/ünï & co.dmg` (a copy of `unmount-test.dmg`).
2. **Two opens of a licence image while its window is still showing.** Expected: one window, one attach.
   - Task 6: `open sla-test.dmg` twice, one second apart, before answering. Exactly one window titled `sla-test.dmg` exists. After Agree, `mount` in the guest shows one darling-dmg mount of it.
3. **Quitting DiskImageMounter, or closing the licence window, while the agreement shows.** Expected: nothing mounted.
   - Task 5: the controller's close-button path.
   - Task 6: `kill` while the window shows, then no mount.
4. **An attach that fails after a licence was agreed to** (the FUSE mount helper fails). Expected: an error alert, nothing left mounted, the mount point directory removed, and DiskImageMounter quits.
   - Task 4 uses `bad-blkx.dmg` for the not-a-disk-image path.
   - Task 4 also runs the image-opener test with an `attach` stub that returns `EIO` and messages, through the opener's attach-function hook.
5. **An image that is already attached, and has a licence.** Expected: no licence window. The volume is opened again.
   - Task 3: the library test.
   - Task 6: attach `sla-test.dmg` with `echo y | hdiutil attach`, then `open` it. No window appears and the stand-in Finder gets `odoc`.

---

### Task 1: A mount root's type is `public.volume` (CoreFoundation)

**Files:**
- Modify: corefoundation `NSURL.m` (`CFURLCreateTypeIdentifier`, the `S_ISDIR` branch around :311)
- Modify: corefoundation `tests/nsurl-volume-keys.m` (add `NSURLTypeIdentifierKey` to the keys it prints)
- Test: darling `tests/volumes.sh` (the "volume keys" case)

**Interfaces:**
- Produces: `NSURLTypeIdentifierKey`/`kCFURLTypeIdentifierKey` gives:
  - `public.volume` for a directory whose path, with any trailing `/` removed (`/` stays `/`), equals the `f_mntonname` of `statfs` on it;
  - otherwise today's result (a declared package or bundle type by extension, else `public.folder`).

- [ ] **Step 1: Write the failing test.**
  - Add `NSURLTypeIdentifierKey` to the key list in `nsurl-volume-keys.m`.
  - In the "volume keys" case of `tests/volumes.sh`, add:
    - `expect_key keys / TypeIdentifier public.volume`
    - `expect_key keys '/Volumes/Unmount Test/' TypeIdentifier public.volume`
    - `expect_key keys-after /Volumes TypeIdentifier public.folder`
    - `expect_key keys '/Volumes/Unmount Test/hello.txt' TypeIdentifier public.plain-text`
- [ ] **Step 2: Run it to see it fail.**
  - Run: `. build/tmp/helpers/mode.sh rootless && sh tests/volumes.sh 2>&1 | grep -E 'volume keys|TypeIdentifier'`
  - Expected: `FAIL: volume keys`, with `/: TypeIdentifier is 'public.folder', expected 'public.volume'` and the same for the image.
- [ ] **Step 3: Implement it.**
  - In the `S_ISDIR` branch, before the extension lookup: a directory that is a mount root gives `public.volume`. A volume is a volume even when its name has an extension.
  - Check it with `statfs` (already used in that file for the volume keys).
- [ ] **Step 4: Run it to see it pass.**
  - Rebuild and install CoreFoundation, then run `tests/volumes.sh` in all three modes.
  - Expected: `PASS: volume keys` and no new FAIL.
- [ ] **Step 5: Commit**
  - In corefoundation `disk-image-mounter`: "NSURL: a mount root's type identifier is public.volume". Provenance: the macOS 26.7.3 observation above.
  - In darling `finder-app`: the bump and the `volumes.sh` lines.
  - Push both.

### Task 2: LaunchServices opens folders and volumes with the application declaring their type

**Files:**
- Modify: `src/frameworks/CoreServices/CoreTypes/Info.plist`: `public.folder` gets `UTTypeTagSpecification` `com.apple.ostype` = `fold`, and `public.volume` gets `disk`.
- Modify: `src/frameworks/CoreServices/src/LaunchServices/launchservicesd/schema.sql`:
  - new table `app_doc_ostype` (`id`, `doc` → `app_doc(id)` ON DELETE CASCADE, `ostype` TEXT), with indexes on `doc` and `ostype`;
  - `globals.version` becomes `'3'`.
- Modify: `src/frameworks/CoreServices/src/LaunchServices/launchservicesd/LSBundle.m`:
  - `DATABASE_VERSION` becomes 3;
  - `upgradeDBSchemaTo3` creates the table and clears the checksums, as `upgradeDBSchemaTo2` does. `setupDBSchema` runs 2, then 3, as needed;
  - in the branch without `LSItemContentTypes`, each string of `CFBundleTypeOSTypes` is inserted into `app_doc_ostype`.
- Modify: `src/frameworks/CoreServices/src/LaunchServices/LSInfo.m` (`LSGetApplicationForURL`, :337)
- Create: `tests/launchservices-folder-handler.py`: SQL-level test, in the style of `launchservices-extension-query.py`.
- Create: `tests/src/launchservices_folder_handler.m`: the runtime test.
- Modify: `tests/volumes.sh`: new case "folder handler".

**Interfaces:**
- Consumes: Task 1's `public.volume`.
- Produces: `LSGetApplicationForURL` and `LSCopyDefaultApplicationURLForURL`, for a `file:` URL of an existing directory:
  - An application bundle keeps today's behaviour: the extension lookup finds `.app`.
  - Otherwise LaunchServices takes the URL's `NSURLTypeIdentifierKey`. It walks that type and every type it conforms to, nearest first (`public.volume`, then `public.folder`, then `public.directory`, …). For each type it looks for an application document type that:
    - lists the type in `LSItemContentTypes`; or
    - lists one of the type's `com.apple.ostype` tags in `CFBundleTypeOSTypes`; or
    - lists one of the type's extension tags in `CFBundleTypeExtensions`.
  - Rank and role filtering are the same as `LSGetApplicationForInfo`.
  - The first match wins. With none, `kLSApplicationNotFoundErr`, so `is_folder_without_handler` still sends the folder to the host.
  - Files keep today's extension lookup.

- [ ] **Step 1: Write the failing tests.**
  - `launchservices-folder-handler.py` loads `schema.sql` into sqlite. It checks:
    - `app_doc_ostype` exists, and `globals.version` is `3`;
    - a document type with OSType `disk` is found by the OSType query that `LSInfo.m` uses (extract it the way `launchservices-extension-query.py` does);
    - an Alternate-rank one loses to a Default one.
  - `launchservices_folder_handler.m <dir>` registers `<dir>/Finder.app` with `LSRegisterURL`. The bundle is made by `make_app`, with an `Info.plist` that adds Finder's two document types (`fold` and `disk`, role Editor, no content types) and bundle ID `com.apple.finder`. The program then prints `LSCopyDefaultApplicationURLForURL(url, kLSRolesAll, NULL)`'s path for `<dir>/plain`, `<dir>/odd.foo`, `/`, `/Volumes/Unmount Test` and `<dir>/x.rtfd`. Expected:
    - the stand-in Finder for the first four;
    - for `x.rtfd`, TextEdit (`/Applications/TextEdit.app`, which declares `rtfd`).
  - The "folder handler" case of `volumes.sh` attaches `unmount-test.dmg`, runs the program, then detaches.
- [ ] **Step 2: Run them to see them fail.**
  - Run: `python3 tests/launchservices-folder-handler.py`
    - Expected: FAIL, no such table `app_doc_ostype`.
  - Run: `sh tests/volumes.sh 2>&1 | grep 'folder handler'`
    - Expected: FAIL, "(none)" for the four directories.
- [ ] **Step 3: Implement the CoreTypes tags, schema v3 with its upgrade, registration of `CFBundleTypeOSTypes`, and the directory lookup in `LSGetApplicationForURL`.**
  - Walk conformance with `UTTypeCopyDeclaration`'s `UTTypeConformsTo` (breadth first, each type once).
  - Read tags with `UTTypeCopyAllTagsWithClass`.
- [ ] **Step 4: Run them to see them pass.**
  - Rebuild and install CoreServices and launchservicesd. The upgrade must work on the existing prefixes: do not delete their databases.
  - Run the Python test, then `tests/volumes.sh` in all three modes. Also run every `tests/launchservices-*` test (`.py` and `.rb`) that the suite already runs.
  - Expected: all PASS.
- [ ] **Step 5: Commit** (darling `finder-app`): "LaunchServices: folders and volumes open with the application declaring their type". Provenance: the CFBundleDocumentTypes documentation (CFBundleTypeOSTypes), the UTI documentation (conformance, tag classes), and the macOS 26.7.3 observations above. Push.

### Task 3: Attaching an attached image returns its mount point (libdarling-dmg, hdiutil)

**Files:**
- Modify: darling-dmg `include/darling-dmg/darling_dmg.h`, `src/darling_dmg.cpp` (`darling_dmg_attach`), `src/main-hdiutil.cpp` (the licence check at :264)
- Modify: darling `tests/dmg-api.c`, `tests/dmg-attach.sh`, `tests/dmg-license.sh`, `tests/dmg-attach.md`

**Interfaces:**
- Produces:
  - `int darling_dmg_attached_mount_point(darling_dmg_image* image, char* mount_point, size_t mount_point_size);`
    - Returns 0 and the mount point when the image (by its `realpath`, which `darling_dmg_open` already keeps) is mounted by darling-dmg.
    - `ENOENT` when it is not; `ENAMETOOLONG` when the buffer is too small.
  - `darling_dmg_attach` returns 0 and that existing mount point, without mounting again, when the image is already attached. Options are ignored then.
  - `hdiutil attach` checks it before asking for the licence agreement. For an attached image it asks nothing and prints the usual table (or plist) for the existing mount.

- [ ] **Step 1: Write the failing tests.**
  - `dmg-api.c`:
    - before any attach, `darling_dmg_attached_mount_point` is `ENOENT`;
    - after attach, it gives `/Volumes/Unmount Test`;
    - a second `darling_dmg_open` of a symlink to the image, then `darling_dmg_attach`, gives `/Volumes/Unmount Test` again, and `getfsstat` shows one darling-dmg mount for it;
    - a copy of the file attaches at `/Volumes/Unmount Test 1`.
  - `dmg-attach.sh`, case "attach twice": `hdiutil attach` twice exits 0 both times, and the last field of the volume line is `/Volumes/Unmount Test` both times.
  - `dmg-license.sh`, case "attached image asks nothing": `printf 'y\n' | hdiutil attach sla-test.dmg`, then `hdiutil attach sla-test.dmg </dev/null`. Expected: exit 0, and no agreement text on stdout.
- [ ] **Step 2: Run them to see them fail.**
  - Run: `TESTS="tests/dmg-attach.sh tests/dmg-license.sh" build/tmp/helpers/all-modes.sh`
  - Expected: FAIL "attach twice" (the second mount is `Unmount Test 1`), FAIL "attached image asks nothing", and `dmg-api` failing to build (no symbol).
- [ ] **Step 3: Implement it.** Scan with `getfsstat(MNT_NOWAIT)`. Compare `f_mntfromname` with the image's real path, and require a darling-dmg file system type. Check what `statfs` reports for a darling-dmg mount (plan 3's `diskarbitrationd` already tells them apart) and use the same test.
- [ ] **Step 4: Run them to see them pass.** Same command, all three modes. Expected: every case PASS.
- [ ] **Step 5: Commit**
  - darling-dmg `disk-image-mounter`: "Attaching an image that is attached returns its mount point".
  - darling: the bump, the tests, and the `dmg-attach.md` observations above.
  - Provenance: the macOS 26.7.3 `hdiutil` observations.
  - Push both.

### Task 4: DiskImageMounter.app: open, attach, open the volume, report errors

**Files:**
- Create `src/DiskImageMounter/`:
  - `CMakeLists.txt`: the TextEdit pattern (Info.plist, PkgInfo, `.icns` from an iconset with `iconset2icns`). It returns early when `NOT COMPONENT_gui`, and installs into `libexec/darling/System/Library/CoreServices/DiskImageMounter.app/Contents` with `COMPONENT gui`.
  - `Info.plist`
  - `main.m`
  - `DIMAppDelegate.h/.m`
  - `DIMImageOpener.h/.m`
  - `DiskImageMounter.iconset/` (Adwaita `devices/drive-removable-media`, at 16, 32, 128, 256 and 512 px, plus @2x)
  - `COPYING.Adwaita`
- Modify: `src/CMakeLists.txt`: `add_subdirectory(DiskImageMounter)` right after `add_subdirectory(external/darling-dmg)` in the `COMPONENT_cli` block, where its library is.
- Create: `tests/src/dim_image_opener.m`: in-process test of `DIMImageOpener`, built with guest-cc from the test and `src/DiskImageMounter/DIMImageOpener.m`, `-framework AppKit -ldarling-dmg`.
- Create: `tests/disk-image-mounter.sh`, cases "opener", "open", "open twice", "odd path" and "bad image".
- Modify: cocotron `tests/nsworkspace-icon-kinds.m` only if a `.dmg` check now fails. They used to SKIP. Never weaken a check.

**Interfaces:**
- Consumes:
  - Task 2's folder handler; Task 3's `darling_dmg_attach` reuse;
  - libdarling-dmg's `darling_dmg_open`, `darling_dmg_license_count`, `darling_dmg_license_for_languages`, `darling_dmg_attach`, `darling_dmg_attach_messages` and `darling_dmg_close`.
- Produces:

```objc
// DIMImageOpener.h
typedef int (*DIMAttachFunction)(darling_dmg_image *, const struct darling_dmg_attach_options *, char *, size_t);

@protocol DIMImageOpenerUI <NSObject>
// Asks for agreement; calls reply exactly once, on the main queue.
- (void)imageOpener:(DIMImageOpener *)opener askAgreementForImage:(darling_dmg_image *)image
              reply:(void (^)(BOOL agreed))reply;
- (void)imageOpener:(DIMImageOpener *)opener showError:(NSString *)message
        informative:(NSString *)informative completion:(void (^)(void))done;
@end

@interface DIMImageOpener : NSObject
- (instancetype)initWithURL:(NSURL *)imageURL ui:(id<DIMImageOpenerUI>)ui;
@property (nonatomic) DIMAttachFunction attachFunction;        // default darling_dmg_attach (tests replace it)
@property (nonatomic, copy) void (^openVolume)(NSURL *volume); // default: [[NSWorkspace sharedWorkspace] openURL:]
// Runs the whole flow, calls done on the main queue with the mounted volume's URL, or nil when
// nothing was opened (disagreed, or an error that was shown).
- (void)startWithCompletion:(void (^)(NSURL *volume))done;
@end
```

  - The flow:
    1. `darling_dmg_open`.
    2. `darling_dmg_attached_mount_point`: when the image is already attached, go straight to step 5.
    3. If there is a licence, ask for agreement (`askAgreementForImage:` gets the open image; Task 5's window reads its languages). Disagree means done(nil).
    4. Attach on a background queue.
    5. `openVolume` with the mount point as a directory URL, then `done`.
    - Errors use Ruling 5's texts.
  - `DIMAppDelegate`:
    - `application:openURLs:` queues the URLs. A URL that is already queued or in progress is not queued again (Review Focus 2).
    - It runs one opener at a time, and calls `[NSApp terminate:nil]` when the queue is empty and nothing runs.
    - It checks once after `applicationDidFinishLaunching:` (on the next run-loop turn), for Ruling 6.
    - It is the `DIMImageOpenerUI`: errors go to an `NSAlert`. In this task, agreement is answered by a minimal alert with Agree/Disagree; Task 5 replaces it with the licence window.
  - `Info.plist`:
    - `CFBundleIdentifier` `com.apple.DiskImageMounter`, `CFBundleExecutable` `DiskImageMounter`, `CFBundlePackageType` `APPL`, `LSUIElement` true, `NSPrincipalClass` `NSApplication`;
    - `CFBundleDocumentTypes`: one type, "Disk Image", role Viewer, `LSItemContentTypes` [`com.apple.disk-image-udif`, `public.disk-image`];
    - `UTExportedTypeDeclarations`: `com.apple.disk-image-udif`, as in Global Constraints.

- [ ] **Step 1: Write the failing tests.**
  - `dim_image_opener.m <dir>`, with a recording fake UI and `openVolume` recording its URL:
    - `unmount-test.dmg`: no agreement asked, `openVolume` gets `file:///Volumes/Unmount%20Test/`, and done gets the same URL;
    - `sla-test.dmg` with the fake answering NO: agreement asked once, nothing mounted (`darling_dmg_attached_mount_point` is `ENOENT`), done(nil), and `openVolume` not called;
    - the same with YES: mounted and opened;
    - `bad-blkx.dmg`: `showError:` with "“bad-blkx.dmg” could not be opened." and "The file is not a disk image this system can read.", then done(nil);
    - a stub `attachFunction` that returns `EIO`: `showError:` informative starting with `strerror(EIO)`, done(nil), nothing mounted (Review Focus 4);
    - it detaches what it attached between cases.
  - `disk-image-mounter.sh`. It needs `DISPLAY` (X11) or the Wayland variables. It makes the stand-in Finder (`make_app` with Task 2's document types, plus `ae-receiver`) and registers it. Cases:
    - "opener": runs `dim_image_opener` and passes on exit 0;
    - "open": `open '$gw/unmount-test.dmg'`. Within 30 s, the stand-in's `events.txt` has `aevt odoc file:///Volumes/Unmount%20Test/`. Within 10 s more, no `DiskImageMounter` process is left (`pgrep -f` in the guest). The volume stays mounted;
    - "open twice": `open` it again. Still one darling-dmg mount for it, and a second `odoc` line;
    - "odd path" (Review Focus 1): the same for `$gw/Dir With Space/ünï & co.dmg`;
    - "bad image": `open bad-blkx.dmg`. A window of the DiskImageMounter process appears within 30 s (X11: `xdotool search --class DiskImageMounter`, since the guest's pids are not the host's; Wayland: an `app_id` of `DiskImageMounter` in `swaymsg -t get_tree`. If the backends do not set the class or app ID from the bundle, that is a finding for the backend, fixed test-first in cocotron). After Return, the process exits within 10 s and nothing is mounted.
  - Every case detaches afterwards, and the script removes the stand-in bundle.
- [ ] **Step 2: Run them to see them fail.**
  - Run: `. build/tmp/helpers/mode.sh rootless && DISPLAY=:97 sh tests/disk-image-mounter.sh`
  - Expected: FAIL in every case (no app, and the opener test does not build).
- [ ] **Step 3: Implement the app.**
  - Render the iconset with `rsvg-convert -w N -h N /usr/share/icons/Adwaita/scalable/devices/drive-removable-media.svg`.
  - `COPYING.Adwaita` states the theme version, licence (LGPLv3 or CC-BY-SA 3.0) and source, as AppKit's does.
- [ ] **Step 4: Run them to see them pass.**
  - Build `DiskImageMounter` and install it, plus AppKit if touched. Then run `disk-image-mounter.sh` in all three modes, on X11 (`:97`) and on Wayland.
  - Also run `tests/volumes.sh`. Expected: the `.dmg` checks of `nsworkspace-icon-kinds` now run instead of printing SKIP, and pass.
  - Expected: all PASS.
- [ ] **Step 5: Commit** (darling `finder-app`): "DiskImageMounter: open disk images and their volumes". Provenance: the spec, the CFBundleDocumentTypes/UTExportedTypeDeclarations documentation, the macOS 26.7.3 observations above, and Adwaita artwork. Push.

### Task 5: The licence agreement window

**Files:**
- Create: `src/DiskImageMounter/DIMLicenseWindowController.h/.m`
- Modify: `src/DiskImageMounter/DIMAppDelegate.m` (its agreement answer uses the controller)
- Modify: `tests/fixtures/make-sla-fixture.py`, adding variants:
  - `one`: English only;
  - `rtf`: English as an `RTF ` resource `{\rtf1\ansi SLA TEST AGREEMENT {\b bold}\par}`;
  - `nostrings`: no `STR#`.
- Create: `tests/fixtures/sla-one.dmg`, `sla-rtf.dmg`, `sla-nostrings.dmg`. Build them on 10.10.10.152 with `hdiutil udifrez`, as `tests/dmg-license.md` describes.
- Modify: `tests/dmg-license.md` (the new fixtures' table rows)
- Create: `tests/src/dim_license_window.m`: in-process test, built with the controller's sources.
- Modify: `tests/disk-image-mounter.sh` (case "license window")

**Interfaces:**
- Consumes: `struct darling_dmg_license` and `darling_dmg_license_at`/`_count`/`_for_languages`.
- Produces:

```objc
@interface DIMLicenseWindowController : NSWindowController
- (instancetype)initWithImage:(darling_dmg_image *)image title:(NSString *)title
           preferredLanguages:(NSArray<NSString *> *)languages;   // the app passes [NSLocale preferredLanguages]
- (void)runWithReply:(void (^)(BOOL agreed))reply;               // shows the window; reply once
@property (readonly) NSPopUpButton *languagePopUp;               // hidden when the image has one language
@property (readonly) NSTextView *textView;
@property (readonly) NSTextField *promptField;
@property (readonly) NSButton *agreeButton, *disagreeButton;
@end
```

  - The window:
    - It is a titled, closable, non-resizable-minimum window. Its title is the image's file name (Ruling 3).
    - Top: the popup lists `language_name`, or the BCP 47 tag when that is NULL, in the image's order.
    - The initial selection is `darling_dmg_license_for_languages(image, preferred)`.
    - Middle: a scrolling, read-only, word-wrapping `NSTextView`. It shows `text` (plain), or `-[NSAttributedString initWithRTF:documentAttributes:]` of it when `is_rtf`.
    - Below: the `message` prompt, then Disagree (key Escape) and Agree (key Return, default button).
    - Choosing a language replaces the text, prompt and button titles; NULL strings use Ruling 4's defaults.
    - Agree replies YES; Disagree or closing the window replies NO (`windowShouldClose:`). Each replies once, then orders the window out.

- [ ] **Step 1: Write the failing tests.**
  - `dim_license_window.m <fixtures dir>` (needs a display; prints PASS/FAIL lines):
    - `sla-test.dmg` with `@[@"de-DE"]`:
      - the popup has 3 items, "English", "Deutsch", "日本語", and "Deutsch" is selected, not hidden;
      - the text view string begins "SLA TEST AGREEMENT (Deutsch)";
      - the buttons are "Akzeptieren" and "Ablehnen";
      - the prompt is "Klicken Sie auf „Akzeptieren“, wenn Sie einverstanden sind.";
      - the window title is "sla-test.dmg";
    - selecting index 0 (`selectItemAtIndex:` plus `sendAction`) switches to the English text, "Agree", "Disagree" and the English prompt;
    - `@[@"fr-FR"]` selects the default (English); `@[@"ja"]` selects 日本語, with text "SLA TEST AGREEMENT (日本語)…";
    - `sla-one.dmg`: the popup is hidden;
    - `sla-rtf.dmg`: the text view string is "SLA TEST AGREEMENT bold\n", and "bold" has a bold font trait;
    - `sla-nostrings.dmg`: "Agree", "Disagree", and Ruling 4's prompt;
    - `performClick:` on Agree replies YES once; on a new controller, Disagree replies NO; on a third, `performClose:` on the window replies NO (Review Focus 3).
  - `disk-image-mounter.sh`, case "license window", on X11 and on Wayland:
    - `open sla-test.dmg`, and a window titled `sla-test.dmg` appears;
    - Escape: nothing mounted, the process exits;
    - `open` again, Return: the volume is mounted and the stand-in gets `odoc`.
- [ ] **Step 2: Run them to see them fail.**
  - Run: `DISPLAY=:97 sh tests/disk-image-mounter.sh 2>&1 | grep -E 'license window'`
  - Expected: FAIL. The window is the minimal alert, not titled `sla-test.dmg`, and the in-process test does not build.
- [ ] **Step 3: Implement the controller and the fixtures.** Then switch `DIMAppDelegate`'s agreement answer to it, keeping one controller alive per image until its reply.
- [ ] **Step 4: Run them to see them pass.**
  - Run `dim_license_window` on X11 and Wayland, `disk-image-mounter.sh` in all three modes on both, and `tests/dmg-license.sh` (the new fixtures must not change its cases).
  - Expected: all PASS.
- [ ] **Step 5: Commit** (darling `finder-app`): "DiskImageMounter: the licence agreement window". Provenance: the spec, public descriptions of the UDIF licence resources (as for plan 2), and our own fixtures. Push.

### Task 6: The flows the tests above leave out, and the record

**Files:**
- Modify: `tests/disk-image-mounter.sh`, cases "license twice", "kill during license" and "attached license image"
- Create: `tests/disk-image-mounter.md`: how to run it, plus the observations section above, verbatim from this plan.
- Modify: `docs/superpowers/plans/2026-11-16-finder-roadmap.md`: row 6 becomes `done: 2026-11-21-disk-image-mounter.md`, with Repos "corefoundation, darling-dmg, darling (CoreServices, DiskImageMounter)".

- [ ] **Step 1: Write the cases.**
  - "license twice" (Review Focus 2): `open sla-test.dmg` twice, one second apart. Exactly one window titled `sla-test.dmg`. After Return, one mount.
  - "kill during license" (Review Focus 3): `open sla-test.dmg`, wait for the window, then kill DiskImageMounter in the guest. Nothing mounted.
  - "attached license image" (Review Focus 5): `printf 'y\n' | hdiutil attach sla-test.dmg`, then `open` it. No window within 10 s, the stand-in gets `odoc`, and the process exits.
- [ ] **Step 2: Run them.**
  - Run: all three modes, X11 and Wayland.
  - Expected: PASS. If one fails, the cause is in Task 4 or 5's code. Fix it there test-first: the failing case is the test.
- [ ] **Step 3: Run the whole set.**
  - Run in all three modes: `TESTS="tests/volumes.sh tests/dmg-attach.sh tests/dmg-license.sh tests/diskarbitration.sh tests/disk-image-mounter.sh" build/tmp/helpers/all-modes.sh`, plus `python3 tests/launchservices-folder-handler.py` and `python3 tests/launchservices-extension-query.py`.
  - Expected: no FAIL other than ones recorded as pre-existing (the Wayland "titlebar" window-drag check and the i386 `NSStringDrawingContext` size check are not run by these).
- [ ] **Step 4: Commit** (darling `finder-app`): "tests: DiskImageMounter's remaining flows; roadmap: plan 6 done". Push.
