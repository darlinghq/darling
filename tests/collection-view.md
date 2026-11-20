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
