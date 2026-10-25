# ARM64 GL upload stack arguments

Regression for #1283. These tests use authored inputs, an authored ELF callee,
and public GL APIs. They never inspect guest code, shaders, or guest buffers.

`glTexSubImage3D`, `glTexSubImage3DEXT`, `glCompressedTexSubImage3D`, and
`glCompressedTexSubImage3DARB` have eight integer register arguments followed by
32-bit format, 32-bit type/size, and a pointer. Darwin arm64 packs these last
arguments at stack offsets 0/4/8; Linux AAPCS64 uses 0/8/16. Returning an unadapted
Linux function address corrupts the trailing arguments.

Signatures: `src/frameworks/OpenGL/include/OpenGL/glext.h` (rung 2).
ABI evidence: compile/run the authored Darwin caller and Linux callee below
(rung 4); no proprietary binary analysis. The adapter is intentionally limited
to these four signatures, not a general untyped ABI conversion.

## Synthetic ABI control

Requires an ARM64 Linux host, configured Darling build and working prefix.
Build the host generator with:

```sh
c++ -std=c++11 -O2 src/libelfloader/wrapgen/wrapgen.cpp -ldl -o /absolute/scratch/wrapgen
```

Set these absolute paths from your build (do not install the fixture):

- `DARLING_SDK`: configured SDK
- `DARLING_LD`: built Darwin linker
- `DARLING_LIBSYSTEM`: built `libSystem.B.dylib`
- `DARLING_CRT`: built `crt1.10.6.dir/start.S.o`
- `DARLING_ELFCALLS`: `src/startup/mldr/elfcalls`
- `DARLING_EXTRA_LDFLAGS`: optional configured linker flags, separated by spaces
- `DARLING_CLANG`: optional Darwin-targeting clang command (default `clang`)

```sh
bash tests/wrapgen/gl-upload-abi/build-fixture.sh /absolute/scratch/wrapgen /absolute/scratch/abi
# Inside darling shell; substitute the actual scratch path:
DYLD_LIBRARY_PATH=/Volumes/SystemRoot/absolute/scratch/abi \
  /Volumes/SystemRoot/absolute/scratch/abi/guest-probe
```

Expected exit 0. All four entry points are exercised five times with changing
pointer identities, distinct callee IDs, all eight register arguments, and
positive/negative compressed sizes. The callee checks invocation counts, so
no-op bridges fail. Unrelated ten-argument uint64 calls check that wide stack
arguments remain intact. The first call exercises uncached native resolution;
subsequent calls exercise its cached path. No pointer is dereferenced by the
synthetic callee. Rebuild with the parent generator for the negative control:
exit 1, `GL trailing arguments corrupted at round 0`.

## Actual GL control

Compile `texture-array.m` with your configured AppKit Objective-C compile/link
recipe, linking AppKit, Foundation, OpenGL, libobjc and libSystem. Run it in a
working display session, first with the original `libGL.dylib`, then with
`DYLD_LIBRARY_PATH` pointing to the directory of the rebuilt native GL wrapper.
No installed files need changing.

The test allocates a 2x2x2 RGBA8 texture array and uploads four distinct byte
patterns through alternating core/EXT entry points, reading back each pattern.
It fails on a missing context, GL error, or byte mismatch. On Asahi/Mesa Apple
M2 Max: original wrapper exits 1 (`upload=500`, mismatch); corrected wrapper
exits 0, all four uploads/readbacks error 0 and matching. This does not prove
Roon's use of these calls or resolve all of its visual/performance defects.
