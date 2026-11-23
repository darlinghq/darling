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
    blobs from our own encoder later?
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
