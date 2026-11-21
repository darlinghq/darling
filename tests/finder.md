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
  items, switching to the list view and back (the selection stays), showing another folder (the view stays), the
  history (back, forward, the enclosing folder with the folder it came from selected), opening the selection, a
  folder of 2000 items (scrolling to the last one shows it), a window whose folder is removed closing, going to a
  folder that can't be read (the window stays, with an alert sheet; a history entry whose folder has gone is
  dropped), and nothing in a window pointing at its controller once that has gone. It passes unchanged on both Macs.
- `tests/src/finder_reveal.m` (needs a display): `openURLs:` with files in two folders, a file that doesn't exist
  and a folder gives a window per folder with its files selected, and the folder's own window with nothing
  selected; revealing an item that appeared after its folder's window opened selects it there. It passes unchanged on
  both Macs.
- `tests/src/finder_list_view.m` (needs a display): the list view. Its columns (Name, Date Modified, Size, Kind), what
  their cells show (sizes as `NSByteCountFormatter` gives them, `--` for a folder; dates in the medium date style and
  short time style; the kind as `NSURLLocalizedTypeDescriptionKey` gives it), sorting by size both ways and by name,
  expanding a folder in place, and selecting items in an expanded folder. It passes unchanged on both Macs.
- `tests/src/finder_sidebar.m <disk image>` (needs a display): the sidebar. Favorites are Applications, the home
  folder (titled with the user name), and Desktop, Documents and Downloads when they exist; Locations has the root
  volume (no eject button), and an attached image's volume appears with an eject button and goes when it is
  detached. The window's folder is the sidebar's selection (setting it shows nothing; a folder not in the sidebar
  selects nothing), and selecting an item shows it. It attaches and detaches the image itself. It passes unchanged on both Macs. In the guest, pass the
  image's guest path: `ARGS=/Volumes/SystemRoot$PWD/tests/fixtures/unmount-test.dmg`.
- `tests/src/finder_dsstore.m <fixture dir> [<store>...]`: the `.DS_Store` reader. It uses the fixtures in
  `tests/fixtures/dsstore/`, which `tests/fixtures/make-dsstore-fixtures.py` writes with our own writer
  (`tests/fixtures/dsstore.py`, whose docstring describes the format and its public sources):
  - `all-types`: a record of each value type, with names that need UTF-16 surrogates and normalization; its values
    are compared;
  - `multi-level`: 3000 records in a two-level tree;
  - `empty`: none;
  - `cycle`, `block-out-of-range`, `bad-type`, `huge-count`, `long-name` and `bad-magic`: malformed, and rejected.

  Every truncation and 2000 mutations of the well-formed stores give nil or a store, without crashing. Extra stores
  given (copies Finder wrote) must parse, and their records are printed. It passes unchanged on both Macs, where the
  fixture directory has to be copied first.
- `tests/src/finder_folder_style.m <fixture dir> [<Finder-made store>]` (AppKit, no window): a folder's view
  settings from its `.DS_Store`:
  - `styled`: the window size, chrome, view, icon and text sizes, label position, positions (a decomposed name found
    by the precomposed one) and picture;
  - `styled-bookmark`: the picture as bookmark data;
  - `colour` and `legacy`: older `icvo`/`BKGD` records;
  - `list`;
  - pictures outside the volume (`..`, a symbolic link), missing or a folder: no picture, the rest kept;
  - `wrong-types`: the defaults;
  - no store, an empty, malformed or `no-settings` one: no style.

  Each case copies the fixture into a folder of a temporary "volume". A store Finder wrote for a styled folder (icon
  96, items at (100, 150) and (400, 150), 500×300, no toolbar) can be given too. It passes unchanged on both Macs.

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
| eject | with a window of F open, Command-E in the volume's window ejects the volume; its window closes, F's stays |
| eject button | the same with the sidebar's eject button (X11 only: a headless sway has no pointer to click with) |
| detached outside | `hdiutil detach` in the guest closes the volume's window |
| reveal | `open -R F/one.txt` gives a window on F |
| reveal several | `open -R` of files in F and G gives windows on both |
| open -R missing | `open -R` with a file that doesn't exist exits 1 with "The file … does not exist." (as on macOS) and reveals nothing |
| open file | a stand-in editor registered for `.fndtest`; `open -R F/doc.fndtest`, then Command-Down, and the editor gets `odoc` for it |
| odd names | a folder named `Ünï 日本 😀` opens in its window; `open -R` of a file in it whose name has a newline, and of one with a 255-byte name, gives that window |
| large folder | a folder of 2000 files shows its window within 5 s of `open` returning; End in it is fine |
| folder removed | with F open too, deleting F2 (from the host side) closes F2's window within 10 s; F's stays |
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
- Window titles are compared as canonically equivalent Unicode. Foundation decomposes file paths, on macOS as
  here (`fileURLWithPath:`, `fileSystemRepresentation` and bookmarks all give the decomposed form on 10.14.4 and
  26.7.3), so a folder whose name is stored precomposed gets a window titled with the decomposed form.
- A window watches its folder with an FSEvents stream (`kFSEventStreamCreateFlagWatchRoot`); on any event, if the
  folder is gone, the window closes. Darling's FSEvents (inotify) reports the folder's own removal as an event on
  it, and sees changes made from the host.
- `scrollToItemsAtIndexPaths:scrollPosition:` with `NSCollectionViewScrollPositionNone` does not scroll on macOS;
  AppKit scrolled. Fixed (`appkit-collection-view-scroll-position-none.m`); Finder uses the nearest edge.
- Ejecting from the volume's own window closes that window from within its own event handling. AppKit used to
  validate the window's toolbar half a second later anyway, and to update closed windows in `updateWindows`; by
  then the toolbar item's target (the window's controller) was gone, and Finder crashed. AppKit now validates a
  toolbar only while its window is shown (`appkit-toolbar-closed-window-validation.m`).
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
