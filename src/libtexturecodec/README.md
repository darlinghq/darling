# libtexturecodec (ASTC LDR decoder)

Plain C99, no dependencies beyond libc. Working names `libtexturecodec`, prefix `dtc_`;
the maintainers may rename them. This directory is the codec layer below ImageIO:
`CMakeLists.txt` builds it as the static library `darling_texturecodec`, which ImageIO
links (`src/frameworks/ImageIO/src/ImageIOASTC.m`) and which exports no symbol.

## What it does

- Decodes ASTC 2D LDR texture data to RGBA8 with straight (non-premultiplied) alpha.
  All 14 2D footprints (4x4 to 12x12), all 10 LDR endpoint modes (0, 1, 4, 5, 6, 8, 9,
  10, 12, 13, with blue contraction and the L+A modes), single and dual plane, 1 to 4
  partitions with the specification's partition hash, trit and quint integer sequence
  encoding, void-extent blocks, and the magenta error colour for illegal blocks.
- sRGB operation mode (`srgb != 0`) returns the stored sRGB-encoded bytes without
  applying any transfer function; linear LDR mode returns `decode_unorm8` values.
- Parses the 16-byte header of the `.astc` container.
- Pure functions: no allocation, no global state, no threads, no I/O. Decoding a
  corrupt block never fails: it yields the error colour, counted in a report.

## API

See `include/texturecodec/texturecodec.h`. The main entry point is

    dtc_status dtc_astc_decode(const uint8_t *data, size_t len, uint32_t block_w,
                               uint32_t block_h, uint32_t width, uint32_t height,
                               int srgb, uint8_t *rgba_out);

`data` is the block payload without the `.astc` header (`dtc_astc_file_parse_2d` gives
the dimensions, the payload starts at `DTC_ASTC_FILE_HEADER_SIZE`). `rgba_out` must hold
`width * height * 4` bytes.

## Out of scope

- Encoders (ASTC or BCn).
- HDR endpoint modes and HDR decode modes. In LDR mode texels that use HDR endpoint
  modes get the error colour, as the specification requires.
- 3D block footprints and 3D images (rejected with `DTC_E_UNSUPPORTED`).
- BCn decoding and KTX1/KTX2 containers, and writing ASTC through
  CGImageDestination. Those are later steps of the design.

## Build and test

In the Darling tree the tests are registered with ctest (`tests/CMakeLists.txt`, built with
the build machine's compiler: `ctest -R texturecodec` from the build directory). On their
own, from this directory:

    make          # build/libdtc.a (GNU make reads GNUmakefile)
    make test     # unit tests and reference vectors
    make test-san # same, with AddressSanitizer and UndefinedBehaviorSanitizer

Warnings are on (`-Wall -Wextra -Wpedantic -Wconversion`) and the target is zero.

## Verification status

Independent reference: the ARM astcenc 5.7.1 release binary, used as a black box (no source
read). Against it this decoder is bit-exact on

- 392 encoder-made vectors (14 footprints x 7 synthetic images x linear and sRGB x two
  presets; 76 committed in `tests/vectors`), including partial edge blocks, dual plane,
  1 to 4 partitions, all 12 weight ranges and all 17 colour ranges;
- about 560k random and randomly configured 128-bit blocks (28 vectors of 300 blocks
  committed in `tests/vectors-random`), which exercise illegal blocks and the error
  colour, HDR endpoint modes inside LDR decoding, endpoint mode 1, void-extent
  validation, mixed-class multi-partition CEMs and dual plane with several partitions.

Checked against the specification text only (by reading, with hand-computed expectations):
the `.astc` header parser, the argument and truncation handling, the infill index guard.
Mutation probes (`tools/mutation_probes.sh`) show the tests fail for 22 of 22 perturbations
of the decoder. Sanitizer output and open questions: `maintainer-question.md`; guessed
constants: `GUESS.tsv`.
