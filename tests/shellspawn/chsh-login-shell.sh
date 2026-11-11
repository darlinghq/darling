#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# Run on the host with a Darling prefix installed (it uses `darling shell`).
# chsh(1) in the prefix changes the account's shell in the prefix's passwd and
# master.passwd, and a new `darling shell` runs that shell as a login shell
# (argv[0] with a leading dash). The original shell is put back at the end.
#
# The shell is identified by feeding commands to `darling shell` on standard
# input; chsh asks for the account's password, and the accounts of a Darling
# prefix have none, so its answer is an empty line.
set -u

tmp=$(mktemp -d /tmp/chsh-login-shell.XXXXXX) || exit 1
trap 'rm -rf "$tmp"' EXIT
printf '\n' > "$tmp/newline"
printf 'echo shell=$0\ngrep "^$(id -un):" /etc/passwd | cut -d: -f7\n' > "$tmp/commands"

fail=0
check() {
	if [ "$2" = "$3" ]; then
		echo "ok: $1"
	else
		echo "FAIL: $1: expected '$3', got '$2'"
		fail=1
	fi
}

# Prints "<argv0 of the shell> <shell field of passwd>"
identify() {
	timeout 90 darling shell < "$tmp/commands" > "$tmp/out" 2>&1
	sed -n -e 's/^shell=//p' "$tmp/out" | tr '\n' ' '
	sed -n -e '/^shell=/!p' "$tmp/out" | tr -d '\n'
}
change_shell() {
	timeout 90 darling shell chsh -s "$1" < "$tmp/newline" > "$tmp/chsh" 2>&1
}

read -r -a before <<< "$(identify)"
original=${before[1]:-}
check "original shell is known" "$([ -n "$original" ] && echo yes)" yes
check "login shell has a leading dash" "${before[0]}" "-${original##*/}"

# change it to another shell listed in /etc/shells
case "$original" in
/bin/bash) other=/bin/zsh ;;
*) other=/bin/bash ;;
esac
change_shell "$other"
check "after chsh $other" "$(identify)" "-${other##*/} $other"

# a shell that is not listed in /etc/shells is refused and nothing changes
change_shell /bin/not-a-shell
check "refused shell leaves $other" "$(identify)" "-${other##*/} $other"

# put it back
change_shell "$original"
check "shell restored" "$(identify)" "-${original##*/} $original"

exit $fail
