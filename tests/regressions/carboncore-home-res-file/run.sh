#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail
umask 077
ulimit -c 0
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
SUITE="$ROOT/tests/regressions/carboncore-home-res-file"
CXX="${1:-g++}"
OPT="${DARLING_HOME_RES_FILE_OPT:--O0}"
SRC="${DARLING_HOME_RES_FILE_SOURCE:-$ROOT/src/frameworks/CoreServices/src/CarbonCore/Resources.cpp}"
case "$OPT" in -O0|-O2) ;; *) echo 'expected -O0 or -O2' >&2; exit 2;; esac
TMP="$(mktemp -d -t darling-home-res-file.XXXXXXXX)"
trap 'rm -rf -- "$TMP"' EXIT
# Suppress existing warnings in unrelated resource APIs and aggregate setup.
"$CXX" -std=c++17 "$OPT" -g -Wall -Wextra -Werror \
  -Wno-unused-parameter -Wno-sign-compare -Wno-reorder -Wno-missing-field-initializers \
  -D__LITTLE_ENDIAN__ -fsanitize=address,undefined -fno-omit-frame-pointer \
  -ffunction-sections -fdata-sections -Wl,--gc-sections \
  -I"$SUITE/shims" -I"$ROOT/src/frameworks/CoreServices/include" \
  -I"$ROOT/src/frameworks/CoreServices/src/CarbonCore" \
  "$SRC" "$ROOT/src/frameworks/CoreServices/src/CarbonCore/ResourcesImpl.cpp" \
  "$SUITE/test.cpp" -pthread -o "$TMP/home-res-file"
TMPDIR="$TMP" ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
  timeout 30s "$TMP/home-res-file"
