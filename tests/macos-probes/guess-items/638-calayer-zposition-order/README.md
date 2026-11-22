# Probe 638: sibling draw order by zPosition in CARenderer

PR: darlinghq/darling-cocotron#638 (QuartzCore: CARenderer draws sublayers in zPosition order).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: no. This is observable: `+[CARenderer rendererWithMTLTexture:options:]` (macOS 10.13 and later) renders a layer tree headlessly into a Metal texture the probe creates itself, and the probe reads one pixel back. No window, no activation. If there is no Metal device, every line says so. The plane fast path (`CALayerPlane`) of the PR is a Darling internal and is not observable.

Layers: three opaque 60x60 layers A (red), B (blue), C (green) on a white 100x100 root, overlapping at the centre pixel. Each line reads that pixel and names the layer on top, for the given add order and zPosition values.

## The GUESS items and what the probe prints for each

1. "A NaN counts as 0 (the documentation does not define it)" -> `638.2.nan`: a NaN against 1, -1, another NaN, in pairs and in a triple, in both add orders, each row annotated with what "NaN as 0" and "array order" would give.
   - Rows follow "NaN as 0": keep. Rows follow array order, or the NaN layer always goes under or over: change (and say what macOS does). `638.5.nan` repeats it for an animated value from NaN to NaN.
2. "An animation class `interpolateFloatInLayerKey` does not handle (for example a group) makes the helper return 0 for that layer ... Say if the model value should be used instead" -> `638.5.group` (a group containing a zPosition animation fixed at 100, with model z 0 and 9, against a sibling at model z 5), plus `638.5.keyframe`, `638.5.spring`, `638.5.additive`, `638.5.byvalue`, `638.5.basic` (the supported baseline in both directions), `638.5.on-b`, `638.5.other-key` (an opacity animation does not disturb z).
   - Group row puts A on top (presentation z 100): macOS evaluates nested animations; the PR's 0 is wrong for that class (change). Group row follows the model value (A at 0 under B at 5, A at 9 over B at 5): the model-value alternative the PR asks about. Group row as if z were 0 matches the PR.
3. "Layers at the same depth keep array order" -> `638.1.equal`, `638.1.triple` (ties inside a triple, both orders), `638.1.negzero` (-0 against 0), `638.1.tiny`, `638.1.large`.
4. The documented base behaviour: higher zPosition drawn later, negative values, sibling-only effect -> `638.1.*` and `638.3.nested` (a child with z 10 inside A against a sibling of A with z 1: is the child above the sibling?), `638.4.perspective` (parent sublayerTransform with m34, where real depth could apply), `638.2.inf`.

Output names: `A(red)`, `B(blue)`, `C(green)`, `white(background)`, `transparent-black(nothing drawn?)` (the renderer drew nothing: then no row is meaningful), or `other:rrggbb`.

## Apple documentation

- https://developer.apple.com/documentation/quartzcore/calayer/zposition
- https://developer.apple.com/documentation/quartzcore/carenderer
- https://developer.apple.com/documentation/quartzcore/carenderer/init(mtltexture:options:)

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>` with a scratch Metal header (declaring the few `MTL*` names the probe uses), plus `-[CALayer zPosition]`, `-[CALayer convertTime:fromLayer:]`, `-[CAKeyframeAnimation values/keyTimes]` and `-[CARenderer addUpdateRect:]`, none of which Darling's SDK headers declare; the macOS headers declare all of them. The stub was only for the syntax check.

## What this probe cannot answer

Behaviour of Darling's GL renderer and its plane fast path; exact pixel values (only which layer is on top is read); whether CARenderer needs a runloop turn between adding an animation and rendering (if every animated row looks like the model order, repeat with a short run-loop spin inserted before `centrePixel`).
