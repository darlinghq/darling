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
