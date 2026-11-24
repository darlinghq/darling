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
- `tests/src/finder_styled_window.m <fixture dir>` (needs a display): a window on a folder with `styled.DS_Store`:
  - 640×400 content with no toolbar or sidebar;
  - items centred where the store puts them, and one without a position below them;
  - icons 128, text 13;
  - the View menu's Show/Hide Toolbar and Sidebar;
  - a folder without a style back in the flow layout, and the stored layout again on going back;
  - hostile positions kept inside the content;
  - window sizes out of range clamped (300×200 up to the screen's visible frame; unreadable ones give the standard
    size), and icon sizes clamped to 16 and 512.

  It passes unchanged on both Macs.

File operations (plan 7c):
- `tests/src/finder_file_operation.m <new directory> [<disk image>]` (no display): the copy and move engine. Copies
  of files and trees (symbolic links copied as links), moves, the conflict choices (Replace, Keep Both, Stop, Apply to
  All), names that differ only in normalization, "copy" names in the item's own folder, transfers refused up front (a
  folder into itself, a Replace of the source's own ancestor), failures and cancelling (nothing left at the final
  name, no staged item), and the default kind across volumes. It passes unchanged on both Macs.
- `tests/src/finder_operation_ui.m` (needs a display): the operation queue. No panel for a quick copy; a long one's
  progress panel (text, a rising indicator, Stop leaving nothing behind); the conflict alert (text, buttons, "Apply
  to All" for several items); alerts for a refused and a failed transfer. It passes unchanged on both Macs.
- `tests/src/finder_item_actions.m <new directory> [<disk image>]`: new folders ("untitled folder", "untitled folder
  2"), the rules for a new name (not empty, no `:` or `/`, no leading `.`, not another item's, also in another
  normalization; a case-only change is allowed), the extension question (asked only when both names have an
  extension and they differ), renaming, and Trash through `NSWorkspace`. It passes unchanged on both Macs except Trash
  on a read-only image, checked in Darling only (over ssh, macOS 26.7.3's `recycleURLs:` never completes for it and
  10.14.4's aborts without a window server).
- `tests/src/finder_commands.m <new directory> [<disk image>]` (needs a display): a window's commands. Copy, Paste and
  Move Item Here through the general pasteboard; Paste disabled without file URLs; New Folder, which starts renaming;
  renaming in the icon and the list view (commit, refusal with an alert, the extension question, cancelling); Move to
  Trash; what is enabled with nothing selected and in a read-only folder. It passes unchanged on 26.7.3; on 10.14.4,
  where the ssh session can't write any pasteboard, the copy and paste cases are skipped.
- `tests/src/finder_get_info.m` (needs a display): the Get Info panel. Title, name, kind, size ("<formatted> (<n>
  bytes)"), where and modified for a file; "Calculating…", then "<formatted> for <n> items" for a folder; one panel
  per item; closing releases it. It passes unchanged on both Macs.
- `tests/src/finder_drop.m <new directory> [<disk image>]` (needs a display): drops in the icon view. Which items take
  drops (folders, symbolic links to folders), the operation (Option copies, Command moves, otherwise a move within a
  volume and a copy across; nothing onto a read-only volume, into a dragged folder, or a move onto the items' own
  folder), where a drop on another item or a gap goes (the window's folder), and the dragged item's file URL. The
  dragging system isn't driven: a stand-in `NSDraggingInfo` holds the pasteboard. It passes on 26.7.3; 10.14.4 skips
  the pasteboard cases.
- `tests/src/finder_list_drop.m <new directory> [<disk image>]` (needs a display): drops in the list view, through
  the outline view's dragging-destination methods at points over its rows (a folder's row, a file's row, below the
  rows, between an expanded folder's children; text refused), the dragged row's file URL, and the sidebar's data
  source (a favourite takes drops; a section title, a gap or a read-only volume doesn't). It passes on 26.7.3;
  10.14.4 skips it (no pasteboard).

AppKit fixes these needed, each with a test in `src/external/cocotron/tests` that passes unchanged on both Macs:
`NSCell` editable/selectable coupling and editable new table columns (`appkit-cell-editable-selectable.m`,
`appkit-tableview-edit-column.m`); a collection view drop retargeted to a nil index path isn't performed
(`appkit-collection-view-drag.m`); table and outline drag and drop with pasteboard writers
(`appkit-tableview-drag.m`, `appkit-outlineview-drag.m`); a label passing a press up the responder chain
(`appkit-textfield-label-mouse-down.m`).

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
| styled window | `open styled-test.dmg`: a window "Styled Test" within 60 s, its client area 640 wide and 400 to 440 high; the background picture shows (#2F6F9F within 8 per channel 20 points in from the bottom left; light at (320, 200), where the picture has a band) |
| styled hidden item | `tests/src/finder_ls.m` on the attached volume lists `Styled App.app` and `Applications`, and not the invisible `Hidden Notes.txt`, `.background` or `.DS_Store` |
| unreadable | a folder that can't be read gives an alert and no window; after the alert Finder quits (skipped when the guest runs as root) |
| option-drag copies within a volume | Option-dragging `a` onto the folder `F` in the same window copies it (`a` stays); a plain drag of `b.txt` onto `F` moves it |
| drop on a folder link | Option-dragging a bundle (files and a symbolic link inside) onto `Install`, a symbolic link to `dest2`, copies it there (`diff -r` matches, the source stays) |
| install from a disk image | dragging `Styled App.app` from the styled window of `styled-test.dmg` into a window of `dest` copies it (another volume; `diff -r` matches in the guest); the image then detaches |
| progress | Option-dragging a 4 GiB file (written past the page cache, so the copy reads the disk) onto a folder shows a window titled Copy within 3 s; its Stop closes it and leaves no file. Skipped if the copy is over before the window's 1 s delay |
| paste | Command-A and Command-C in one window, Command-V in another, copies the item |
| new folder and rename | Shift-Command-N, typing `Neu` and Return make the folder `Neu` |
| trash | Command-A and Command-Delete move an item to `~/.Trash` (the item is in a folder the guest makes on the home folder's volume) |

The drag cases place the windows (`xdotool windowmove`, `swaymsg move position`) and find an 800-point window's icons
by the order of their names: the first icon row is 452 points up from the window's bottom edge, item *i* 263 + 115·*i*
points from its left edge (measured up from the bottom, as Wayland windows have the menu bar on top). On Wayland they
need `VIRTUAL_POINTER` (see the header of the script); Option is held with `wtype -M alt` for the whole drag.

## Notes

- An application LaunchServices launches runs without `HOME` in Darling. `NSHomeDirectory()` then comes from the user
  database, as macOS does it (macOS takes it from there even when `HOME` is set). So in rootless mode, where the guest
  runs as uid 0, Finder's home folder is `/var/root`.
- `NSURLIsHiddenKey` is YES for a name starting with ".", for `UF_HIDDEN`, and for the Finder's invisible flag
  (`kIsInvisible`, byte 8 = 0x40 of `com.apple.FinderInfo`), as on macOS 10.14.4 and 26.7.3; setting it stores
  both, and Darling's file systems keep the FinderInfo one (`chflags` has no `UF_HIDDEN` there)
  (`src/external/corefoundation/tests/URLIsHidden.m`). Foundation's directory listings used to compute the hidden
  keys themselves and store them through their setters, which cleared the flags on disk
  (`src/external/foundation/tests/filemanager-contents-hidden-keys.m`).
- A disk image's extended attributes reach the guest: darling-dmg answers the kernel with Linux error numbers, and
  names its attributes as Darling's kernel layer keeps guest attributes (`user.<name>`); an item without FinderInfo
  has none (`tests/src/dmg_xattr.m`, which passes unchanged on both Macs).
- Styled windows (`.DS_Store`):
  - the format is not documented by Apple. Reader and fixture writer follow public third-party descriptions: Wim
    Lewis's DSStoreFormat notes (in the Mac::Finder::DSStore distribution), Mark Mentovai's description, and the
    documentation of the Python `ds_store` and `mac_alias` packages. No code was copied;
  - a malformed store is ignored as a whole; a setting of an unexpected type, or a number that isn't finite, keeps
    its default;
  - records match items by canonical equivalence of the names, case-sensitively;
  - window size and chrome apply when a window opens on the folder; the view, sizes, label position, background
    and positions whenever it is shown. A folder without a usable store gets the standard look;
  - window content 300×200 up to the screen's visible frame; icons 16 to 512 points (64); text 10 to 16 (12);
  - a position is the item's centre, top-left origin; items without one go in a grid below the others;
  - the background picture is drawn at its natural size, top left, and must be a regular file inside the folder's
    own volume (alias tag 18 or 2, or a bookmark's path), or it isn't used. A picture over 8192 pixels either way
    (read from its header through ImageIO, which now gives a PNG's `PixelWidth` and `PixelHeight`, as on both Macs:
    `imageio-png-properties.m`), or a file that isn't a picture, isn't used either; the colour is, if one is given;
  - items placed to the right of the visible width can't be scrolled to yet. With a layout of its own, macOS
    26.7.3's `NSCollectionView` is as wide as the content; 10.14.4's, like ours, keeps the scroll view's width;
  - older `icvo` and `BKGD` records are read when there is no `icvp`; `vstl` `Nlsv` is the list view;
  - View > Show/Hide Toolbar (⌥⌘T) and Show/Hide Sidebar (⌃⌘S). On macOS, `NSWindow` neither retitles a
    `toggleToolbarShown:` item nor enables it without a toolbar; the application titles it
    (`appkit-window-toggle-toolbar-title.m`). A hidden toolbar takes no room: the window's frame is that of a window
    without one, and the content keeps its size, on both Macs; AppKit kept the toolbar's height
    (`appkit-toolbar-hidden-frame.m`).
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
- Open: Move to Trash fails, with an alert, for an item on `/Volumes/SystemRoot` (the host's file system), which has no
  Trash in Darling; macOS would offer to delete the item immediately on a volume without one. Items on the home
  folder's volume go to `~/.Trash`.
- Conflict and error alerts of copies and moves are app-modal, not sheets on the window the operation came from: the
  engine waits for the answer (plan 7c, Task 2).

## Observed on macOS

Finder's `Info.plist` on 26.7.3:
- `CFBundleIdentifier` `com.apple.finder`, `CFBundlePackageType` `FNDR`, `NSPrincipalClass` `TApplication`, no
  `LSUIElement`;
- document types "Folder" (`CFBundleTypeOSTypes` `fold`) and "Volume" (`disk`), both role Editor, with no
  `LSItemContentTypes`.

`.DS_Store` files Finder wrote on 26.7.3 parse (`finder_dsstore.m`): `/Applications/.DS_Store` holds only
`lg1S`, `ph1S` (`comp`) and `modD`, `moDD` (`blob`, not `dutc`) for `Xcode.app`. Driving macOS Finder with
`osascript` from ssh (a styled folder written by Finder, and reading back how it shows `styled-test.dmg`) needs the
Automation permission on the Mac; until it is given, those checks are not made. Open: they are what would show
whether `Iloc` centres, the item frames and the picture reference are read as Finder writes them (the reader and
the fixture writer share one reading of the public descriptions).

Ours differs in two ways:
- the package type is `APPL`, because LaunchServices counts only `APPL` bundles as applications;
- the principal class is `NSApplication`.
