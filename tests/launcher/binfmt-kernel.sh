#!/usr/bin/env bash
# Copyright (C) 2026 Lubos Dolezel
#
# Checks, against the running kernel, what the launcher's binfmt_misc handling assumes (see
# src/startup/launcher-binfmt.h): an entry with the O flag and no P flag starts the interpreter as
#   <interpreter> <path> <arguments...>
# with the descriptor of the file in AT_EXECFD. The entry is registered in a binfmt_misc mount of a
# private user namespace (Linux 6.7 or later), so nothing on the host is registered or needs root.
#
# usage: tests/launcher/binfmt-kernel.sh   (from the source tree; needs cc, unshare, python3)
# Exit status 77 means the kernel or the system does not allow this test.
set -u
here=$(cd "$(dirname "$0")" && pwd)
src=$(cd "$here/../../src/startup" && pwd)
work=$(mktemp -d /tmp/binfmt-kernel.XXXXXX)
trap 'rm -rf "$work"' EXIT
failures=0

for tool in cc unshare python3; do
	if ! command -v "$tool" > /dev/null; then
		echo "skip: $tool is not installed"
		exit 77
	fi
done

cc -std=gnu11 -Wall -D_GNU_SOURCE -I"$src" -o "$work/interpreter" \
	"$here/binfmt-interpreter.c" "$src/launcher-binfmt.c" || exit 2

# the magic of a 64-bit Mach-O file, followed by padding
printf '\xcf\xfa\xed\xfe\x07\x00\x00\x01padding' > "$work/prog"
chmod +x "$work/prog"
cp "$work/prog" "$work/shell"
ln -s prog "$work/alias"

# runs inside the user namespace: registers the entry, then runs the command
cat > "$work/inner.sh" <<'INNER'
#!/usr/bin/env bash
set -eu
work=$1; shift
mkdir -p "$work/binfmt"
mount -t binfmt_misc none "$work/binfmt"
printf ':macho-test:M::\\xcf\\xfa\\xed\\xfe::%s:O' "$work/interpreter" > "$work/binfmt/register"
cd "$work"
exec "$@"
INNER
chmod +x "$work/inner.sh"

run() {
	unshare --user --map-root-user --mount "$work/inner.sh" "$work" "$@"
}

if ! unshare --user --map-root-user --mount true 2>/dev/null; then
	echo "skip: no unprivileged user namespaces"
	exit 77
fi
if ! run true 2>/dev/null; then
	echo "skip: cannot mount binfmt_misc in a user namespace (needs Linux 6.7 or later)"
	exit 77
fi

check() {
	local name=$1 want=$2 got=$3
	if [ "$got" = "$want" ]; then
		echo "ok   $name"
	else
		echo "FAIL $name"
		echo "  want: $(printf '%s' "$want" | tr '\n' '|')"
		echo "  got:  $(printf '%s' "$got" | tr '\n' '|')"
		failures=$((failures + 1))
	fi
}

want=$(printf 'execfd present\narg <%s>\narg <exec>\narg <%s>\narg <a>\narg <b c>' "$work/interpreter" "$work/prog")
check "relative path and arguments" "$want" "$(run ./prog a 'b c')"

want=$(printf 'execfd present\narg <%s>\narg <exec>\narg <%s>\narg <shell>\narg <-h>' "$work/interpreter" "$work/shell")
check "program named like a subcommand" "$want" "$(run ./shell shell -h)"

want=$(printf 'execfd present\narg <%s>\narg <exec>\narg <%s>\narg <x>' "$work/interpreter" "$work/prog")
check "symlink resolves to the file" "$want" "$(run ./alias x)"

# fexecve() with an inheritable descriptor: the kernel then puts /dev/fd/N in argv, a name that is
# only good while that descriptor stays open, while the descriptor of AT_EXECFD names the file.
want=$(printf 'execfd present\narg <%s>\narg <exec>\narg <%s>\narg <y>' "$work/interpreter" "$work/prog")
check "fexecve of an inheritable descriptor" "$want" "$(run python3 -I -c '
import os
fd = os.open("prog", os.O_RDONLY)
os.set_inheritable(fd, True)
os.execve(fd, ["prog", "y"], os.environ)')"

if [ "$failures" -ne 0 ]; then
	echo "$failures failed"
	exit 1
fi
echo "binfmt kernel contract: ok"
