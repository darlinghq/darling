# Probe index, batch 2: GUESS items of the open draft PRs

**macOS result: NOT YET OBSERVED** for every row. Public API only; written from public documentation without reading the PR code, so a probe can also show macOS doing something the PR did not expect. Run `sh run-all.sh` in this directory on a Mac with the stock command line tools, in a logged-in GUI session; it writes `results-batch2-<macOS version>.txt`. Windows stay at alpha 0 or are never ordered in and the app is never activated; opt-in flags are listed in INDEX-B.md (649 and 650 `--synth`, 649 `--manual-menubar`, 671 `--warp`). Group B's details are in INDEX-B.md.

| dir | PR | GUESS items covered |
|---|---|---|
| 147-filewrapper-read-write | darling-foundation #147 | 12 of 12 |
| 2406-webauth-session-request | darling #2406 | 7 of 14 (the rest need a populated request; no public initializer) |
| 638-calayer-zposition-order | #638 | 2 of 2, plus baselines (headless Metal texture) |
| 639-ctfont-copy-traits-family | #639 | 5 of 5 |
| 640-symbol-configuration | #640 | 4 observable items |
| 641-popup-content-bindings | #641 | 5 of 5 |
| 642-menu-appearance | #642 | 2 |
| 643-sharing-picker | #643 | 3 |
| 646-precise-scroll-events | #646 | 3 of 3 |
| 649-menu-highlight-delegate | #649 | 3 of 4 |
| 650-popup-menu-delegate | #650 | 3 |
| 651-menu-key-equivalent | #651 | 3 (+2 related) |
| 652-alert-sheet-lifetime | #652 | 1 |
| 656-tableview-drop-retarget | #656 | 3 of 4 |
| 657-hidden-table-columns | #657 | 4 of 4 |
| 659-tableview-click-beside-columns | #659 | 2 of 2 |
| 660-column-value-transformer | #660 | 2 of 2 |
| 661-column-font-bindings | #661 | 3 of 3 |
| 663-textfield-max-lines | #663 | 3 of 3 |
| 665-type-for-contents | #665 | 1 |
| 666-attrstring-url-at-index | #666 | 3 of 3 |
| 667-untitled-document-error | #667 | 1 |
| 668-box-clip-browsercell | #668 | 7 |
| 669-make-document-errors | #669 | 3 |
| 670-default-type | #670 | 2 (17 bundle variants) |
| 671-border-while-inside | #671 | 4 |
| 674-window-size-limits | #674 | 2 (+ conflict and clamp questions) |
| 677-display-fade-reservation | #677 | 5 of 5 |

Not observable by a black-box macOS probe (see the READMEs): the drop-on outline look (#656), Wayland/X11 protocol items (#646 axis semantics, #647, #653 to #655, #672 to #674 compositor parts), the menu-bar overflow chevron (#649), the placeholder-symbol resize (#640), the CALayerPlane fast path (#638) and whether releasing a fade reservation restores a display (#677).
