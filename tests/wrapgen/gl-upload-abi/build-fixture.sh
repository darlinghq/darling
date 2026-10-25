#!/usr/bin/env bash
# Build an authored ELF callee, generated Mach-O bridge, and Darwin caller.
set -euo pipefail
: "${DARLING_SDK:?Set the configured Darling SDK directory}"
: "${DARLING_LD:?Set the configured Darwin linker}"
: "${DARLING_LIBSYSTEM:?Set the built libSystem.B.dylib path}"
: "${DARLING_CRT:?Set the built crt1 start.S.o path}"
: "${DARLING_ELFCALLS:?Set the directory containing elfcalls.h}"
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 /absolute/path/to/wrapgen /absolute/output/directory" >&2
  exit 2
fi
wrapgen=$1
mkdir -p "$2"
out=$(realpath "$2")
src=$(cd -- "$(dirname -- "$0")" && pwd)
"${CC:-cc}" -shared -fPIC "$src/native-fixture.c" \
  -Wl,-soname,"$out/fixture.so" -o "$out/fixture.so"
"$wrapgen" "$out/fixture.so" "$out/fixture.c" "$out/fixture_vars.h"
"${DARLING_CLANG:-clang}" -target arm64-apple-macosx26.0 \
  -isysroot "$DARLING_SDK" -I "$DARLING_ELFCALLS" \
  -c "$out/fixture.c" -o "$out/fixture.o"
# DARLING_EXTRA_LDFLAGS is a whitespace-separated list of linker flags.
read -r -a extra <<< "${DARLING_EXTRA_LDFLAGS:-}"
"$DARLING_LD" -dylib -arch arm64 -platform_version macos 26.0 26.0 \
  -syslibroot "$DARLING_SDK" "${extra[@]}" \
  -install_name /usr/lib/libdarling-gl-abi-fixture.dylib \
  "$out/fixture.o" "$DARLING_LIBSYSTEM" \
  -o "$out/libdarling-gl-abi-fixture.dylib"
"${DARLING_CLANG:-clang}" -target arm64-apple-macosx26.0 \
  -isysroot "$DARLING_SDK" -c "$src/guest-probe.c" -o "$out/guest-probe.o"
"$DARLING_LD" -arch arm64 -platform_version macos 26.0 26.0 \
  -syslibroot "$DARLING_SDK" "${extra[@]}" "$DARLING_CRT" \
  "$out/guest-probe.o" "$out/libdarling-gl-abi-fixture.dylib" \
  "$DARLING_LIBSYSTEM" -o "$out/guest-probe"
