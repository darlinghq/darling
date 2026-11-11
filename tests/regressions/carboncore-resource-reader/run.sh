#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Linux host test of the real reader with test-only memory/xattr adapters.
# Usage: bash tests/regressions/carboncore-resource-reader/run.sh g++
set -Eeuo pipefail
umask 077
ulimit -c 0
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
SUITE="$ROOT/tests/regressions/carboncore-resource-reader"
SRC="${DARLING_RESOURCE_READER_SOURCE:-$ROOT/src/frameworks/CoreServices/src/CarbonCore/ResourcesImpl.cpp}"
CXX="${1:-g++}"
OPT="${DARLING_RESOURCE_READER_OPT:--O0}"
case "$OPT" in -O0|-O2) ;; *) echo 'FAIL: expected -O0 or -O2' >&2; exit 2;; esac
[[ -f "$SRC" && -f "$SUITE/test.cpp" ]] || { echo 'FAIL: missing reader source/tests' >&2; exit 2; }
command -v "$CXX" >/dev/null 2>&1 || { echo "FAIL: missing $CXX" >&2; exit 2; }
TMP="$(mktemp -d -t darling-resource-reader.XXXXXXXX)"
trap 'rm -rf -- "$TMP"' EXIT
# Existing add() aggregate initialization and constructor order also occur
# in the unpatched source used for the before/after comparison.
"$CXX" -std=c++17 "$OPT" -g -Wall -Wextra -Werror \
  -Wno-unused-parameter -Wno-reorder -Wno-missing-field-initializers \
  -D__LITTLE_ENDIAN__ -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$SUITE/shims" -I"$ROOT/src/frameworks/CoreServices/src/CarbonCore" \
  "$SRC" "$SUITE/test.cpp" -o "$TMP/resource-reader"
TMPDIR="$TMP" ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  timeout 30s "$TMP/resource-reader" "${@:2}"
