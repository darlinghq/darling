#!/bin/sh

BUILDSERVER="osx"

set -e

runtest() {
	source="$1"
	extension="${source##*.}"
	source_fn=$(echo "$source" | sed 's/\//_/g')

	case "$extension" in
		"cpp")
			tool="g++"
			;;
		"c" | "m")
			tool="gcc"
			;;
		*)
			echo "Unsupported file type: $extension"
			exit 1
	esac

	echo "====="
	echo "Running test '$source'"
	echo "====="

	cflags="$(grep '// CFLAGS' "$source" || true)"
	cflags="-w $(echo "$cflags" | cut -b 12-)"

	echo "Copying the source code to Darwin..."
	scp "$source" "$BUILDSERVER:/tmp/$$.$source_fn" >/dev/null
	echo "Building the source code for Darwin..."
	ssh "$BUILDSERVER" "$tool $cflags '/tmp/$$.$source_fn' -o '/tmp/$$.$source_fn.bin'"
	echo "Copying the binary over..."
	scp "$BUILDSERVER:/tmp/$$.$source_fn.bin" "/tmp" >/dev/null
	ssh "$BUILDSERVER" "rm -f /tmp/$$.$source_fn*"

	echo "Running Darwin binary..."
	out_darwin=$(dyld "/tmp/$$.$source_fn.bin")
	rm -f "/tmp/$$.$source_fn.bin"

	echo "Compiling native..."
	$tool $cflags "$source" -o "/tmp/$$.$source_fn.bin"
	echo "Running native binary..."
	out_native=$("/tmp/$$.$source_fn.bin")

	if [ "$out_darwin" != "$out_native" ]; then
		echo "*** ERROR: Output doesn't match!"
		echo "---"
		echo "Darwin build:"
		echo "---"
		echo "$out_darwin"
		echo "---"
		echo "Native build:"
		echo "---"
		echo "$out_native"
		exit 1
	fi
	rm -f "/tmp/$$.$source_fn.bin"

	echo "Everything OK, outputs match"
}

for test in "$@"; do
	runtest "$test"
done

