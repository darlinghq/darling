#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Run on the host with a Darling prefix installed (it uses `darling shell`).
# chsh(1) in the prefix changes the account's shell in the prefix's passwd and
# master.passwd, and a new `darling shell` runs that shell as a login shell.
# The original shell is put back at the end.
#
# (getpwnam() is covered by tests/src/opendirectory_write.c.)
#
# chsh asks for the account's password; the accounts of a Darling prefix have
# none, so the answer is an empty line.
set -u

fail=0
check() {
	if [ "$2" = "$3" ]; then
		echo "ok: $1"
	else
		echo "FAIL: $1: expected '$3', got '$2'"
		fail=1
	fi
}

original=$(darling shell 'dscl . -read /Users/$(id -un) UserShell' </dev/null | sed -n 's/^UserShell: //p')
check "original shell is known" "$([ -n "$original" ] && echo yes)" yes

# default shell
check "login shell before" "$(darling shell 'echo $0' </dev/null)" "-${original##*/}"

# change it to bash
printf '\n' | darling shell chsh -s /bin/bash >/dev/null 2>&1
check "dscl after chsh" "$(darling shell 'dscl . -read /Users/$(id -un) UserShell' </dev/null | sed -n 's/^UserShell: //p')" /bin/bash
check "new shell is bash" "$(darling shell 'echo $0' </dev/null)" -bash
check "bash is a login shell" "$(darling shell 'shopt -q login_shell && echo login' </dev/null)" login

# a shell that is not listed in /etc/shells is refused and nothing changes
printf '\n' | darling shell chsh -s /bin/not-a-shell >/dev/null 2>&1
check "refused shell leaves bash" "$(darling shell 'echo $0' </dev/null)" -bash

# put it back
printf '\n' | darling shell chsh -s "$original" >/dev/null 2>&1
check "shell restored" "$(darling shell 'echo $0' </dev/null)" "-${original##*/}"

exit $fail
