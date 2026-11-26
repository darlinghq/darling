# NSCollectionView: behaviour observed on macOS

Black-box observations (AGENTS.md §3, tier 2) behind the collection view tests. Each test program compiles and passes unchanged
on both hosts:

| Host | macOS | Architecture |
|------|-------|--------------|
| A | 26.7.3 | x86_64 |
| B | 10.14.4 | x86_64 (binary built on A with `-mmacosx-version-min=10.14`) |

Where the two hosts differ, the tests pin only what they share, and the difference is listed here.

## Layout attributes and invalidation contexts

`tests/src/uifoundation_collection_layout_attributes.m`, same on both hosts:

- Item attributes start with alpha 1, zIndex 0, not hidden, a zero frame, category Item and no element kind.
- Setting `size` keeps the frame's center: a frame of {10, 20, 30, 40} given the size 50×60 becomes {0, 10, 50, 60}.
- Gap attributes have the category InterItemGap and the kind `NSCollectionElementKindInterItemGapIndicator`.
- A copy is equal to its original. Two attributes made separately with the same values are equal and hash equally, and
  they differ when alpha or frame differ.
- A fresh invalidation context reports nothing invalidated, no invalidated items (nil) and no adjustments. A fresh flow
  layout context has `invalidateFlowLayoutDelegateMetrics` and `invalidateFlowLayoutAttributes` both YES.

## The layout contract

`tests/src/uifoundation_collection_layout_contract.m`, same on both hosts:

- The base layout has no collection view and a zero content size. It answers nil for every attributes query and for a
  drop target, and an empty or nil list for the elements in a rect. A bounds change doesn't invalidate it. Its target
  content offset is the proposed one.
- `layoutAttributesClass` is NSCollectionViewLayoutAttributes and `invalidationContextClass` is
  NSCollectionViewLayoutInvalidationContext; the flow layout's is NSCollectionViewFlowLayoutInvalidationContext.
- `invalidateLayout` calls `invalidateLayoutWithContext:` once, with a new context of `invalidationContextClass` whose
  `invalidateEverything` and `invalidateDataSourceCounts` are both NO, attached to a collection view or not. A
  collection view's `reloadData` invalidates its layout with a context that has both YES.
- `invalidationContextForBoundsChange:` makes a context of `invalidationContextClass` that does not invalidate
  everything.
- Setting a collection view's layout sets the layout's `collectionView`; the layout it replaces has none afterwards.
  AppKit logs that a layout which doesn't override `collectionViewContentSize` may show no items (not checked).

## Flow layout

`tests/src/uifoundation_flow_layout.m`, same on both hosts. A collection view of the given width is the document view
of a scroll view in a window; frames are {x, y, width, height}.

- 80×30 items, default spacings (10 and 10), no insets:
  - 200 wide, 5 items: {0, 0}, {120, 0}, {0, 40}, {120, 40}, {0, 80}; content 200×110.
  - 300 wide, 5 items: {0, 0}, {110, 0}, {220, 0}, {0, 40}, {110, 40}; content 300×70.
  - 250 wide, 5 items: {0, 0}, {170, 0}, {0, 40}, {170, 40}, {0, 80}; content 250×110.
  - 300 wide, 2 items: {0, 0}, {110, 0}; content 300×30. 4 items: the fourth at {0, 40}.
  - 120 wide (one column), sections of 3 and 1: every item at x 20; y 0, 40, 80, then 110; content 120×140.
  - Items 250 wide in a 200 wide view: x -25; content 200×70. The flow layout logs that its behaviour is undefined.
  - Interitem spacing 20 and line spacing 5, 300 wide: {0, 0}, {110, 0}, {220, 0}, {0, 35}, {110, 35}; content 300×65.
  - No items: content 200×0.
- Default 50×50 items, 200 wide, 4 items: {0, 0}, {75, 0}, {150, 0}, {0, 60}; content 200×110.
- So when every item has one size, the items sit on columns: as many as fit with the minimum spacing, the first at the
  left, the last at the right, equal gaps between; every row, the last too, uses those columns, and one column is
  centered. The flow delegate giving every item the same size behaves the same (60×30 in 250: x 0, 95, 190).
- Insets 5, 10, 5, 10 (top, left, bottom, right), 200 wide: {10, 5}, {110, 5}, {10, 45}; content 200×80. Two sections of
  3 and 2 without insets: section 1 starts at y 70 (no line spacing between sections); with the insets at y 85,
  content 200×120. Sections of 3 and 1, 300 wide: the second section's item at {0, 30}; content 300×60.
- Sizes from the flow delegate (other sizes than one):
  - Heights 30, 50, 20, 40 (80 wide), 200 wide: {0, 10}, {120, 0}, {0, 70}, {90, 60}: a row is as high as its highest
    item, the others centered in it; the last row is packed at the left with the minimum spacing. Content 200×100.
  - Widths 60, 100, 40 in 250 (one row, the last): x 0, 70, 180.
  - Widths 150, 150, 60, 60 in 250: x 50 (alone in its row: centered), then 0 and 190, then 0 (last row).
  - Widths 100, 60, 100, 100, 60 in 250: x 0 and 190, 0 and 150, then 0.
- Horizontal direction, 80×30 items, 100 high: {0, 0}, {0, 70}, {90, 0}, {90, 70}, {180, 0}; content 260×100.
- The elements in {0, 35, 250, 10} of the 250 wide 5-item layout are items 2 and 3; there are none in the column gap.
- Bounds changes (vertical flow, view 250×110): a new width or a new x origin invalidates; scrolling (a new y origin)
  and a new height don't. On host A the first question asked of a new layout answers YES whatever the rect; the test
  asks a question whose answer is YES first.
- Drop targets in the 250 wide 5-item layout: on an item, that item's attributes (category Item). Between two items of
  a row, the gap before the second, spanning the space between them ({80, 0, 90, 30}). Between rows, the gap before the
  first item of the next row (its frame is zero there).
- Gap attributes: before an item that starts a row, one interitem spacing left of it ({-10, 0, 10, 30}); before any
  other item, the space from the previous one ({80, 0, 90, 30}); after the last item, one spacing right of it
  ({80, 80, 10, 30}).

## Collection view placement and reuse

`appkit-collection-view-layout.m` and `collection-data-source.m` (darling-cocotron's tests), same on both hosts:

- `reloadData` asks the data source for no item; the next layout pass does. It invalidates the layout once, with a
  context whose `invalidateEverything` and `invalidateDataSourceCounts` are YES.
- Item views are subviews of the collection view, at their layout attributes' frames and alpha.
- With 5 000 80×30 items in a 300×300 scroll view, only the visible items are asked for (fewer than 100). Scrolling to
  the end makes at most a row more items than are visible; the others are reused, and `prepareForReuse` is called.
- After a reload with fewer items, the views of items no longer shown stay subviews of the collection view but are
  hidden.
- Layout passes without an invalidation don't prepare the layout again; an invalidation prepares it once more.
- In a clip view the collection view is at least as large as the clip view. A vertical layout (any layout that isn't a
  horizontal flow) keeps the clip view's width even when its content is wider (content 400×300 in a 300×200 clip view:
  300×300) and grows to the content's height; a horizontal flow grows in width instead (5 items 80×30 in 200×100:
  260×100). With no content at all, host A sizes the view to the clip view and host B leaves its frame alone (not
  checked).
- The default `backgroundColors` is one color, the control background color.
- A plain NSCollectionViewItem without a nib raises when its view is loaded on host B (it looks for a nib named after
  the class); the tests give their item classes a `loadView`.

## Selection

`appkit-collection-view-selection.m` (darling-cocotron), same on both hosts:

- Defaults: not selectable, no multiple selection, empty selection allowed, `selectionIndexPaths` an empty set and
  `selectionIndexes` an empty index set.
- `selectItemsAtIndexPaths:scrollPosition:` and `deselectItemsAtIndexPaths:` change the selection without asking or
  telling the delegate (as documented), also when the view isn't selectable or several paths are selected without
  multiple selection, and without a key-value notification for `selectionIndexPaths`; setting `selectionIndexPaths`
  notifies its observers.
- Visible items show the selection (`selected`), items made later for selected paths are selected and reused ones for
  other paths aren't; programmatic selection leaves `highlightState` at none.
- `selectAll:` selects nothing without multiple selection and every item with it; `deselectAll:` empties the selection.
- A relayout keeps the selection. Any `reloadData`, with fewer items or the same ones, empties it.
- `selectionIndexes` are section 0's index paths, both ways.

## Selecting with the mouse and the keyboard

`appkit-collection-view-mouse.m` (darling-cocotron): events sent to an ordered-in window; three columns of 80×30 items.
Same on both hosts, except where noted. The highlight states are 1 (for selection), 2 (for deselection) and 0 (none).

- A click on an item, nothing selected: shouldHighlight/didHighlight item → 1, shouldSelect, didSelect,
  shouldHighlight/didHighlight item → 0.
- A click on another item with multiple selection: the new item → 1; the old ones → 2, shouldDeselect, didDeselect,
  → 0; then shouldSelect and didSelect the new one, and it goes back to 0.
- Without multiple selection (⌘-click too): the new item → 1, shouldSelect, didSelect, → 0, then shouldDeselect,
  didDeselect, → 0 of the old one.
- ⌘-click on an unselected item adds it (→ 1, should/didSelect, → 0); on a selected one removes it (→ 2,
  should/didDeselect, → 0). Shift-click adds just the clicked item, not the items between.
- A click on the background deselects everything (shouldDeselect, didDeselect, → 0; no highlight before); when empty
  selection isn't allowed, nothing happens.
- When shouldSelect returns an empty set: the item → 1, shouldSelect, → 0, nothing selected and no didSelect.
- A rubber band highlights the items as it reaches them (→ 1; the delegate is also called with empty sets while it
  moves) and ends with shouldSelect of the covered items, didSelect of what the delegate allowed, and → 0 of the covered
  items. A ⌘-band that starts by reaching a selected item deselects the covered ones (→ 2, should/didDeselect of all
  covered, → 0). Without multiple selection a band only deselects (the background click) and selects nothing.
- A second click (click count 2) on a selected item reaches the item's view with the item selected, and calls no
  delegate method.
- Arrow keys: shouldDeselect and didDeselect of the old item, then shouldSelect and didSelect of the new one, no
  highlight calls. Left and right move within a row (right at a row's end does nothing); up and down to the item above
  or below. Shift-arrows select the range in index order from the item first selected to the new one (shift-right then
  shift-down from 4 adds 5, then 6–8; shift-up then removes 6–8). With nothing selected, arrows do nothing.
- Hosts differ at the last row: from item 8 (row 2, column 2), down selects item 9 (row 3, column 0) on host A and
  does nothing on host B; the test pins neither.
- The first click a process sends to its window activates it and its mouse up arrives late; the test starts with a
  click that isn't part of any case.

## Dragging sessions

Pinned by `appkit-dragging-session.m` (object model, both hosts) and `appkit-dragging-session-live.m` (a drag between
two windows of one process, driven by `appkit-dragging-session-{x11,wayland}.sh`).

- A new `NSDraggingItem` has a zero dragging frame. `setDraggingFrame:contents:` installs an image components provider
  that returns one component keyed `NSDraggingImageComponentIconKey`, framed at (0, 0, width, height) of the dragging
  frame. A new `NSDraggingImageComponent` has its key, nil contents and a zero frame.
- `beginDraggingSessionWithItems:event:source:` returns a session whose formation is `NSDraggingFormationNone`, which
  animates to its destination, and whose sequence number is nonzero. The drag pasteboard holds one item per dragging
  item, in order. An item with a zero-size dragging frame raises `NSRangeException`.
- A session's dragging location and leader index before the drag starts differ between runs and hosts; nothing pins
  them.
- Enumerating a session's dragging items with a class filter visits only the items readable as that
  class: with items URL, string, URL, `NSURL` visits items 0 and 2 and `NSString` item 1.
- `NSURL` writes only its URL type (`public.file-url` for a file URL, `public.url` otherwise) to a pasteboard item; no
  plain-text form.

Not observed on the hosts (the live drag needs synthesized input there); `appkit-dragging-session-live.m` checks these
from the documented contract on Darling only:

- The destination sees the source's operation mask narrowed by the modifier held (Option: Copy). With no modifier, a
  source allowing Copy and Move and a destination answering Move end with Move. The destination's `draggingSource` is
  the source object passed to the session.
- The source is sent willBeginAtPoint, movedToPoint while the pointer moves (X11 only; Wayland gives clients no global
  pointer position), then endedAtPoint:operation:.

## Dragging and dropping items

Pinned by `appkit-collection-view-drag.m`: the source side by mouse events sent to the window, with
`beginDraggingSessionWithItems:event:source:` overridden to record the drag instead of starting it; the destination
side by calling the view's NSDraggingDestination methods with a stand-in NSDraggingInfo. The view uses overlay
scrollers: with legacy ones the hosts narrow it after the first layout and keep the item views where that layout put
them, so item views and layout attributes disagree.

- A new collection view's source mask is every operation within the application and none outside it.
  `setDraggingSourceOperationMask:forLocal:` sets each.
- With a delegate that provides pasteboard writers, a press on an item does not track the mouse in `mouseDown:`; the
  drag starts on a later dragged event. Host A started on a 3 point move and on a 2 point move split over four events,
  host B on a 4 point move; neither on a single 2 point event. The test pins only that a click starts none.
- A drag from a selected item asks `canDrag` with the whole selection and the mouse-down event, then
  `pasteboardWriterForItemAtIndexPath:` for each selected item in index order, and begins one session with the
  mouse-down event and the view as the source: one dragging item per writer, framed at the item's view, with one image
  component. An item whose writer is nil is left out.
- A drag from an unselected item first selects only it (shouldDeselect the others, then shouldSelect it), then drags
  it. When `canDrag` answers NO nothing is dragged; it is asked again on every dragged event, and the selection stays.
- A plain click on an item that is already selected changes nothing and calls no delegate method (no highlight, no
  selection), with or without a drag-capable delegate. Pinned in `appkit-collection-view-mouse.m`.
- The drop proposed at a point is the flow layout's drop target there: on an item, `NSCollectionViewDropOn` that
  item (to its edges); in a gap of a row, `NSCollectionViewDropBefore` the item after it; right of a row's last item,
  before the next index; between rows, before the first item of the lower row; after the last item or below every
  item, before the item count. The delegate's changes to the index path and operation in `validateDrop:` are what
  `acceptDrop:` gets; when it returns None, enter and update return None and the drop is not accepted
  (`prepareForDragOperation:` still answers YES).
- Over an item the item's `highlightState` is `NSCollectionViewItemHighlightAsDropTarget`; over a gap, a view not
  part of any item fills the gap's frame (the layout's inter-item gap attributes). The gap view goes when the drag
  leaves. The item highlight stays through `performDragOperation:` and `concludeDragOperation:` and goes at
  `draggingEnded:`. Right after `draggingExited:` both hosts showed item 4 as the drop target again; not pinned.
- A view with no items takes a drop too: the flow layout's drop target anywhere in it is the gap before item 0 of
  section 0 (`NSCollectionElementCategoryInterItemGap`, zero frame), and the delegate is asked to validate, then
  accept, `NSCollectionViewDropBefore` index path {0,0}. The gap before an empty section's first item is one item size
  at the section's top inset, at x 0 rather than the left inset; the hosts give that same frame, with the item category,
  for any later index of the empty section (not pinned). Pinned by `test_drop_on_empty_view` and
  `uifoundation_flow_layout.m` `test_drop_target_in_empty_sections`.

## File URLs on the pasteboard

Pinned by `appkit-pasteboard-file-urls.m`, on a uniquely named pasteboard with two files made in the temporary
directory.

- Writing two file NSURLs gives two items, each with `public.file-url`; they read back as the two URLs in order, and
  `NSFilenamesPboardType` gives their paths.
- A pasteboard on which `NSFilenamesPboardType` is declared and set to a list of paths has `public.file-url` among
  its `types`, holds one item per path, and reads back as file URLs in order.
- `NSPasteboardURLReadingFileURLsOnlyKey` leaves out a web URL; `NSPasteboardURLReadingContentsConformToTypesKey`
  keeps the files whose type conforms to one of the given types (`public.image` keeps a `.png` and not a `.txt`;
  `public.movie` keeps neither). `canReadObjectForClasses:options:` answers as `readObjectsForClasses:options:` reads.
- On a pasteboard that holds only a list of file names, `availableTypeFromArray:` finds `public.file-url`, and
  `stringForType:` / `dataForType:` of it give the first file's URL string.
