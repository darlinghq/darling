# Provenance

This decoder is clean-room: it was written from specification text only. No source
code of any ASTC decoder or encoder (astcenc, Mesa, any GitHub decoder) was read,
copied, translated or paraphrased.

## Specification sources (public)

| What | URL | sha256 of the fetched file | Fetched |
|---|---|---|---|
| Khronos Data Format Specification 1.4.0 (HTML), section 23 "ASTC Compressed Texture Image Formats" | https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html | `cbf7dd2aa550327615612cd731f388ab38ccd0f1c9728155fd74ffa4ce2b0c98` | 2026-11-22 |
| Plain-text extract of section 23, made with `tools/spec_to_text.py` (tags stripped, one table row per line) | derived from the file above | `e02468bf2f37db2571280e771993a48689ef84ff80a21f5a6b556c796828684b` | 2026-11-22 |

The fetched copies live outside this repository, in
`~/tmp-opencode/issue-2215-astc/spec-cache/khronos/` and `.../text/`. Everything the
decoder implements comes from that section: block layout (23.9), block modes
(23.10, tables 160 and 161), colour endpoint modes (23.11), integer sequence encoding
(23.12), endpoint and weight unquantization (23.13, 23.16), LDR endpoint decoding
(23.14.1), weight infill (23.17), weight application (23.18.1), dual plane (23.19),
partition function and `hash52` (23.20), data size determination (23.21), void-extent
blocks (23.22), illegal encodings (23.23), LDR profile (23.24).

The `.astc` container header (16 bytes: magic `13 AB A1 5C`, three block dimension
bytes, three 24-bit little-endian image dimensions) is taken from ARM's public
container description, https://github.com/ARM-software/astc-encoder/blob/main/Docs/FileFormat.md
(documentation, Apache-2.0; read as prose only, no source file).

The Khronos licence allows using the specification to implement the functionality;
no specification text is reproduced here.

## Apple documentation

None used. Only the names of the public ImageIO constants appear in the design note
(not in this repository).

## Test vectors

Reference vectors were produced by running the unmodified ARM astcenc 5.7.1 release
binary as a black box (see `tests/vectors/README.md`). Its source was not read.

| What | URL | sha256 |
|---|---|---|
| astcenc 5.7.1 Linux arm64 release zip | https://github.com/ARM-software/astc-encoder/releases/download/5.7.1/astcenc-5.7.1-linux-arm64.zip | `d87287e02e3fd335334dace5a6a1ffcb705ed5aa9d5a5c5624fa50860f665fde` (matches `release-sha256.txt` of that release) |
| `bin/astcenc-neon` extracted from the zip | | `e461c24b72ee39e26fc1c8b447a708685fd8cedd5e06f329d082b52a7acc6579` |

Licence of the tool: Apache-2.0. The compressed `.astc` blobs it produced from our own
synthetic images are committed with a notice (`tests/vectors/NOTICE`); see
`maintainer-question.md` for the open question about that.
