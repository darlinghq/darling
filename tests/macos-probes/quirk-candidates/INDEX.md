# Probe index (round 4, slice RQ)

Black-box probes for the 17 patches that round 1 classified QUIRK-NEEDS-MAINTAINER or BLOCKED-NEEDS-VALUES, plus six open questions that belong to other patches (items a to f). Every probe: public API only, standalone, prints `PROBE <id> <input> -> <output>` lines and starts with the macOS version and architecture.

**macOS result: NOT YET OBSERVED** for every row. No Mac was available to the author; nothing here claims a Mac result. The local Darling guest did not run any probe (see the end).

| id | directory | purpose | window server | notes |
|---|---|---|---|---|
| 0012 | `0012-control-metrics/` | Prints cell and intrinsic sizes, title and drawing rectangles, slider and progress defaults, segmented widths and rendered pixels of the standard controls at three control sizes in light and dark. | yes |  |
| 0024 | `0024-bindings-selection-and-errors/` | Prints selectedObjects and selectionIndexes for selections that name rows the content lacks, and records exceptions from bind, model-to-view and view-to-model changes. | yes |  |
| 0033 | `0033-scroller-geometry/` | Prints scroller widths per control size and style, the rectangle of every part for legacy and overlay styles, and sample pixels of a rendered legacy scroller in light and dark. | yes |  |
| 0044 | `0044-alert-layout/` | Builds NSAlert panels in several configurations without showing them and prints window size, title bar, icon, text, accessory, suppression checkbox and button frames and key equivalents. | yes |  |
| 0050 | `0050-tabview-geometry/` | Prints contentRect, minimumSize and intrinsic size per tab view type and control size, where each tab is by hit testing, and a pixel grid of a rendered tab view in light and dark. | yes |  |
| 0056 | `0056-print-callback-and-help/` | Part 1 prints when and how the delegate of runOperationModalForWindow:... is called for a panel-less save-to-PDF operation. Part 2 (with --ui) lists windows that appear for help calls without a help book. | yes | --ui for part 2 |
| 0057 | `0057-view-rects-outside-display/` | Prints what getRectsBeingDrawn:count: and needsToDrawRect: answer inside drawRect: for ten scenarios (no window, image focus, subview, dirty rects, windows never shown, real display passes). | yes |  |
| 0108 | `0108-menu-item-getter-lifetime/` | With manual reference counting, prints whether the object a getter returned is still alive after the item is released but before the autorelease pool drains. | no | built without ARC on purpose |
| 0109 | `0109-imageio-metadata/` | Prints the values behind the public CGImageMetadata and thumbnail names and exercises tag creation, path set/get/remove, metadata read from a JPEG written by the probe, and thumbnail size rules. | no |  |
| 0113 | `0113-toolbar-button-metrics/` | Shows a toolbar window at alpha 0 and prints the window chrome height, the size and place of each item view, item min and max sizes and a pixel row through the toolbar. | yes | --front for a key window |
| 0122 | `0122-clip-edge-coverage/` | Prints the alpha coverage of the pixels around a clip edge moved by tiny offsets, with antialiasing on and off, three clip calls, three CTM scales, and the clip bounding box. | no |  |
| 0201 | `0201-no-window-server-run-loop/` | Prints, step by step, what NSApplication does for nextEventMatchingMask:untilDate:inMode:dequeue: (return value, wall and CPU time, whether a timer and a main-queue block run) in a session with and without a window server. | yes | run twice: normal session and no window server |
| 0215 | `0215-ctfont-copy-memory-font/` | Creates a CGFont from font file bytes (not installed), a CTFont from it, and prints identity, size, matrix and metrics of copies made with size, matrix, feature settings, another name and bold traits. | no | optional font path argument |
| 0350 | `0350-document-app-launch/` | Builds two tiny document apps (with and without NSUbiquitousContainers) and prints, 2 s after launch, the windows and documents that exist for each combination of the user default and the delegate answer, with and without a file argument. | yes | builds two bundles, run.sh |
| 0352 | `0352-file-reference-urls/` | Prints, as booleans, what the file-reference URL family answers for existing and missing files, directories, symlinks, non-file URLs and a moved file. | no |  |
| 0361 | `0361-intersection-rect/` | Prints NSIntersectionRect (both argument orders), NSIntersectsRect, CGRectIntersection and NSIsEmptyRect for 17 rectangle pairs, including largest-double, infinite and NaN edges. | no |  |
| 0009 | `0009-secitem-no-match/` | Prints the status and what is left in the result pointer for read-only queries that match nothing, for a NULL result pointer, for client-certificate enumeration and for malformed queries. | no |  |
| item-a | `item-a-calayer-iosurface-use-count/` | Prints use count, in-use flag and retain count of an IOSurface across assigning it to layers, replacing and clearing contents, rendering, an application-owned use count and layer release. | no |  |
| item-b | `item-b-textfinder-matching/` | Presses find-next repeatedly over a fixed text for each matching type, case option and needle, and prints the selected ranges, for the classic find panel action and the find bar action. | yes |  |
| item-c | `item-c-preferred-font-text-style/` | Prints point size, name, weight, traits and metrics of the font for each text style, plus the system font sizes and related class methods. | yes |  |
| item-d | `item-d-cgpdf-object-type/` | Prints each kCGPDFObjectType enumerator by its public name and the type CGPDFObjectGetType reports for objects of a small PDF the probe writes. | no |  |
| item-e | `item-e-ats-fonts-path-timing/` | Prints, at each launch stage (constructor, start of main, after NSApplication creation, will/did finish launching, first main-queue turn), whether the bundled font is registered in the process scope and where its PostScript name resolves. | yes | builds a bundle, run.sh, needs a font file |
| item-f | `item-f-menu-effective-appearance/` | Prints appearance and effectiveAppearance names for a three-level menu hierarchy and a detached menu across ten settings of menu and application appearances. | yes |  |

## Run everything

```sh
cd probes && sh run-all.sh        # PROBE_UI=1 sh run-all.sh also runs the window-opening parts
```

`run-all.sh` builds each probe with its `build.txt`, runs it (with a time limit), and writes one file `results-<macOS version>.txt` with every `PROBE` line. User name, host name, home directory and temporary directory paths are removed by the script. A probe that fails to build or run produces a `PROBE <dir> BUILD-FAILED` or `RUN-FAILED` line, not silence.

## Items that need extra steps

- 0201: needs a second run where no window server is reachable (ssh to a Mac with nobody logged in, or a launchd daemon). run-all.sh only produces the baseline.
- 0350 and item-e build small app bundles; item-e copies a font file from the Mac (set `PROBE_FONT=/path/to/font.ttf` to choose it).
- 0056 part 2 and 0113 `--front` need `PROBE_UI=1`.

## Items a human maintainer must still decide, whatever the probes print

- 0012, 0033, 0044, 0050, 0113: whether Darling adopts a newer macOS look. The probes give geometry and pixels, not the decision.
- 0056: what a print panel offers and says (user-interface design).
- 0009: approval for any behaviour change in security-sensitive code (AGENTS.md section 3).
- 0024: the nib font-size and font-substitution hunks have no macOS question.
- 0201: the no-window-server run needs a specially prepared Mac.
- Items b, c, e, f and a: the patches behind them were not in this slice; the probes answer the question as phrased. A maintainer must map output to the PR.

## Syntax check on the Darling SDK tree

All 23 probes were syntax-checked on Linux with `clang -fsyntax-only` (`-target x86_64-apple-macos11`, `-isysroot` the Darling SDK named "macOS 11.0 (Darling)", plus the CoreServices and ApplicationServices framework directories for AE.h). 20 parse as they are. 0109, item-b and item-d do not, because the public names they exist to test are missing from Darling's SDK headers (CGImageMetadataType enumerators and metadata constants; NSTextFinder option constants and `-performFindPanelAction:`; CGPDFObjectType and the CGPDF accessors). With a scratch header declaring just those names, all three parse too. Version-guarded code (macOS 11 and later) is skipped by that SDK as it stands; with the guard forced on, 0113, 0009 and item-c parse (with the scratch header), and item-e parses except its macOS 12 call `CTFontManagerCopyRegisteredFontDescriptors`, which Darling's SDK also lacks.

## Darling guest

One attempt to run a built probe (0361, linked against the guest's own Foundation, arm64) with `timeout 120 darling shell` returned "Timed out waiting for shellspawn in container" at once; no more attempts were made. No probe has run on the Darling guest.
