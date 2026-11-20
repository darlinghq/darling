# NSCollectionView, dragging sessions and file-URL drags Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give AppKit a modern, layout-driven `NSCollectionView` (data source, item reuse, flow layout, selection by mouse and keyboard, drag source and drop target) and the `NSDraggingSession` API it drags with, so Finder's icon view (plan 7) can be written against the documented API.

**Architecture:** As on macOS 10.14+, the layout classes (`NSCollectionViewLayout`, `NSCollectionViewFlowLayout`, `NSCollectionViewLayoutAttributes`, `NSCollectionViewLayoutInvalidationContext`) live in UIFoundation (`src/private-frameworks/UIFoundation`, darling repo), which AppKit re-exports; `NSCollectionView`, `NSCollectionViewItem` and the dragging-session classes live in AppKit (cocotron). A layout talks to its collection view only through Objective-C messages (no link dependency from UIFoundation on AppKit). `-[NSView beginDraggingSessionWithItems:event:source:]` is built on the existing `NSDraggingManager` and its X11 (XDND) and Wayland backends, so drags to and from the host keep using the existing `DarlingFileURIList` bridge.

**Tech Stack:** Objective-C (manual retain/release in cocotron and UIFoundation), CMake, Darling guest tests (`guest-run.sh`), Xvfb + xdotool and a private Wayland compositor for GUI tests, `ssh lubos@10.10.10.152` (macOS 26) for black-box expected values.

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md`, section "NSCollectionView and drag & drop".

## Global Constraints

- AGENTS.md applies in full: documented contract first; anything else is a `QUIRK:` with tier-2 evidence (a test that passes unchanged on macOS 26) and a test; no app-specific code; no debug leftovers; Provenance note in every commit.
- Layering: layout classes in UIFoundation, view classes in AppKit. UIFoundation never links AppKit; it sends messages to `collectionView` only.
- Legacy (i386) ABI: public instance-variable layouts stay Apple's size. New state goes through `NS_EXTRA_IVARS_BEGIN/END` (`<QuartzCore/LegacyIvarLayout.h>`); `tests/appkit-i386-ivar-layout.m` (NSCollectionView = 272) and `tests/src/uifoundation_collection_layouts.m` (flow layout ivars) must keep passing.
- Expected values the documentation does not pin are first confirmed on the Macs: each new test is written so that it compiles and passes unchanged on both `lubos@10.10.10.152` and `lubos@10.10.10.148` (`clang -framework AppKit`). What was observed goes into `tests/collection-view.md` (darling repo) with each host's macOS version and architecture. Where the two Macs disagree, the test pins only what they share, and the difference is recorded. "Confirm on the Mac" in the tasks below means both hosts.
- Build in `~/Projects/darling/build`, install only into `~/darling-finder`; re-apply setuid root to `~/darling-finder/bin/darling` after `make install`.
- Branches: cocotron `finder-collection-view`, cut from `finder-volumes` (so darling's `finder-app` keeps one linear cocotron history); darling `finder-app`. Push, no PRs.
- Every GUI test runs on a private Xvfb display (never `:99`) and a private Wayland compositor, as the darling-skills `darling-guest-testing` skill describes.

## Review Focus

1. **A folder with thousands of items.** Only items in the visible rectangle are made; scrolling reuses them. A data source asked for 5 000 items must not be asked for all 5 000 views at `reloadData` (Task 4: `test_only_visible_items_are_made`).
2. **Resizing the window while a selection exists.** The flow layout re-flows on a width change and the selection survives it (Task 3: `test_width_change_invalidates`; Task 5: `test_selection_survives_relayout`).
3. **The data source shrinks between reloads.** A selection containing index paths that no longer exist after `reloadData` must not crash or leave stale `selected` items (Task 5: `test_reload_drops_vanished_selection`).
4. **Dragging several selected items.** One `NSPasteboardItem` per dragged item, in index-path order, each readable as a file `NSURL` by the destination (Task 8: `test_multi_item_drag_pasteboard`).
5. **A drop on empty background.** The delegate is offered a "before" drop at the nearest gap, never an "on" drop on an item that is not under the pointer (Task 8: `test_background_drop_proposes_gap`).

---

### Task 1: Layout attributes and invalidation contexts (UIFoundation)

**Files:**
- Modify: `src/private-frameworks/UIFoundation/src/NSCollectionViewLayoutAttributes.m`, `src/private-frameworks/UIFoundation/src/NSCollectionViewLayoutInvalidationContext.m`, their headers in `src/private-frameworks/UIFoundation/include/` (and the AppKit public headers `src/external/cocotron/AppKit/include/AppKit/NSCollectionViewLayout.h`, `NSCollectionViewLayoutAttributes.h`, `NSCollectionViewLayoutInvalidationContext.h` that declare them)
- Test: `tests/src/uifoundation_collection_layout_attributes.m`
- Create: `tests/collection-view.md` (observations)

**Interfaces:**
- Produces: `NSCollectionViewLayoutAttributes` with `frame`, `size`, `alpha`, `zIndex`, `hidden`, `indexPath`, `representedElementCategory`, `representedElementKind`; `+layoutAttributesForItemWithIndexPath:`, `+layoutAttributesForInterItemGapBeforeIndexPath:`, `+layoutAttributesForSupplementaryViewOfKind:withIndexPath:`, `+layoutAttributesForDecorationViewOfKind:withIndexPath:`; `NSCopying`, `-isEqual:`/`-hash`.
- Produces: `NSCollectionViewLayoutInvalidationContext` with `invalidateEverything` (readonly), `invalidateDataSourceCounts` (readonly), `-invalidateItemsAtIndexPaths:`, `invalidatedItemIndexPaths`, `contentOffsetAdjustment`, `contentSizeAdjustment`; `NSCollectionViewFlowLayoutInvalidationContext` with `invalidateFlowLayoutDelegateMetrics`, `invalidateFlowLayoutAttributes` (both default YES on UIKit; confirm).

- [ ] **Step 1: Write the failing test** `tests/src/uifoundation_collection_layout_attributes.m` (same `expect()`/ALL PASSED style as `tests/src/uifoundation_collection_layouts.m`). Checks: `test_item_attributes_defaults` (alpha 1, zIndex 0, hidden NO, category Item, kind nil, frame zero, indexPath as given); `test_size_follows_frame` (setting `size` keeps the origin; setting `frame` changes `size`); `test_gap_attributes_category` (InterItemGap, kind `NSCollectionElementKindInterItemGapIndicator`); `test_supplementary_and_decoration_kinds`; `test_copy_is_equal_and_independent`; `test_equality_by_properties` (two separately made attributes with the same values are equal and hash equally; differing alpha are not); `test_context_defaults` (a fresh context: `invalidateEverything` NO, `invalidateDataSourceCounts` NO, `invalidatedItemIndexPaths` nil, adjustments zero) and `test_context_collects_index_paths`.
- [ ] **Step 2: Confirm on the Mac.** `scp` the file to the Mac, `clang -framework AppKit -framework Foundation`, run. Expected: ALL PASSED. Where it fails, the Mac is right: correct the assertion and record the value in `tests/collection-view.md` ("Layout attributes" section, macOS 26.x).
- [ ] **Step 3: Run on Darling.** `guest-run.sh tests/src/uifoundation_collection_layout_attributes.m`. Expected: FAIL (the classes forward every message today).
- [ ] **Step 4: Implement** the properties and class factories as plain stored state; equality compares every property listed above.
- [ ] **Step 5: Run on Darling.** Expected: ALL PASSED; `tests/src/uifoundation_collection_layouts.m` still ALL PASSED.
- [ ] **Step 6: Commit** (darling, `finder-app`): `UIFoundation: collection view layout attributes and invalidation contexts`.

### Task 2: The NSCollectionViewLayout contract (UIFoundation)

**Files:**
- Modify: `src/private-frameworks/UIFoundation/src/NSCollectionViewLayout.m` and its header; create `src/private-frameworks/UIFoundation/src/NSCollectionViewLayoutPrivate.h`
- Test: `tests/src/uifoundation_collection_layout_contract.m`

**Interfaces:**
- Consumes: Task 1's classes.
- Produces: `collectionView` (readonly, unretained) and the private `-_setCollectionView:(NSCollectionView *)view` that AppKit calls; `+layoutAttributesClass`, `+invalidationContextClass`; `-prepareLayout` (no-op); `-collectionViewContentSize` (NSZeroSize); `-layoutAttributesForElementsInRect:`, `-layoutAttributesForItemAtIndexPath:`, `-layoutAttributesForSupplementaryViewOfKind:atIndexPath:`, `-layoutAttributesForDecorationViewOfKind:atIndexPath:`, `-layoutAttributesForDropTargetAtPoint:`, `-layoutAttributesForInterItemGapBeforeIndexPath:` (base: empty/nil, values per the Mac); `-shouldInvalidateLayoutForBoundsChange:` (NO); `-invalidationContextForBoundsChange:`; `-invalidateLayout` → `-invalidateLayoutWithContext:` with a fresh `[[[self class] invalidationContextClass] new]` marked "everything"; `-invalidateLayoutWithContext:` sends `-_collectionViewLayout:didInvalidateWithContext:` to `collectionView` if it responds (this message name is ours; declared in `NSCollectionViewLayoutPrivate.h`, implemented by AppKit in Task 4); `-registerClass:forDecorationViewOfKind:`, `-registerNib:forDecorationViewOfKind:`.

- [ ] **Step 1: Write the failing test.** A subclass `CountingLayout` overriding `prepareLayout` (counts calls) and a stand-in collection-view object that implements `numberOfSections`/`numberOfItemsInSection:` and `_collectionViewLayout:didInvalidateWithContext:` (records the context). Checks: `test_base_defaults` (content size zero, attributes for a rect empty array or nil as the Mac says, shouldInvalidate NO); `test_invalidate_layout_marks_everything` (the context passed has `invalidateEverything` YES and is of `invalidationContextClass`); `test_custom_context_class` (a subclass returning its own context class gets that class); `test_layout_attributes_class_default`. The same program runs on the Mac: there the stand-in is a real `NSCollectionView`, and the check that the private message arrives runs only when `[NSCollectionView instancesRespondToSelector:@selector(_collectionViewLayout:didInvalidateWithContext:)]`.
- [ ] **Step 2: Confirm on the Mac.** Expected: ALL PASSED after recording any differing defaults in `tests/collection-view.md`.
- [ ] **Step 3: Run on Darling.** Expected: FAIL.
- [ ] **Step 4: Implement.**
- [ ] **Step 5: Run on Darling.** Expected: ALL PASSED (both layout tests).
- [ ] **Step 6: Commit:** `UIFoundation: the collection view layout contract`.

### Task 3: NSCollectionViewFlowLayout (UIFoundation)

**Files:**
- Modify: `src/private-frameworks/UIFoundation/src/NSCollectionViewFlowLayout.m`, `NSCollectionViewFlowLayoutInvalidationContext.m`, header
- Test: `tests/src/uifoundation_flow_layout.m`; update `tests/src/uifoundation_collection_layouts.m` only where the Mac contradicts its frames (ledger a ruling)

**Interfaces:**
- Consumes: Task 2 (`collectionView`, invalidation).
- Produces: `itemSize` (50×50), `minimumLineSpacing` (10), `minimumInteritemSpacing` (10), `sectionInset` (zero), `scrollDirection` (vertical), `estimatedItemSize`, `headerReferenceSize`/`footerReferenceSize` (stored only; headers and footers are not laid out in this plan); `prepareLayout` reads the section and item counts from `collectionView` and the per-item/per-section metrics from `[collectionView delegate]` when it conforms to `NSCollectionViewDelegateFlowLayout` (`sizeForItemAtIndexPath:`, `insetForSectionAtIndex:`, `minimumLineSpacingForSectionAtIndex:`, `minimumInteritemSpacingForSectionAtIndex:`); `collectionViewContentSize`; `layoutAttributesForElementsInRect:` (items intersecting the rect); `layoutAttributesForItemAtIndexPath:`; `layoutAttributesForDropTargetAtPoint:` (an item's attributes when the point is inside an item, otherwise the gap attributes of the nearest insertion point); `layoutAttributesForInterItemGapBeforeIndexPath:`; `shouldInvalidateLayoutForBoundsChange:` YES exactly when the width (vertical) or height (horizontal) changes.

- [ ] **Step 1: Write the failing test.** Uses a real `NSCollectionView` in an off-screen window with a data source and `layoutSubtreeIfNeeded`, then reads both the layout's attributes and the item views' frames. Cases: `test_row_distribution` (200 wide, 80×30 items, 5 items; 300 wide; 260 wide with two items per row and slack — this pins whether rows are packed to the left or the slack is spread between items, which the documentation leaves open); `test_section_insets`; `test_two_sections`; `test_delegate_item_sizes` (varying heights in one row: row height and vertical alignment); `test_horizontal_direction`; `test_content_size`; `test_elements_in_rect` (only intersecting items, any order compared as a set); `test_width_change_invalidates`; `test_drop_target_on_item_and_in_gap`.
- [ ] **Step 2: Confirm on the Mac** and write every observed frame into `tests/collection-view.md` ("Flow layout"). Expected: ALL PASSED once the assertions carry the Mac's frames. If the Mac contradicts `checkFlow` in `uifoundation_collection_layouts.m`, change those frames to the Mac's and ledger `Ruling: corrected unobserved flow frames to macOS's`.
- [ ] **Step 3: Run on Darling.** Expected: FAIL (the frames still come from NSCollectionView's own code).
- [ ] **Step 4: Implement** the flow computation in the layout. Row distribution follows what Step 2 recorded; if it is anything other than the documented "minimum" spacings, it is a `QUIRK:` comment at the distribution helper naming `test_row_distribution`.
- [ ] **Step 5: Run on Darling.** Expected: `uifoundation_flow_layout` ALL PASSED. (`uifoundation_collection_layouts` passes after Task 4, when NSCollectionView stops placing items itself; note that in the ledger.)
- [ ] **Step 6: Commit:** `UIFoundation: NSCollectionViewFlowLayout lays out items`.

### Task 4: NSCollectionView placed by its layout, with item reuse (AppKit)

**Files:**
- Modify: `src/external/cocotron/AppKit/NSCollectionView/NSCollectionView.m`, `NSCollectionViewItem.m`, `AppKit/include/AppKit/NSCollectionView.h`, `NSCollectionViewItem.h`
- Test: `src/external/cocotron/tests/appkit-collection-view-layout.m`; existing `tests/collection-data-source.m`, `tests/appkit-i386-ivar-layout.m`

**Interfaces:**
- Consumes: Tasks 2–3 (`-_setCollectionView:`, `prepareLayout`, attributes queries, `-_collectionViewLayout:didInvalidateWithContext:`).
- Produces: `setCollectionViewLayout:` (calls `-_setCollectionView:` on the old layout with nil and the new one with self); `reloadData` (all items go back to the reuse queues, layout invalidated, `needsLayout`); `-layout` (prepare if invalidated, size self to the content size — and, inside a clip view, to at least the clip view's size if the Mac does so — then make items only for `layoutAttributesForElementsInRect:[self visibleRect]` and apply `frame`, `alpha`, `hidden`, `zIndex` ordering); re-query on clip-view bounds changes; `makeItemWithIdentifier:forIndexPath:` dequeues (calling `-prepareForReuse`) before allocating, and loads a registered nib with `registerNib:forItemWithIdentifier:`; `itemAtIndexPath:`, `visibleItems`, `indexPathsForVisibleItems`, `indexPathForItem:`, `indexPathForItemAtPoint:`, `layoutAttributesForItemAtIndexPath:`, `frameForItemAtIndex:`, `scrollToItemsAtIndexPaths:scrollPosition:`; `backgroundColors` (first color fills the background; default per the Mac) and `backgroundView`/`backgroundViewScrollsWithContent`. New state goes into `NS_EXTRA_IVARS(NSCollectionView)`. The legacy `content`/`itemPrototype` path keeps working.

- [ ] **Step 1: Write the failing test** (`appkit-collection-view-layout.m`, prints PASS/FAIL, runs on the Mac too): `test_items_follow_custom_layout` (a layout subclass returning fixed frames and an alpha; item views get exactly those); `test_prepare_layout_once_per_invalidation` (two `layoutSubtreeIfNeeded` without invalidation → one `prepareLayout`); `test_only_visible_items_are_made` (5 000 items in a 300×300 scroll view: the data source is asked for fewer than 100 items); `test_scroll_reuses_items` (after scrolling to the end, items made ≤ visible count + one row, `prepareForReuse` called); `test_index_path_queries`; `test_document_view_size` (content size vs clip view size); `test_reload_after_count_change`.
- [ ] **Step 2: Confirm on the Mac** (record the visible-item counts, the reuse behavior and the sizing rule in `tests/collection-view.md`). Expected: ALL PASSED.
- [ ] **Step 3: Run on Darling.** Expected: FAIL.
- [ ] **Step 4: Implement,** deleting `_layoutLoadedItems` (placement now comes from the layout).
- [ ] **Step 5: Run on Darling:** the new test, `collection-data-source`, `appkit-i386-ivar-layout` (i386 and x86_64), `tests/src/uifoundation_collection_layouts.m`, `appkit-collection-view-compositional-layout`. Expected: all pass.
- [ ] **Step 6: Commit** (cocotron `finder-collection-view`): `AppKit: NSCollectionView places items from its layout and reuses them`; then bump cocotron in darling `finder-app`.

### Task 5: Selection model (AppKit)

**Files:**
- Modify: `NSCollectionView.m`, `NSCollectionViewItem.m`, headers
- Test: `src/external/cocotron/tests/appkit-collection-view-selection.m`

**Interfaces:**
- Consumes: Task 4 (visible items).
- Produces: `selectable` (default NO), `allowsMultipleSelection` (NO), `allowsEmptySelection` (YES), `selectionIndexPaths` (KVO-compliant), `-selectItemsAtIndexPaths:scrollPosition:`, `-deselectItemsAtIndexPaths:`, `-selectAll:`, `-deselectAll:`; item `selected` and `highlightState` kept in sync on visible items and set on items made later; legacy `selectionIndexes` mapped onto section 0. Private helpers used by Task 6: `-_userSelectIndexPaths:(NSSet *)paths byExtending:(BOOL)extend` (runs the delegate's should/did pair) and `-_userDeselectIndexPaths:`.

- [ ] **Step 1: Write the failing test:** `test_selection_defaults`; `test_programmatic_selection_skips_delegate` (documented); `test_items_reflect_selection` (visible and later-made items); `test_select_all_respects_multiple` ; `test_kvo_notifies_selection`; `test_selection_survives_relayout`; `test_reload_drops_vanished_selection` (what the Mac keeps after a shrinking reload is recorded and matched); `test_legacy_selection_indexes`.
- [ ] **Step 2: Confirm on the Mac;** record. Expected: ALL PASSED.
- [ ] **Step 3: Run on Darling.** Expected: FAIL.
- [ ] **Step 4: Implement.**
- [ ] **Step 5: Run on Darling.** Expected: ALL PASSED, plus Task 4's tests.
- [ ] **Step 6: Commit:** `AppKit: NSCollectionView selection`.

### Task 6: Selecting with the mouse and the keyboard (AppKit)

**Files:**
- Modify: `NSCollectionView.m` (mouseDown:, mouseDragged:, mouseUp:, keyDown:, moveUp:/moveDown:/moveLeft:/moveRight: and their …AndModifySelection: forms, drawing of the rubber band)
- Test: `src/external/cocotron/tests/appkit-collection-view-mouse.m` (events built with `+[NSEvent mouseEventWithType:…]`/`keyEventWithType:…` and sent through `-[NSWindow sendEvent:]`, so it runs headless on the Mac and in Darling)

**Interfaces:**
- Consumes: Task 5 helpers; Task 3 frames for geometry.
- Produces: the user-selection behaviors and the delegate call order: `shouldSelectItemsAtIndexPaths:` (its returned set is what gets selected), `didSelectItemsAtIndexPaths:`, `shouldDeselectItemsAtIndexPaths:`, `didDeselectItemsAtIndexPaths:`, `shouldChangeItemsAtIndexPaths:toHighlightState:`, `didChangeItemsAtIndexPaths:toHighlightState:`.

- [ ] **Step 1: Write the failing test,** each case recording the delegate calls in order: `test_click_selects_only_that_item`; `test_command_click_toggles`; `test_shift_click_extends_in_index_order`; `test_click_background_deselects` (and keeps the selection when `allowsEmptySelection` is NO); `test_delegate_filters_selection`; `test_rubber_band_selects_intersecting` (and ⌘-rubber band toggles); `test_arrow_keys_move_by_geometry` (left/right previous/next; up/down to the item above/below); `test_shift_arrow_extends`; `test_not_selectable_ignores_clicks`; `test_double_click_reaches_item_view` (an item-view subclass gets `mouseDown:` with `clickCount` 2 and the item is already selected).
- [ ] **Step 2: Confirm on the Mac** (the delegate call order and the up/down rule go into `tests/collection-view.md`). Expected: ALL PASSED.
- [ ] **Step 3: Run on Darling.** Expected: FAIL.
- [ ] **Step 4: Implement.**
- [ ] **Step 5: Run on Darling.** Expected: ALL PASSED; Tasks 4–5 tests pass.
- [ ] **Step 6: Commit:** `AppKit: select collection view items with the mouse and the keyboard`.

### Task 7: Dragging sessions (AppKit)

**Files:**
- Modify: `AppKit/NSDraggingItem.m`, `AppKit/include/AppKit/NSDraggingItem.h`, `NSDraggingManager.{h,m}`, `X11.backend/X11DraggingManager.m`, `Wayland.backend/WaylandDraggingManager.m`, `NSView.m`, the NSDraggingInfo implementations (`X11.backend/X11DropSession.m`, `Wayland.backend/WaylandDropSession.m`)
- Create: `AppKit/NSDraggingSession.m`, `AppKit/include/AppKit/NSDraggingSession.h` (and add it to `AppKit.h`)
- Test: `src/external/cocotron/tests/appkit-dragging-session.m` (object model, runs on the Mac) and `src/external/cocotron/tests/appkit-dragging-session-live.m` driven by `appkit-dragging-session-x11.sh` / `-wayland.sh` (xdotool / the compositor's pointer, in the style of `appkit-window-drag-x11.sh`)

**Interfaces:**
- Produces: `NSDraggingItem` (`initWithPasteboardWriter:`, `item`, `draggingFrame`, `imageComponentsProvider`, `imageComponents`, `-setDraggingFrame:contents:`); `NSDraggingImageComponent` (`key`, `contents`, `frame`, `+draggingImageComponentWithKey:`); `NSDraggingSession` (`draggingFormation`, `animatesToStartingPositionsOnCancelOrFail`, `draggingLeaderIndex`, `draggingPasteboard`, `draggingSequenceNumber`, `draggingLocation`, `-enumerateDraggingItemsWithOptions:forView:classes:searchOptions:usingBlock:`); `-[NSView beginDraggingSessionWithItems:event:source:]` — writes each item's writer to the drag pasteboard as its own `NSPasteboardItem`, composes one drag image from the items' frames and contents, returns the session at once and starts the backend drag when the current event handler returns; source callbacks `draggingSession:sourceOperationMaskForDraggingContext:` (Within/OutsideApplication), `willBeginAtPoint:`, `movedToPoint:`, `endedAtPoint:operation:`; destination `NSDraggingInfo`: `draggingSourceOperationMask` narrowed by the modifier keys as the NSDraggingInfo documentation describes (Option → copy, Command → generic, Control → link), `numberOfValidItemsForDrop`, `draggingSequenceNumber`, `-enumerateDraggingItemsWithOptions:…`.

- [ ] **Step 1: Write the failing tests:** object model — `test_dragging_item_frame_and_components`, `test_begin_returns_session_with_pasteboard_items` (n writers → n pasteboard items, in order); X11/Wayland — `test_source_callbacks_order` (willBegin, moved…, ended with the operation the target accepted), `test_modifier_narrows_operation` (Option held while dropping on a target offering copy|move → copy), `test_drop_reads_file_urls`.
- [ ] **Step 2: Confirm the object-model test on the Mac.** Expected: ALL PASSED. (The live drags cannot be driven headless there; their expectations come from the documentation.)
- [ ] **Step 3: Run on Darling (both displays).** Expected: FAIL.
- [ ] **Step 4: Implement** on top of `NSDraggingManager`; the legacy `dragImage:at:offset:event:pasteboard:source:slideBack:` keeps its behavior.
- [ ] **Step 5: Run on Darling** plus the existing drag tests (`appkit-window-drag-x11.sh`, `-wayland.sh`, the pasteboard tests). Expected: all pass.
- [ ] **Step 6: Commit:** `AppKit: dragging sessions`.

### Task 8: NSCollectionView as drag source and drop target (AppKit + flow layout)

**Files:**
- Modify: `NSCollectionView.m` (dragging source/destination), header (`NSCollectionViewDropOperation`, delegate methods)
- Test: `src/external/cocotron/tests/appkit-collection-view-drag.m` (+ `-x11.sh`, `-wayland.sh`)

**Interfaces:**
- Consumes: Task 7 sessions; Task 3 `layoutAttributesForDropTargetAtPoint:`; Task 5/6 selection.
- Produces: source — a drag past the hysteresis that starts on a selected item asks `collectionView:canDragItemsAtIndexPaths:withEvent:`, then `pasteboardWriterForItemAtIndexPath:` per item (falling back to `writeItemsAtIndexPaths:toPasteboard:`), begins one session with one dragging item per index path (each framed at its item), calls `draggingSession:willBeginAtPoint:forItemsAtIndexPaths:` and `draggingSession:endedAtPoint:dragOperation:`; `-setDraggingSourceOperationMask:forLocal:` (defaults per the Mac). Destination — `registerForDraggedTypes:`; `validateDrop:proposedIndexPath:dropOperation:` with in/out index path and operation (`NSCollectionViewDropOn` over an item, `NSCollectionViewDropBefore` at the nearest gap otherwise); `acceptDrop:indexPath:dropOperation:`; drop-target highlight on the item and the inter-item gap indicator, both removed when the drag exits or ends.

- [ ] **Step 1: Write the failing tests:** `test_multi_item_drag_pasteboard` (two selected file items dragged to a second window's collection view: two pasteboard items, read back as two file NSURLs in index-path order); `test_can_drag_refusal_cancels`; `test_drop_on_item`; `test_background_drop_proposes_gap`; `test_validate_can_retarget` (the delegate changes the proposed index path and operation; `acceptDrop` receives the changed values); `test_drop_from_host` (a `text/uri-list` drag from an X11 client: `xdotool` + a small `xdnd` source script, or the existing host-drag fixture if one exists).
- [ ] **Step 2: Run on Darling (both displays).** Expected: FAIL. (Default source masks are confirmed on the Mac with a non-interactive check: `-draggingSourceOperationMaskForLocal:` on a fresh view, added to `appkit-collection-view-selection.m`'s Mac run.)
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run on Darling.** Expected: all pass; Tasks 4–7 tests pass.
- [ ] **Step 5: Commit:** `AppKit: drag and drop in NSCollectionView`.

### Task 9: File URLs on the pasteboard (AppKit)

**Files:**
- Test: `src/external/cocotron/tests/appkit-pasteboard-file-urls.m`
- Modify (only where the test finds a gap): `AppKit/NSPasteboard.m`, `AppKit/NSPasteboardSupport.m`

**Interfaces:**
- Consumes: the existing `writeObjects:` / `readObjectsForClasses:options:`.
- Produces: pinned behavior: writing n file NSURLs → n items of `public.file-url`; `NSFilenamesPboardType` is readable as the array of their paths, and writing that type is readable back as file NSURLs; `NSPasteboardURLReadingFileURLsOnlyKey` drops non-file URLs; `NSPasteboardURLReadingContentsConformToTypesKey` filters by UTI; `canReadObjectForClasses:options:` agrees with `readObjectsForClasses:options:`.

- [ ] **Step 1: Write the test** with the cases above, on a uniquely named pasteboard.
- [ ] **Step 2: Confirm on the Mac.** Expected: ALL PASSED.
- [ ] **Step 3: Run on Darling.** Expected: pass or fail; for each failing case, fix it (RED first, it already is) and say in the commit which cases were new.
- [ ] **Step 4: Run on Darling** together with `appkit-pasteboard-api`, `appkit-pasteboard-over-cfpasteboard`. Expected: all pass.
- [ ] **Step 5: Commit:** `AppKit: file URLs on the pasteboard` (or `tests: …` if nothing needed fixing).

### Task 10: Whole-feature run and roadmap

**Files:**
- Modify: `docs/superpowers/plans/2026-11-16-finder-roadmap.md` (plan 5 row: repos `cocotron, darling (UIFoundation)`, status done), `tests/collection-view.md`

- [ ] **Step 1:** `make install` into `~/darling-finder`, re-apply setuid root, restart the guests, and run every test from Tasks 1–9 in the rootless, non-root and root containers (`all-modes.sh`), both displays where the test is a GUI test. Expected: all pass.
- [ ] **Step 2:** Review the full diffs of both repos for leftover probes (AGENTS.md §6).
- [ ] **Step 3: Commit** the roadmap and observations; push cocotron `finder-collection-view` and darling `finder-app`.
