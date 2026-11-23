#!/usr/bin/env bash
# Builds the parts of FEX that libdarling-fexcore.so links (FEXCore's static archives plus cephes
# and SoftFloat) and the shared fmt that matches FEX's bundled fmt headers. Neither FEXLoader, the
# thunks nor FEX's tests are built. src/startup/mldr/CMakeLists.txt (WITH_FEXCORE) describes what
# the shim needs from the result; tests/mldr/build-fexcore-shim.py consumes it.
#
#   tests/mldr/build-fex-for-shim.sh FEX_SOURCE_DIR FEX_BUILD_DIR
#
# FEX_SOURCE_DIR is a FEX checkout (src/startup/mldr/fex/FEX_VERSION names the commit the shim is
# built and tested against) with the submodules External/fmt, External/range-v3,
# External/unordered_dense and Source/Common/cpp-optparse. No other submodule is configured with
# the options below: vixl needs BUILD_TESTING or the VIXL simulator/disassembler, zydis needs
# ENABLE_ZYDIS, rpmalloc needs ENABLE_FEX_ALLOCATOR, jemalloc_glibc needs
# ENABLE_JEMALLOC_GLIBC_ALLOC, and xxhash comes from libxxhash-dev. Needs clang, clang++, cmake,
# ninja, python3 and libxxhash-dev. CC and CXX override the compilers; JOBS the parallelism. The
# compiler must accept preserve_all on aarch64 (a recent clang): the shim's options assume FEX used
# it, and tests/mldr/build-fexcore-shim.py checks FEX's cache for it.
#
# The FEX options mirror a plain release build without allocator hooks, LTO (the archives must hold
# object code, not bitcode), telemetry or tuning for the build machine (-mcpu=native would make a
# cached build depend on the runner's CPU model).
set -euo pipefail

src=$(cd "${1:?usage: $0 FEX_SOURCE_DIR FEX_BUILD_DIR}" && pwd)
mkdir -p "${2:?usage: $0 FEX_SOURCE_DIR FEX_BUILD_DIR}"
build=$(cd "$2" && pwd)
cc=${CC:-clang}
cxx=${CXX:-clang++}
jobs=${JOBS:-$(nproc)}

# FEX falls back to its bundled static fmt when no fmt package is found, but the shim is compiled
# with FMT_SHARED against the bundled headers and links a shared libfmt. A distro libfmt of another
# version would not match those headers, so build the bundled one as a shared library.
fmt_prefix=$build/fmt-prefix
cmake -S "$src/External/fmt" -B "$build/fmt" -G Ninja -DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=ON -DFMT_TEST=OFF -DFMT_DOC=OFF -DFMT_INSTALL=ON \
	-DCMAKE_C_COMPILER="$cc" -DCMAKE_CXX_COMPILER="$cxx" "-DCMAKE_INSTALL_PREFIX=$fmt_prefix"
cmake --build "$build/fmt" -j "$jobs"
cmake --install "$build/fmt" > /dev/null

cmake -S "$src" -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER="$cc" -DCMAKE_CXX_COMPILER="$cxx" \
	-DBUILD_TESTING=OFF -DBUILD_THUNKS=OFF -DBUILD_FEXCONFIG=OFF \
	-DENABLE_LTO=OFF -DENABLE_ASSERTIONS=OFF -DENABLE_CCACHE=OFF -DENABLE_FEX_ALLOCATOR=OFF \
	-DENABLE_JEMALLOC_GLIBC_ALLOC=OFF -DENABLE_OFFLINE_TELEMETRY=OFF -DTUNE_CPU=none \
	"-Dfmt_DIR=$fmt_prefix/lib/cmake/fmt"
# These four targets produce exactly the archives the shim links.
cmake --build "$build" -j "$jobs" --target FEXCore FEXCore_Base cephes_128bit softfloat_3e
ls -l "$build/FEXCore/Source/libFEXCore.a" "$build/FEXCore/Source/libFEXCore_Base.a" \
	"$build/External/cephes/libcephes_128bit.a" "$build/External/SoftFloat-3e/libsoftfloat_3e.a"
