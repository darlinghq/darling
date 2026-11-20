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
