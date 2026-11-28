# Probe index, batch 2, group B (AppKit menus, documents, windows, buttons)

**macOS result: NOT YET OBSERVED** for every row. Public API only; no Mac was available to the author. Run `sh run-all-B.sh` on a Mac with the stock command line tools (a logged-in GUI session is needed). Windows stay at alpha 0 or are never ordered in; the application is never activated (649 `--manual-menubar` is the one opt-in exception). Opt-in flags: 649 and 650 `--synth` (Quartz key and mouse events posted only to this process; may need the Accessibility permission), 649 `--manual-menubar`, 671 `--warp` (moves the real pointer and restores it).

| dir | PR | GUESS items covered | window server | notes |
|---|---|---|---|---|
| 642-menu-appearance | 642 | 2 | yes | extends batch 1 item-f |
| 643-sharing-picker | 643 | 3 | yes | custom do-nothing service; picker menu cancelled by the probe |
| 649-menu-highlight-delegate | 649 | 3 of 4 (menu bar title only with --manual-menubar; overflow chevron not observable) | yes | |
| 650-popup-menu-delegate | 650 | 3 | yes | selection order needs --synth |
| 651-menu-key-equivalent | 651 | 3 (+2 related) | connection only | |
| 652-alert-sheet-lifetime | 652 | 1 | yes | built without ARC |
| 665-type-for-contents | 665 | 1 | connection only | relaunches in temporary app bundles |
| 667-untitled-document-error | 667 | 1 | connection only | relaunches in temporary app bundles |
| 669-make-document-errors | 669 | 3 | yes | relaunches in a temporary app bundle |
| 670-default-type | 670 | 2 | connection only | 17 bundle variants |
| 671-border-while-inside | 671 | 4 | yes | bitmap comparison |
| 674-window-size-limits | 674 | 2 (+ conflict and clamp questions) | yes | |
| 668-box-clip-browsercell | 668 | 7 | yes | |

## Syntax check on Darling's SDK tree

All 13 parse with `clang -fsyntax-only -x objective-c -target x86_64-apple-macos11 -isysroot <Darling SDK>` plus the CoreServices and ApplicationServices framework directories, after a scratch stub header (kept outside the probe directories) that declares public names the Darling SDK lacks: NSSharingService and NSSharingServicePicker members and delegate protocols, NSMenu cancelTracking / performActionForItemAtIndex: / highlightedItem / popUpMenuPositioningItem:atLocation:inView:, NSDocumentController makeUntitledDocumentOfType:error: and reopenDocumentForURL:..., NSClipView constrainBoundsRect:, CGWindowListCopyWindowInfo and its keys, CGWarpMouseCursorPosition, NSModalResponseStop. 642, 665 and 674 parse without stubs; 651 needed only the removal of the macOS 10.14 protocol name NSMenuItemValidation from the probe.
