#!/usr/bin/env bash
# Builds and runs, with plain clang and no CMake, the host unit tests that the amd64 CI job never
# compiles: the FEX bridge tests (registered in src/startup/mldr/CMakeLists.txt on aarch64 hosts
# only) and the launcher tests (src/startup/CMakeLists.txt). Neither needs FEX, the submodules or
# a Darling build, so this runs on any aarch64 machine with clang and clang++.
#
#   tests/mldr/run-fex-unit-tests.sh [BUILD_DIR]
#
# BUILD_DIR (default: a temporary directory, removed on exit) receives the test binaries.
# CC and CXX override the compilers. Prints "FAIL <test> (build|run)" for every failing test and
# exits 1 if there was one. The source lists below mirror the CMake targets; the last check
# fails when a launcher test, or an aarch64-only mldr test, registered in CMake is missing here.
#
# Flags match what the package build uses: RelWithDebInfo (-O2 -g; the package sets no build
# type and CMakeLists.txt defaults to it, with -DNDEBUG removed), -fPIC, and the include
# directories of src/startup and src/startup/mldr/include (mirroring CMake). The latter resolves
# to nothing without the src/external/cctools submodule, which this script does not need and CI
# does not check out, so loader.h compiles its fallback structs, not the cctools header.
# -Wall is stricter than the package.
set -u

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
cc=${CC:-clang}
cxx=${CXX:-clang++}

if [ "$(uname -m)" != aarch64 ] && [ "$(uname -m)" != arm64 ]; then
	echo "run-fex-unit-tests.sh: the FEX sources build on aarch64 hosts only (this is $(uname -m))" >&2
	exit 2
fi

if [ $# -ge 1 ]; then
	out=$1
	mkdir -p "$out"
	out=$(cd "$out" && pwd)
else
	out=$(mktemp -d)
	trap 'rm -rf "$out"' EXIT
fi

cflags=(-std=gnu11 -g -O2 -fPIC -D_GNU_SOURCE -DMLDR_BUILD -DINSTALL_PREFIX='"/usr"' -Wall "-I$root/src/startup")
mldr_cflags=("${cflags[@]}" -pthread "-I$root/src/startup/mldr" "-I$root/src/startup/mldr/include")
fex_dir=$root/src/startup/mldr/fex
failed=()
passed=0
covered_ctest=()

# run_test NAME BUILD_COMMAND...: builds, then runs "$out/NAME" from a scratch directory
run_test() {
	local name=$1
	shift
	local log=$out/$name.log
	echo "== $name"
	if ! "$@" -o "$out/$name" > "$log" 2>&1; then
		cat "$log"
		echo "FAIL $name (build)"
		failed+=("$name")
		return
	fi
	mkdir -p "$out/$name.cwd"
	(cd "$out/$name.cwd" && timeout -k 10 120 "$out/$name") > "$log" 2>&1
	local status=$?
	if [ $status = 0 ]; then
		passed=$((passed + 1))
	else
		cat "$log"
		if [ $status = 124 ]; then
			echo "FAIL $name (run: exit 124, a timeout after 120 s or the test's own status)"
		else
			echo "FAIL $name (run: exit status $status)"
		fi
		failed+=("$name")
	fi
}

# the FEX bridge: the dlsym bridge and callback thunks, and the shim search order
run_test mldr_fex_bridge_test "$cc" "${mldr_cflags[@]}" \
	"$root/tests/mldr/fex_bridge.c" "$fex_dir/fex_elfcalls.c" "$fex_dir/fex_syscall.c" -ldl
covered_ctest+=(mldr_fex_bridge)

covered_ctest+=(mldr_fexcore_paths)

# fex_engine.cpp is C++; the C sources are compiled by the C compiler first
mkdir -p "$out/fexcore_paths_obj"
: > "$out/fexcore_paths_obj.log"
fexcore_paths_objs=()
fexcore_paths_ok=1
for src in "$root/tests/mldr/fexcore_paths.c" "$fex_dir/fex_elfcalls.c" "$fex_dir/fex_syscall.c"; do
	obj=$out/fexcore_paths_obj/$(basename "$src").o
	"$cc" "${mldr_cflags[@]}" -c -o "$obj" "$src" >> "$out/fexcore_paths_obj.log" 2>&1 || fexcore_paths_ok=0
	fexcore_paths_objs+=("$obj")
done
obj=$out/fexcore_paths_obj/fex_engine.cpp.o
"$cxx" -std=c++17 -g -O2 -fPIC -D_GNU_SOURCE -DMLDR_BUILD -DINSTALL_PREFIX='"/usr"' -pthread \
	"-I$root/src/startup" "-I$root/src/startup/mldr" "-I$root/src/startup/mldr/include" -c -o "$obj" "$fex_dir/fex_engine.cpp" \
	>> "$out/fexcore_paths_obj.log" 2>&1 || fexcore_paths_ok=0
fexcore_paths_objs+=("$obj")
if [ $fexcore_paths_ok = 1 ]; then
	run_test mldr_fexcore_paths_test "$cxx" -pthread "${fexcore_paths_objs[@]}" -ldl
else
	echo "== mldr_fexcore_paths_test"
	cat "$out/fexcore_paths_obj.log"
	echo "FAIL mldr_fexcore_paths_test (build)"
	failed+=(mldr_fexcore_paths_test)
fi

# launcher tests: test source, then the launcher sources it checks (src/startup)
launcher_test() {
	local name=$1 test_src=$2
	shift 2
	local srcs=()
	for s in "$@"; do srcs+=("$root/src/startup/$s"); done
	run_test "launcher_${name}_test" "$cc" "${cflags[@]}" "$root/tests/launcher/$test_src" "${srcs[@]}"
	covered+=("tests/launcher/$test_src")
}
covered=()
launcher_test ids ids.c launcher-ids.c
launcher_test binfmt binfmt.c launcher-binfmt.c
launcher_test env env.c launcher-env.c
launcher_test locale locale.c launcher-locale.c
launcher_test pulse pulse.c launcher-pulse.c
launcher_test tmp tmp.c launcher-tmp.c
launcher_test mode mode.c launcher-mode.c launcher-server.c
launcher_test rootless_ns rootless-ns.c launcher-server.c
launcher_test prefix prefix.c launcher-prefix.c
launcher_test muvm_trampoline muvm-trampoline.c muvm-trampoline.c
launcher_test server server.c launcher-server.c
launcher_test prefix_state prefix-state.c
launcher_test profile_prefix profile-prefix.c

# A test added to CMake must be added here too, or it would silently go unrun on arm64. The lists
# are read from CMake and an empty result is an error, so a moved or reworded file cannot make
# the guard pass vacuously. Not guarded: the compile flags and the mldr source lists.
guard_fail() {
	echo "FAIL $1"
	failed+=("$1")
}
launcher_cmake=$root/src/startup/CMakeLists.txt
mldr_cmake=$root/src/startup/mldr/CMakeLists.txt
registered_launcher=$(grep -o 'tests/launcher/[A-Za-z0-9_-]*\.c' "$launcher_cmake" 2> /dev/null | sort -u)
# add_test names inside the aarch64-only blocks of the mldr CMake file. Limits: the block must open
# with a column-0 `if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")` and close with a column-0
# `endif()`, and each add_test(NAME x ...) must start on one line. POSIX awk only (gawk --posix).
registered_mldr=$(awk '/^if \(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64\|arm64"\)/ {in_block = 1; next}
	/^endif\(\)/ {in_block = 0}
	in_block && match($0, /add_test\(NAME [A-Za-z0-9_]+/) {print substr($0, RSTART + 14, RLENGTH - 14)}' \
	"$mldr_cmake" 2> /dev/null | sort -u)
[ -n "$registered_launcher" ] || guard_fail "drift guard: no launcher tests found in $launcher_cmake"
[ -n "$registered_mldr" ] || guard_fail "drift guard: no aarch64 mldr tests found in $mldr_cmake"
for registered in $registered_launcher; do
	case " ${covered[*]} " in
	*" $registered "*) ;;
	*) guard_fail "drift guard: $registered is built by src/startup/CMakeLists.txt but not by this script" ;;
	esac
done
for registered in $registered_mldr; do
	case " ${covered_ctest[*]} " in
	*" $registered "*) ;;
	*) guard_fail "drift guard: ctest $registered is registered for aarch64 in src/startup/mldr/CMakeLists.txt but not run by this script" ;;
	esac
done

echo
if [ ${#failed[@]} -gt 0 ]; then
	echo "${#failed[@]} failed, $passed passed. Failed:"
	printf '  %s\n' "${failed[@]}"
	exit 1
fi
echo "all $passed tests passed"
