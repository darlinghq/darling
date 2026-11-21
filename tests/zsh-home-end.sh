#!/bin/bash
# Test for the Home/End key bindings that /etc/zshrc sets up in interactive zsh.
# Copyright (C) 2026 Lubos Dolezel
#
# Usage: zsh-home-end.sh [path-to-zsh]       (default: zsh from PATH)
# The ^[OH / ^[OF (application keypad mode) bindings are also what real macOS
# has. The ^[[H / ^[[F ones are a Darling QUIRK (see /etc/zshrc, src/external/zsh/zshrc):
# real macOS does not bind them, so those two checks fail there by design.
# Prints PASS/FAIL lines and exits non-zero if anything failed.

ZSH=${1:-zsh}
fails=0
pass() { echo "PASS $1"; }
fail() { echo "FAIL $1"; fails=$((fails + 1)); }
# bound KEYSEQ WIDGET: the interactive shell's global rc must bind KEYSEQ to WIDGET
bound() {
	local got
	got=$("$ZSH" -i -c "bindkey '$1'" </dev/null 2>/dev/null)
	if [ "$got" = "\"$1\" $2" ]; then pass "$1 -> $2"; else fail "$1 -> $2 (got '$got')"; fi
}

# application keypad mode, bound on macOS too
bound '^[OH' beginning-of-line
bound '^[OF' end-of-line
# test_quirk_home_end_normal_keypad_mode: normal keypad mode (xterm-style terminals)
bound '^[[H' beginning-of-line
bound '^[[F' end-of-line
# documented behaviour must not be weakened
bound '^[[3~' delete-char

[ "$fails" -eq 0 ]
