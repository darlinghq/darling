# Finder.app for Darling — design

Date: 2026-11-16
Status: draft for approval

## Intent

Darling should ship a small Finder of its own, the way it already ships
TextEdit. The headline use case: a Linux user double-clicks a `.dmg` on their
Linux desktop, and ends up looking at the disk image's window in Finder —
including the "styled" window many apps ship (a background picture with an
arrow, the app icon on one side and an `Applications` alias on the other),
from which they drag the app into `/Applications`. Beyond that, Finder offers
trivial file management.

A second, equally important intent: every piece of missing platform
functionality this needs gets implemented **in Darling's frameworks and
system components**, at the layer where macOS has it, so that other
applications benefit. Finder is the driver for that work, not a place to
hide workarounds. Where macOS splits responsibilities between components
(e.g. Finder does not mount disk images; DiskImageMounter does), we split
them the same way.

## Goals

1. **Open a disk image from the Linux desktop.** Double-clicking a `.dmg` in a
   Linux file manager mounts it inside Darling and opens its volume window in
   Finder.
2. **Licence agreements.** A disk image carrying an embedded software licence
   agreement is mounted only after the user accepts it, as on macOS.
3. **Styled disk-image windows.** Show a volume's window the way its
   `.DS_Store` describes it: background image, icon size, icon positions,
   window size, toolbar/sidebar hidden when the image says so.
4. **Install by drag and drop.** Dragging an item from a disk-image window
   onto the `Applications` alias (or into an `/Applications` window) copies it
   there, with progress and error reporting.
5. **Eject.** A disk image's volume can be ejected from Finder (menu item and
   sidebar eject button); its windows close.
6. **Trivial file management.** Browse folders in icon view and list view;
   open files with their default app (via LaunchServices); new folder; rename;
   copy/move by drag and drop and by copy/paste; move to Trash; Get Info
   (basic: name, kind, size, dates, path); a sidebar with Applications, home,
   Desktop, Documents, Downloads and mounted volumes.
7. **Be the system's "Finder" for other apps.** `NSWorkspace` APIs that on
   macOS hand off to Finder (`selectFile:inFileViewerRootedAtPath:`,
   `activateFileViewerSelectingURLs:`) open a Finder window with the items
   selected.

## Non-goals

- **No desktop.** Finder does not draw the desktop, desktop icons or the
  Trash on the desktop, and is not a permanently running process. It is
  launched on demand (by LaunchServices when a folder/volume is opened, by
  another app, or by the user) and quits when its last window closes.
- No column view, gallery view, Quick Look, tags, search/Spotlight, AirDrop,
  network browsing, burning, file sharing, compression, or Finder sync
  extensions.
- No AppleScript dictionary for Finder (DMG *creation* scripts drive Finder
  via AppleScript; consuming DMGs does not need it). Finder only handles the
  standard open-documents and reveal Apple Events.
- No writing of `.DS_Store` files; view settings of ordinary folders are not
  persisted in this version.
- No recreation of Apple's private DiskImages framework API. The attach
  library introduced here is Darling's own private library with its own
  interface.
- Mounting formats beyond what `darling-dmg` already reads.

## Anti-goals (failure even if it "works")

- Any app-specific logic: no checks on bundle IDs, app names, DMG names or
  specific background images (AGENTS.md §1).
- Re-implementing lower-layer functionality privately in a higher layer —
  e.g. Finder or AppKit parsing the mount table, anything above
  `libsystem_kernel` calling `fusermount`, DiskArbitration invoking `hdiutil`
  (AGENTS.md §2).
- Finder mounting disk images itself.
- Hand-rolling a private icon grid in Finder instead of completing
  `NSCollectionView`.
- Mounting an image with a licence agreement without the user's acceptance.
- Leftover debugging code (AGENTS.md §6).
- Anything derived from Apple's Finder, DiskImageMounter, DiskImages or other
  Apple binaries, or from third-party code without a compatible license
  (AGENTS.md §4, §7).

## Constraints

- **Language:** Finder and DiskImageMounter are Objective-C only. Framework
  and system work is in the language of the component it lands in
  (AppKit/Foundation: Objective-C; DiskArbitration/CoreServices: C/ObjC as
  they are; `libsystem_kernel`: C; darling-dmg: C++; launcher: C).
- **AGENTS.md** governs everything, in particular: layering (§2), quirks with
  `QUIRK:` comments and tests (§3), no verbatim copying (§4), tests derived
  from the documented contract (§8), provenance notes in PRs (§7).
- **Repositories:** AppKit lives in the `cocotron` submodule, Foundation in
  `foundation`, `hdiutil`/FUSE in `darling-dmg`, `libsystem_kernel` in `xnu`
  — each change there is a PR to that submodule's repo plus a submodule bump
  in the main repo.
- **Bundle identities and locations (as on macOS):**
  - Finder: `com.apple.finder`, `/System/Library/CoreServices/Finder.app`.
  - DiskImageMounter: `com.apple.DiskImageMounter`,
    `/System/Library/CoreServices/DiskImageMounter.app`.
  Both directories are already scanned by launchservicesd. Applications locate
  Finder by bundle ID (e.g. to reveal files), so the IDs are required for
  compatibility. All artwork is our own.
- **All three launcher modes** (root, rootless, non-root) must support mount,
  eject and the end-to-end flow.

## Approaches considered

### Who opens a `.dmg`

1. **Finder runs `hdiutil attach` itself.** Public interface, one app — but
   real Finder never mounts images, so this puts the work in the wrong
   component. Initially chosen, then **rejected** once it was clear macOS
   routes disk images through DiskImageMounter.
2. **DiskImageMounter.app + a shared attach library — chosen.** As on macOS:
   LaunchServices hands `.dmg` files to DiskImageMounter, which (after any
   licence agreement) attaches through the same library `hdiutil` uses, then
   asks LaunchServices to open the mounted volume, which lands in Finder.
   Finder stays a pure file viewer. Any app opening a `.dmg` through
   `NSWorkspace` gets the same behaviour.

### How icon views are drawn

- **`NSCollectionView` — chosen**, because it means implementing missing
  AppKit functionality other apps use. Finder's grid uses
  `NSCollectionViewFlowLayout`; the styled disk-image window uses a
  Finder-defined `NSCollectionViewLayout` subclass that places items at the
  `.DS_Store` positions (ordinary app code using the public subclassing
  contract).
- A custom free-placement view inside Finder: less framework work, but
  effort no other app benefits from. Rejected.

### How DiskArbitration is built

- **A `diskarbitrationd` launchd daemon plus the client library — chosen.**
  The documented contract (cross-process appeared/disappeared callbacks,
  approval callbacks able to dissent from another process's unmount/eject,
  stable disk identities) needs one shared arbiter.
- A per-process library over `getfsstat`: smaller, but cross-process
  notifications and dissenters would silently not work. Rejected.

### How unmounting works

- **Real `unmount(2)` in `libsystem_kernel` — chosen.** Today it is a stub
  returning `EBUSY`. It performs a Linux unmount; when that is not permitted
  for a FUSE mount (non-root mode, possibly rootless), it falls back to the
  host's `fusermount -u` through the ELF bridge. That host interaction then
  lives at the lowest layer, and `hdiutil detach`, the attach library,
  DiskArbitration and any other caller just use `unmount()`.
- A dedicated helper program called by both `hdiutil` and DiskArbitration:
  adds a component without gaining anything. Rejected.

## Architecture

```
 Linux desktop (.desktop, MimeType=application/x-apple-diskimage)
        │  darling <generic "open host file"> <host path>   (launcher, C)
        ▼
 /usr/bin/open → LaunchServices
        │  .dmg (com.apple.disk-image-udif)
        ▼
 DiskImageMounter.app (ObjC)
   ├─ licence agreement? → show, require Agree
   ├─ attach via libDarlingDiskImages (private, shared with hdiutil)
   └─ open mounted volume URL via NSWorkspace/LaunchServices
        ▼
 Finder.app (com.apple.finder, ObjC)
   ├─ windows: sidebar, toolbar, icon view (NSCollectionView), list view (NSOutlineView)
   ├─ styled window: .DS_Store reader + free-form NSCollectionViewLayout subclass
   └─ file ops via NSFileManager / NSWorkspace; eject via NSWorkspace
        ▼
 AppKit      NSWorkspace (icons, volumes, eject, recycle, reveal, mount notifications),
             NSCollectionView, drag & drop, pasteboard file URLs
 Foundation  volume enumeration, trash, NSURL volume resource keys
 DiskArbitration (client lib)  ⇄  diskarbitrationd (launchd job)
        ▼
 libsystem: getfsstat / statfs / unmount(2)      darling-dmg FUSE (attach library)
```

### Disk images: attach library, hdiutil, DiskImageMounter

- **Attach library.** darling-dmg's attach/detach logic moves out of the
  `hdiutil` executable into a private dylib (name is the builder's call;
  "libDarlingDiskImages" used here). Interface (indicative): open an image;
  report whether it has a licence agreement and return its text(s) per
  language (plain text and/or RTF); attach with options (mount root /
  mount point, read-only, …) returning the mount point(s) and disk name(s);
  detach by mount point or disk name. It is C (or C-callable) so that the C++
  `hdiutil` and the ObjC DiskImageMounter can both use it.
- **Licence agreements.** Read from the image's embedded resources (the
  `LPic`/`STR#`/`TEXT`/`RTF ` resources in the UDIF resource data, per public
  third-party descriptions of the format; paraphrased, no copied code).
  Choose the language matching the user's preferred localisation, else the
  image's default. Only presentation differs between front ends:
  - `hdiutil attach` follows its documented macOS behaviour (shows the
    agreement and asks on the terminal; honours the documented options that
    pre-accept or suppress it in scripted use). Where the exact non-interactive
    behaviour is undocumented, it is established by black-box observation and
    recorded as a `QUIRK:` (AGENTS.md §3) — not guessed.
  - DiskImageMounter shows a window with the agreement, a language popup if
    several are present, and Agree / Disagree. Disagree → nothing is mounted.
- **Volume naming.** The mount point under `/Volumes` is the HFS+ volume
  name (as on macOS), de-duplicated (`Name 1`, …), not the file name.
- **Disk identity.** The FUSE filesystem is mounted with a distinctive
  subtype that identifies it as a darling-dmg disk image (we control it).
  `diskarbitrationd` recognises such mounts and assigns each a unique
  `diskN` (`diskNsM` for the volume). `hdiutil attach -plist` / `-puppetstrings`
  report these names instead of the hard-coded `/dev/disk1` by asking
  DiskArbitration (downward dependency: DiskImages-level code using
  DiskArbitration is the macOS direction).
- **Attach options:** DiskImageMounter attaches *without* `-nobrowse`
  semantics, so the volume is visible to Finder and other apps.
- **After attaching**, DiskImageMounter opens the volume's root through
  `NSWorkspace`/LaunchServices, which opens a Finder window on it, then
  quits. DiskImageMounter has no Dock-visible windows except the licence
  agreement and error alerts.
- **Images stay mounted** when Finder or DiskImageMounter quit, until
  ejected (as on macOS).

### Unmount and DiskArbitration

- **`unmount(2)`** (xnu `libsystem_kernel` emulation, currently a stub):
  translate the guest path to the host path, try the Linux unmount honouring
  `MNT_FORCE`; on a permission failure for a FUSE mount, run the host's
  `fusermount -u` via the ELF bridge. Return correct errnos (`EBUSY`,
  `EPERM`, `EINVAL`, `ENOENT`). Tests cover all three launcher modes.
- **`diskarbitrationd`:** a launchd daemon that keeps the disk/volume model,
  built from the mount table (`getfsstat`) and refreshed on change (a
  mount-table change notification or an explicit nudge from the client
  library after mount/unmount; mechanism is the builder's call, must not
  depend on higher layers). It serves the DiskArbitration client library over
  IPC (Mach/XPC per what Darling already uses for daemons), assigns `diskN`
  names, runs approval callbacks (dissenters) for unmount/eject across
  processes, and delivers appeared/disappeared/description-changed callbacks
  to registered sessions.
- **DiskArbitration client library:** replaces the current dummy:
  `DASessionCreate`, scheduling on run loops and dispatch queues,
  `DADiskCreateFromBSDName`, `DADiskCreateFromVolumePath`,
  `DADiskCopyDescription` (volume name/path/kind/UUID, media
  ejectable/removable/writable/whole/leaf, device model "Disk Image" etc.),
  `DADiskUnmount`, `DADiskEject`, appeared/disappeared/approval callback
  registration, `DADissenter` — with documented completion semantics.
  Unmount is performed via `unmount(2)`; ejecting a disk-image disk detaches
  it (stops the FUSE process) via the lower layer, not via `hdiutil`.
- **CoreServices** `FSEjectVolumeSync`/`FSUnmountVolumeSync` currently only
  know the boot volume. Out of scope here beyond not contradicting DA; noted
  as follow-up.

### NSWorkspace / Foundation

- Volume enumeration: `NSFileManager -mountedVolumeURLsIncludingResourceValuesForKeys:options:`
  (`getfsstat`), NSURL volume resource keys (`NSURLVolumeNameKey`,
  `NSURLVolumeIsEjectableKey`, `NSURLVolumeIsRemovableKey`,
  `NSURLVolumeIsReadOnlyKey`, `NSURLVolumeIsLocalKey`,
  `NSURLVolumeLocalizedFormatDescriptionKey`, …) from `statfs` and the
  DiskArbitration description. `NSWorkspace -mountedLocalVolumePaths` /
  `-mountedRemovableMedia` / `-getFileSystemInfoForPath:…` are built on these.
  (Foundation using DiskArbitration for "ejectable" must be checked against
  layering — if Foundation must not depend on DA, the key comes from what
  `statfs` and the mount flags expose; builder to confirm and ask if unclear.)
- Eject: `-unmountAndEjectDeviceAtURL:error:` and
  `-unmountAndEjectDeviceAtPath:` → DiskArbitration unmount then eject;
  errors mapped to documented `NSError`s.
- Notifications: `NSWorkspaceDidMountNotification`,
  `NSWorkspaceWillUnmountNotification`, `NSWorkspaceDidUnmountNotification`
  on the workspace notification center, driven by DiskArbitration callbacks,
  so they are system-wide.
- `iconForFile:` for any file (falls back to the file's type icon; folders,
  volumes, disk images included), `iconForFiles:`.
- `recycleURLs:completionHandler:` and `NSFileManager
  -trashItemAtURL:resultingItemURL:error:` (to `~/.Trash`, collision-safe).
- `selectFile:inFileViewerRootedAtPath:` and
  `activateFileViewerSelectingURLs:` send the file viewer the reveal event
  macOS 26 sends (see "Reveal requests" below), via LaunchServices
  (launching it by bundle ID if needed).

### NSCollectionView and drag & drop

- Modern `NSCollectionView` API: `dataSource`, `delegate`,
  `registerClass:/registerNib:forItemWithIdentifier:`,
  `makeItemWithIdentifier:forIndexPath:`, `reloadData`, sections, item views
  placed from the `collectionViewLayout`; `NSCollectionViewLayout` subclassing
  contract (`prepareLayout`, `collectionViewContentSize`,
  `layoutAttributesForElementsInRect:`,
  `layoutAttributesForItemAtIndexPath:`, invalidation);
  `NSCollectionViewFlowLayout` (item size, spacing, section insets);
  selection (click, ⌘/⇧-click, rubber band, arrow keys,
  `selectionIndexPaths`, should/did select/deselect); `backgroundView`,
  `backgroundColors`; double-click handling; drag source
  (`pasteboardWriterForItemAtIndexPath:`, dragging session) and drop target
  (`validateDrop:…`/`acceptDrop:…`, drop on item and drop between/background);
  scrolling in `NSScrollView`.
- Pasteboard/drag & drop: file URLs (`NSPasteboardTypeFileURL`,
  `NSFilenamesPboardType`, `readObjectsForClasses:@[NSURL]` with file-URL
  options), multi-item dragging sessions, `NSDragOperation` negotiation with
  modifier keys, drags to/from the host desktop through the existing
  `DarlingFileURIList` bridge where it already supports that.
- Anything else found missing while building (NSPathControl, NSSplitView
  sidebar, toolbar, NSOutlineView details) is fixed in AppKit under the same
  rules, not worked around in the apps.

### `.DS_Store` reading (in Finder)

`.DS_Store` is Finder's private file, so its reader lives in Finder.app.
Apple never documented it; it is implemented from public third-party format
descriptions (e.g. Wim Lewis's DSStoreFormat notes, Mark Mentovai's
description, the documentation of the MIT-licensed Python
`ds_store`/`mac_alias` packages) — paraphrased, written independently, no
copied code (AGENTS.md §3/§4). It handles:

- the buddy-allocator container and the record B-tree;
- for the folder itself (`.` record): `bwsp` (window bounds, toolbar/sidebar
  visibility), `icvp` (icon size, text size, label position, background type,
  colour and image alias), `vstl`/`vSrn` (view style), and the older
  `icvo`/`BKGD` records where needed;
- per item: `Iloc` (icon centre position, flipped coordinates);
- the background image alias (Alias Manager record or bookmark data),
  resolved using only the volume-relative path inside it and the mounted
  volume. No app- or DMG-specific fallbacks.

Hostile input: DMGs are untrusted. Malformed/unknown data is ignored and the
window falls back to a plain icon view; the reader must never crash or read
out of bounds. Unit tests use synthetic files built by the tests from the
format description, including malformed and truncated input.

### Styled-window behaviour

- Content size from `bwsp`; background image drawn at its natural size,
  top-left; items centred at their `Iloc` points; icon size from `icvp`.
- Hidden items not shown: dot-files (`.background`, `.DS_Store`,
  `.fseventsd`, `.Trashes`, …) and items with the invisible Finder flag if
  `com.apple.FinderInfo` is exposed.
- Drop on a folder or folder symlink (e.g. `Applications` → `/Applications`)
  copies into its target. Copy by default across volumes, move within a
  volume, ⌥ forces copy. Conflicts ask Replace / Keep Both / Stop. Copies
  longer than ~1 s show a progress window.

### Linux desktop integration

- A generic launcher feature to open a **host** file inside Darling: maps the
  host path into the guest (`/Volumes/SystemRoot/...`, as the launcher
  already does for program paths) and runs `open` on it. Exact CLI shape is
  the builder's call; it must not be Finder- or DMG-specific.
- A `.desktop` file declaring `MimeType=application/x-apple-diskimage;` that
  uses it, plus a shared-mime-info entry for `*.dmg` (installed so it does not
  conflict with a desktop that already defines the type).
- Packaging: install from CMake; add to the Debian/RPM file lists of the GUI
  package.

## Apps: structure (indicative)

- `src/Finder/` and `src/DiskImageMounter/` in the main repository (new,
  in-tree like `src/diskutil`), built with `add_darling_executable`,
  installed under `libexec/darling/System/Library/CoreServices/`, mirroring
  the TextEdit CMake pattern (Info.plist, PkgInfo, `.icns` from iconsets).
- Finder `Info.plist`: `CFBundleDocumentTypes` for folders and volumes
  (viewer role). Not `LSUIElement`.
- DiskImageMounter `Info.plist`: `CFBundleDocumentTypes` for disk images
  (`com.apple.disk-image-udif` and the related disk-image UTIs, viewer role);
  it also exports `com.apple.disk-image-udif` (`UTExportedTypeDeclarations`:
  conforms to `com.apple.disk-image` and `public.disk-image`; extensions
  `dmg`, `udif`, `img`; MIME type `application/x-apple-diskimage`), as
  macOS's DiskImageMounter does (observed with `lsregister -dump` on macOS
  26.7.3). Until then a `.dmg` is a file of unknown type, and
  `nsworkspace-icon-kinds.m` skips its `.dmg` checks;
  `LSUIElement` so it has no Dock presence.
- Finder classes (names are the builder's call): app delegate (open
  documents, reveal requests, quit after last window), window controller,
  sidebar, icon-view controller, list-view controller, free-form layout,
  `.DS_Store` reader, file-operation queue with progress window, Get Info
  panel.
- UI in code or nibs — builder's call — but nibs must be buildable with the
  tools available in the Darling build.

## Testing

- Every framework/system change has tests in the owning project's suite
  (cocotron `tests/`, foundation tests, `darling/tests/` for
  DiskArbitration, `unmount(2)`, launcher), derived from the documented
  contract, with negative cases.
- `unmount(2)` and DiskArbitration eject tested in all three launcher modes.
- Licence-agreement extraction tested with synthetic images built by the test
  (or a small project-made, license-clean fixture).
- `.DS_Store` reader: synthetic fixtures incl. malformed input.
- End-to-end: a project-made DMG (created by the test or a small checked-in
  fixture) with a background image, `Iloc` positions, an `Applications`
  symlink and a licence agreement: open from the host side → agreement →
  mount → Finder window layout → copy into a temporary destination → eject.
  GUI checks follow the darling-skills X11/Wayland test guides.

## Reveal requests (observed on macOS 26)

What NSWorkspace and `open` send the file viewer, observed on macOS 26 with a
logger app registered as `com.apple.finder` (black-box, tier 2):

| Call | Event the file viewer gets |
|------|----------------------------|
| `selectFile:inFileViewerRootedAtPath:` | `aevt`/`odoc`, a list of 1 item |
| `activateFileViewerSelectingURLs:` | one `aevt`/`odoc` with all the items in order, even from different folders |
| `open -R <file>` | `aevt`/`odoc`, a list of 1 item |
| `selectFile:nil inFileViewerRootedAtPath:<dir>` | `aevt`/`rapp`, no items |

- Every `odoc` Finder receives is a reveal: Finder shows each item's folder and
  selects the item (items in one folder are selected together). A reveal and a
  plain open (`open -a Finder <item>`, `LSOpenFromURLSpec`) are the same event:
  `aevt`/`odoc`, a list of bookmark-data items, `'aapd'` = true, and the same
  `esrc`/`inte` attributes, for files and folders alike (observed on macOS
  26.7.3 with an `NSFileViewer` logger; see `tests/volumes.md`). LaunchServices
  adds `'aapd'` to every `odoc` and `rapp` it delivers, so it carries no
  meaning for Finder.
- Each item is bookmark data (`typeBookmarkData`, `'bmrk'`), not a path or an
  alias; Finder resolves it with `URLByResolvingBookmarkData:` (or the CFURL
  call).
- The root path argument of `selectFile:inFileViewerRootedAtPath:` is never
  sent.
- A file viewer that isn't running is launched by bundle ID, and the first
  event arrived about 70 ms after it finished launching, so Finder installs
  its Apple Event handlers in `applicationWillFinishLaunching:`.
- The bundle ID is `com.apple.finder`; the `NSFileViewer` default can name
  another one.
- macOS 26 sent no `misc`/`mvis` or Finder-specific event. An AppleScript
  dictionary would only matter for `tell application "Finder" to reveal`.
- Darling: NSWorkspace sends these same events. Only when no file viewer app
  is installed in the prefix does it fall back to asking the Linux file
  manager to show the item.

## Decisions made by the designer (for review)

- Attach without `-nobrowse` semantics, so mounted images are visible.
- Reveal hand-off uses the event macOS 26 was observed to send (below), not
  the older "make objects visible" event, which macOS 26 never sent.
- Images stay mounted after Finder/DiskImageMounter quit.
- New apps live in-tree (`src/Finder`, `src/DiskImageMounter`).
- darling-dmg FUSE mounts carry a distinctive subtype that
  `diskarbitrationd` uses to recognise disk images.
- `.DS_Store` files are read, never written.

## Left to the builder

- Class names, file layout, nib vs code UI.
- Name and exact interface of the attach library.
- Exact launcher CLI for opening host files (generic, documented).
- diskarbitrationd's change-detection and IPC mechanism, within the layering
  rules.
- Order of work, provided each change lands tested and self-contained in the
  right repository.
- Default view settings for ordinary folders.
- Original icon artwork for Finder, DiskImageMounter and generic volume /
  disk-image icons.
