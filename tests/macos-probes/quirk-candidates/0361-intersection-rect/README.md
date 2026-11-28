# Probe 0361: NSIntersectionRect for ordinary, touching, empty and unbounded rectangles

**macOS result: NOT YET OBSERVED**

Purpose: Prints NSIntersectionRect (both argument orders), NSIntersectsRect, CGRectIntersection and NSIsEmptyRect for 17 rectangle pairs, including largest-double, infinite and NaN edges.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Pairs: overlap, contained, touching edge, touching corner, disjoint, zero size inside, zero width inside, negative size, a rectangle of the largest double size against a small one and against another huge one, an origin at the largest double, infinite size, infinite origin with infinite size (undefined edge), the full negative-to-positive largest range, NaN origin, NaN size, and two huge rectangles that do not overlap. Each pair is evaluated in both argument orders.

## Why it matters, and Darling code affected

The patch changes what is returned when an edge is infinite or undefined, so that a rectangle meeting a finite one is bounded by it instead of becoming the zero rectangle. Documented behaviour only covers finite rectangles; the unbounded cases are observable.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- darling-foundation master (read 2026-11-21): src/NSGeometry.m:126-158 (NSIntersectionRect: returns the zero rectangle when an edge test with <= is true; origin and size computed with branches on <= and >=) [master as read]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| max-size-vs-small returns the small rectangle | macOS bounds the result by the finite rectangle. By reading (not run), Darling's branches at NSGeometry.m:137-153 also return it for a finite largest-double rectangle. |
| The infinite-origin-with-infinite-size and NaN rows return a bounded non-zero rectangle | The patch direction (zero only when provably disjoint, the finite rectangle bounds the rest) is right. By reading (not run), Darling's branch order gives a NaN width for an undefined edge (a maximum edge of -inf + inf is NaN, every comparison is false), so those rows are where Darling would differ. |
| Touching edge returns the zero rectangle with zero origin | Pins the zero-rectangle origin convention. |
| Swapped order gives a different result | A macOS asymmetry; both orders must be matched. |

## Public documentation

- https://developer.apple.com/documentation/foundation/nsintersectionrect(_:_:)
- https://developer.apple.com/documentation/foundation/nsintersectsrect(_:_:)

## What this probe cannot answer

Nothing known; every item is an API-visible value or behaviour.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.
