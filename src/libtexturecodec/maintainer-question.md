# Open questions from the ASTC decoder (codec layer)

One item per question. Each says what the specification leaves open, what the code does
meanwhile (also listed in `GUESS.tsv`, ids in brackets) and who has to decide. Nothing
here blocks the decoder; each default is easy to change.

## Specification gaps

1. Void-extent validation [G4]. KDFS 23.22 says void extents "may not be checked" and
   23.23 lists "low coordinate greater than or equal to the high coordinate" as an
   illegal encoding. The code returns the error colour for such blocks (and for cleared
   reserved bits 10 and 11). The astcenc black box agrees on random void blocks. Keep?
2. Weight unquantization of bit-only ranges [G1]. 23.16 gives a 3-bit example that ends
   in a "5-bit value" although the target is 0..63. The code replicates to 6 bits.
   Confirmed bit-exact by the astcenc black box, but the written spec contradicts itself.
   Should we note the erratum upstream?
3. sRGB alpha [G2]. 23.18.1 can be read as using the 0x80 expansion for alpha too. The
   code does; the astcenc black box agrees. A Khronos or ARM statement would settle it.
4. Position of the extra CEM bits and the CCS bits [G8]. The spec says "variable
   position, directly below the weight data". Reading: weights occupy the top bits, the
   extra CEM bits sit directly below them and the CCS bits below those. Confirmed by the
   black box on random blocks only, not by any text.
5. Table 161 reading [G9]. The block-mode table was recovered from the HTML colspans
   because plain-text extraction loses the bit widths. The counts it implies (1872 legal,
   4 void-extent, 172 reserved block modes) were re-derived independently.
6. Out-of-grid weight reads in infill [G12]. Spec silent. Shown by brute force never to
   matter for the 14 footprints, guarded anyway.

## Behaviour choices that belong to the maintainers

7. sRGB contract [G3, design Q6, Q7b]. The decoder returns the stored sRGB-encoded bytes
   and no EOTF is applied; straight alpha. What colour space and alpha convention should
   ImageIO report for a decoded texture? The codec cannot decide this.
8. Linear mode versus HDR [G7]. 23.5 lets an implementation promote linear LDR to HDR
   operation. The decoder is strict LDR (HDR endpoint modes give magenta). If ImageIO has
   to show HDR ASTC, a later phase must add HDR modes and a float16 path. Needed?
9. Error blocks [G6, design Q10]. Corrupt blocks decode to magenta and are only counted
   (`dtc_decode_report`). Should ImageIO fail the whole image above some error count,
   or always show magenta?
10. Limits [G16]. No size cap besides `size_t` overflow. Does Darling want a per-axis cap
    (design suggests 16384) or a maximum decoded size?
11. Partial blocks and trailing bytes [G14, G15, G17]: tolerated trailing bytes, error on
    short payloads, edge texels clipped. Fine as policy?
12. Container strictness [G18]. Footprints outside the tables are CORRUPT, 3D and
    depth > 1 are UNSUPPORTED. Fine?

## Process and legal

13. Committed vectors from astcenc [design Q12]. `tests/vectors` holds 76 compressed
    files plus astcenc's decompression of them (about 290 KB), `tests/vectors-random`
    28 files with a hash (about 230 KB), with a `NOTICE`. The source images are ours.
    The astcenc binary is Apache-2.0 and was used as a black box (no source read), but the
    outputs are data derived from it. Is the notice enough, or should we regenerate the
    blobs from our own encoder later? In particular: does Apache-2.0 section 4 require the
    licence text of the tool to be shipped with output of this kind? The notices do not copy
    it and claim no copyright year or holder for the files.
14. Licence identifier [G19]. Sources carry `GPL-3.0-only`. Should it be `or-later`?
15. Python in the repository. The vector generators are Python 3 (standard library only)
    and need an astcenc release binary; the tests themselves need only a C compiler. Is a
    Python tool acceptable, or should the generators live outside the tree?
16. Patents and trademarks: not assessed (design Q15).

## Verification limits (for the PR text)

- Independent reference: astcenc 5.7.1 black box. Bit-exact on 392 encoder-made vectors
  (76 committed), and on about 560k random and randomly configured blocks (28 vectors
  of 300 committed; the larger runs were generated with tools/gen_random_vectors.py in
  scratch and an independent reviewer ran about 3.4M more, all matching). That covers everything except the items below.
- Reference gaps: astcenc never emitted endpoint mode 1, error blocks or HDR modes itself,
  so those are compared only through the random-block vectors (which it decodes as a black
  box). No HDR output exists (out of scope).
- Checked against the specification text by reading only: the `.astc` header parser, the
  argument and truncation handling, and the infill index guard.
- Not built or run: i386 and arm64-Mach-O slices, Darling's own toolchain. Only a native
  aarch64 Linux build with gcc and clang was run.

## ImageIO hook (src/frameworks/ImageIO/src/ImageIOASTC.m)

The hook decodes a `.astc` file to an RGBA8 CGImage with `dtc_astc_decode_ex`. The choices
below were made because neither the decoder nor the public ImageIO documentation settles
them; each is a GUESS item (I1 to I7 in `GUESS.tsv`) and is one small edit to change.

1. Type identifier [I3, design Q3]. No public source names a uniform type identifier for
   ASTC, so none is used: `CGImageSourceGetType` returns NULL and the type is not in
   `CGImageSourceCopyTypeIdentifiers`. A human with a Mac can print the real value through
   public calls; until then data is recognised by its magic number only. Does the
   NULL type cause trouble for callers that expect a string?
2. Error blocks [I4, decoder item 9]. An image with any error block yields NULL, never a
   partly magenta image. Is "fail the whole image" right, or should a threshold or the
   magenta image be preferred?
3. Limits [I5, decoder item 10]. At most 16384 texels per side and 2^26 texels in total,
   and the payload must hold every block the header asks for, all checked before
   allocating. Different numbers?
4. Colour space and alpha [I1, I2, decoder item 7]. sRGB colour space, straight alpha,
   RGBA8. To be probed on a Mac by a human; the tests state it as this implementation's
   contract, not as observed macOS behaviour.
5. Where the hook sits [design 4.5, Q5]. `CGImageSource.m` asks the ASTC source first and
   falls back to the Onyx2D decoders unchanged, because the Onyx2D decoder list is closed and
   lives in the darling-cocotron submodule. Alternative: a new `O2ImageSource_ASTC` in
   darling-cocotron plus a submodule bump. The first keeps the change in one repository.
6. Public symbols. The static library hides every `dtc_` symbol (`DTC_API` is defined empty
   for it and its users in `CMakeLists.txt`), so ImageIO's export list is unchanged. Is that
   the expected shape for an internal library, or should the build also check the export list?
7. Licence identifier [G19]. The decoder files say `GPL-3.0-only`, the new Darling-side files
   `GPL-3.0-or-later` (the repository's usual choice). One of the two should win.
8. Status of an undecodable file [I7]. A file with the magic number that does not decode is a
   source with count 0, and `CGImageSourceGetStatus` now reports invalid data for every source
   whose count is 0 (it said complete for any existing source). That also applies to the other
   formats. Is a status derived from the count right, or should only the ASTC source differ?

## Resolved by observation of macOS (batch 11d)

The ImageIO items 1 to 4 and 8 are settled by black-box runs on macOS 26.7.3 and 10.14.4
(cristim-mac-evidence.md, section 16; GUESS.tsv I1 to I7 say what each now is):

- type identifier org.khronos.astc, listed by CGImageSourceCopyTypeIdentifiers (item 1);
- an illegal block anywhere gives an image of the right size that draws nothing, not NULL (item 2);
- a side of 1048576 or more gives no image, nothing else limits the size (item 3); the 16384 and
  2^26 caps are gone;
- sRGB colour space, straight alpha, RGBA8 (item 4);
- every ASTC source has count 1 and status complete; the status function is unchanged (item 8).

Decoder items 1 (void extent validation) and the 8-bit conversion of a void-extent colour follow
macOS in the DTC_COMPAT mode that ImageIO uses (GUESS.tsv G4); the strict reading stays the default
of the library and is what the astcenc vectors check. Not settled: item 7 and decoder item 14 (the
licence identifier), items 13, 15 and 16 (the vectors' notice, Python tools, patents), item 5 (hook
location).
