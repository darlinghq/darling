# Finder.app

Finder (`src/Finder`, installed as `/System/Library/CoreServices/Finder.app`) shows folders and volumes in windows.
LaunchServices hands it folders and volumes: it declares Finder's "Folder" (`fold`) and "Volume" (`disk`) document
types.

## Running the tests

In-process tests, in the guest (`build/tmp/helpers/src-test.sh <mode> <file>`; each makes its own directory):
- `tests/src/finder_directory.m`: the folder model. Visible items (no names starting with `.`, nothing with
  `NSURLIsHiddenKey`), their order, and the errors for a missing or unreadable folder. It passes unchanged on macOS
  10.14.4 and 26.7.3; the order it checks is the one macOS gives.
- `tests/src/finder_window.m` (needs a display): a window's title and size, its icon view's items and labels, selecting
  items, switching to the list view and back (the selection stays), and showing another folder (the view stays). It
  passes unchanged on both Macs.
- `tests/src/finder_list_view.m` (needs a display): the list view. Its columns (Name, Date Modified, Size, Kind), what
  their cells show (sizes as `NSByteCountFormatter` gives them, `--` for a folder; dates in the medium date style and
  short time style; the kind as `NSURLLocalizedTypeDescriptionKey` gives it), sorting by size both ways and by name,
  expanding a folder in place, and selecting items in an expanded folder. It passes unchanged on both Macs.

The harness, `tests/finder.sh`, runs Finder through LaunchServices (`open` in the guest) on X11 or Wayland, like
`tests/disk-image-mounter.sh` (same environment; see the header of the script):

| case | checks |
|---|---|
| open folder | `open <folder>` gives a window titled with the folder's name |
| open twice | opening it again leaves one window for it |
| views | Command-2 shows the list (the window's image changes), Command-1 the icons again; the window stays |
| quit | Command-W on the last window ends Finder |
| launch | `open -a Finder` (no documents) gives a window on the home folder |
| volume | opening a disk image gives a window on its volume (DiskImageMounter, then LaunchServices) |
| unreadable | a folder that can't be read gives an alert and no window; after the alert Finder quits (skipped when the guest runs as root) |

## Notes

- An application LaunchServices launches runs without `HOME` in Darling. `NSHomeDirectory()` then comes from the user
  database, as macOS does it (macOS takes it from there even when `HOME` is set). So in rootless mode, where the guest
  runs as uid 0, Finder's home folder is `/var/root`.
- `NSURLIsHiddenKey` can't be set in Darling: `chflags` has no `UF_HIDDEN` to store, and Foundation's
  `setResourceValue:forKey:` says it succeeded anyway. `finder_directory` prints SKIP for that case. On macOS the
  flag is stored and the item is hidden.
- The tests that need a stand-in for Finder keep working where Finder is installed:
  - `tests/disk-image-mounter.sh` declares the stand-in's types at `LSHandlerRank` `Owner`;
  - `tests/src/launchservices_folder_handler.m` expects the installed Finder, with the stand-in competing with it.

- The list view needed three fixes below Finder, each with its own test:
  - CoreFoundation's `NSURLLocalizedTypeDescriptionKey` (`URLLocalizedTypeDescription.m`);
  - AppKit's `outlineView:sortDescriptorsDidChange:` (`appkit-outline-view-sort-descriptors.m`);
  - AppKit's `columnsInRect:`, which left out the last column when the first was wide, so it wasn't drawn
    (`appkit-table-view-columns-in-rect.m`).
- The kind strings are LaunchServices' type descriptions ("plain text", where macOS says "Plain Text Document").

## Observed on macOS

Finder's `Info.plist` on 26.7.3:
- `CFBundleIdentifier` `com.apple.finder`, `CFBundlePackageType` `FNDR`, `NSPrincipalClass` `TApplication`, no
  `LSUIElement`;
- document types "Folder" (`CFBundleTypeOSTypes` `fold`) and "Volume" (`disk`), both role Editor, with no
  `LSItemContentTypes`.

Ours differs in two ways:
- the package type is `APPL`, because LaunchServices counts only `APPL` bundles as applications;
- the principal class is `NSApplication`.
