#!/usr/bin/env bash
# Tests the validation of tests/mldr/build-fexcore-shim.py (--check mode: no FEX tree or compiler
# needed) against scratch copies of src/startup/mldr/CMakeLists.txt, FEX's build cache and the
# FEX pin. The unchanged files must pass; each variant must fail with the stated message.
#
#   tests/mldr/test-build-fexcore-shim.sh
set -u

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
failed=0

# fresh NAME: a scratch tree laid out like the repository, with a fake FEX build cache
fresh() {
	t=$scratch/$1
	mkdir -p "$t/tests/mldr" "$t/src/startup/mldr/fex" "$t/fmt/lib/cmake/fmt" "$t/build"
	cp "$root/tests/mldr/build-fexcore-shim.py" "$t/tests/mldr/"
	cp "$root/src/startup/mldr/CMakeLists.txt" "$t/src/startup/mldr/"
	cp "$root/src/startup/mldr/fex/FEX_VERSION" "$t/src/startup/mldr/fex/"
	cp "$root/THIRD-PARTY-NOTICES.md" "$t/"
	: > "$t/fmt/lib/libfmt.so"
	printf 'HAS_CLANG_PRESERVE_ALL:INTERNAL=1\nfmt_DIR:PATH=%s/fmt/lib/cmake/fmt\n' "$t" > "$t/build/CMakeCache.txt"
}

# expect NAME WANT: runs --check in scratch tree NAME; WANT is "pass" or text the failure must print
expect() {
	local name=$1 want=$2 log
	log=$(FEXCORE_SOURCE_DIR=$scratch/none FEXCORE_BUILD_DIR=$scratch/$name/build \
		"$scratch/$name/tests/mldr/build-fexcore-shim.py" --check 2>&1)
	local status=$?
	if [ "$want" = pass ] && [ $status = 0 ]; then
		echo "PASS $name"
	elif [ "$want" != pass ] && [ $status = 1 ] && [[ $log == *"$want"* ]] && [[ $log != *Traceback* ]]; then
		echo "PASS $name (failed as expected)"
	else
		echo "FAIL $name (want $want; exit $status): $log"
		failed=1
	fi
}

fresh current
expect current pass

n=0
for variant in \
	"target_compile_definitions(darling-fexcore PRIVATE -DEXTRA=1)|target_compile_definitions() changes" \
	"target_sources(darling-fexcore PRIVATE extra.cpp)|target_sources() changes" \
	"set_target_properties(darling-fexcore PROPERTIES CXX_VISIBILITY_PRESET hidden)|set_target_properties() changes" \
	"target_compile_options(darling-fexcore PRIVATE -DEXTRA=1)|target_compile_options() names darling-fexcore 2 times" \
	"target_link_libraries(darling-fexcore PRIVATE -lextra)|target_link_libraries() names darling-fexcore 2 times"; do
	n=$((n + 1))
	name=cmake_variant_$n
	fresh "$name"
	printf '\n%s\n' "${variant%%|*}" >> "$scratch/$name/src/startup/mldr/CMakeLists.txt"
	expect "$name" "${variant##*|}"
done

fresh no_preserve_all
sed -i 's/HAS_CLANG_PRESERVE_ALL:INTERNAL=1/HAS_CLANG_PRESERVE_ALL:INTERNAL=0/' "$scratch/no_preserve_all/build/CMakeCache.txt"
expect no_preserve_all "HAS_CLANG_PRESERVE_ALL=1"

fresh preserve_all_missing
sed -i '/HAS_CLANG_PRESERVE_ALL/d' "$scratch/preserve_all_missing/build/CMakeCache.txt"
expect preserve_all_missing "HAS_CLANG_PRESERVE_ALL=1"

fresh no_cache
rm "$scratch/no_cache/build/CMakeCache.txt"
expect no_cache "cannot read FEX's build cache"

fresh no_fmt_dir
sed -i '/^fmt_DIR/d' "$scratch/no_fmt_dir/build/CMakeCache.txt"
expect no_fmt_dir "fmt_DIR is not set"

fresh no_fmt_library
rm "$scratch/no_fmt_library/fmt/lib/libfmt.so"
expect no_fmt_library "cannot find libfmt.so"

fresh short_pin
echo 8e95a994a > "$scratch/short_pin/src/startup/mldr/fex/FEX_VERSION"
expect short_pin "40 digit commit id"

fresh pin_differs
echo 0000000000000000000000000000000000000000 > "$scratch/pin_differs/src/startup/mldr/fex/FEX_VERSION"
expect pin_differs "THIRD-PARTY-NOTICES.md names FEX commit"

exit $failed
