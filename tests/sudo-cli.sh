#!/bin/sh
# Copyright (C) 2026 Adam Burdett
#
# Check sudo's NAME=value command prefix and argument boundaries (issue #1388).
# Contract: https://github.com/sudo-project/sudo/blob/cc856f0fc2ce40994cf699b246dee57e4fcbb5aa/docs/sudo.man.in
#
# Usage: sh sudo-cli.sh /absolute/path/to/darling/sudo
# Inside Darling, keep fixtures for cleanup from the host after shutdown:
#   SUDO_TEST_KEEP=1 sh sudo-cli.sh /usr/bin/sudo
# Host runs require a test build whose setuid/setgid calls are stubbed.
# There is no default host sudo invocation; do not select the host system sudo.

if [ "$#" -ne 1 ] || [ ! -x "$1" ]; then
    echo "Usage: $0 /absolute/path/to/darling/sudo" >&2
    exit 2
fi
case "$1" in
    /*) SUDO=$1 ;;
    *) echo "The sudo test binary path must be absolute." >&2; exit 2 ;;
esac

T=$(mktemp -d "${TMPDIR:-/tmp}/darling-sudo-cli.XXXXXX") || exit 2
if [ "${SUDO_TEST_KEEP:-0}" = 1 ]; then
    printf 'Keeping test fixtures: %s\n' "$T"
else
    trap 'rm -rf "$T"' 0
fi
trap 'exit 2' HUP INT TERM
checks=0
fails=0

check() {
    name=$1; expected_status=$2; expected_output=$3; shift 3
    actual_output=$("$@" 2>"$T/stderr"); actual_status=$?
    checks=$((checks + 1))
    if [ "$actual_status" -eq "$expected_status" ] &&
       [ "$actual_output" = "$expected_output" ] && [ ! -s "$T/stderr" ]; then
        printf 'PASS: %s\n' "$name"
    else
        fails=$((fails + 1))
        printf 'FAIL: %s (status %s, expected %s)\n' "$name" "$actual_status" "$expected_status"
        printf '  output: <%s>\n  expected: <%s>\n' "$actual_output" "$expected_output"
        cat "$T/stderr"
    fi
}

reject() {
    name=$1; shift
    actual_output=$("$@" 2>"$T/stderr"); actual_status=$?
    checks=$((checks + 1))
    if [ "$actual_status" -gt 0 ] && [ "$actual_status" -lt 128 ] && [ -z "$actual_output" ]; then
        printf 'PASS: %s\n' "$name"
    else
        fails=$((fails + 1))
        printf 'FAIL: %s (status %s, output <%s>)\n' "$name" "$actual_status" "$actual_output"
        cat "$T/stderr"
    fi
}

cat >"$T/path-probe" <<'EOF'
#!/bin/sh
printf '%s' "$DARLING_SUDO_TEST"
EOF
chmod +x "$T/path-probe" || exit 2
cp "$T/path-probe" "$T/DARLING_SUDO_TEST=command" || exit 2

mkdir "$T/original-path" "$T/assigned-path" || exit 2
cat >"$T/original-path/path-probe" <<'EOF'
#!/bin/sh
printf 'original|%s' "$PATH"
EOF
cat >"$T/assigned-path/path-probe" <<'EOF'
#!/bin/sh
printf 'assigned|%s' "$PATH"
EOF
chmod +x "$T/original-path/path-probe" "$T/assigned-path/path-probe" || exit 2

DARLING_SUDO_TEST=parent
export DARLING_SUDO_TEST

check "ordinary command" 0 ordinary "$SUDO" /bin/sh -c 'printf %s ordinary'
check "single assignment" 0 one "$SUDO" DARLING_SUDO_TEST=one \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "multiple assignments" 0 'one|two' "$SUDO" \
    DARLING_SUDO_FIRST=one DARLING_SUDO_SECOND=two \
    /bin/sh -c 'printf "%s|%s" "$DARLING_SUDO_FIRST" "$DARLING_SUDO_SECOND"'
check "inherited value" 0 parent "$SUDO" /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "last assignment wins" 0 last "$SUDO" DARLING_SUDO_TEST=first DARLING_SUDO_TEST=last \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "empty value remains set" 0 '<>:x' "$SUDO" DARLING_SUDO_TEST= \
    /bin/sh -c 'printf "<%s>:%s" "$DARLING_SUDO_TEST" "${DARLING_SUDO_TEST+x}"'
check "spaces and equals in value" 0 'two words=three' "$SUDO" \
    'DARLING_SUDO_TEST=two words=three' /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "HOME assignment from issue 1388" 0 "$T/home" "$SUDO" "HOME=$T/home" \
    /bin/sh -c 'printf %s "$HOME"'
check "command argument is not an assignment" 0 'environment|DARLING_SUDO_TEST=argument' \
    "$SUDO" DARLING_SUDO_TEST=environment /bin/sh \
    -c 'printf "%s|%s" "$DARLING_SUDO_TEST" "$1"' sh DARLING_SUDO_TEST=argument
check "command flags stay with the command" 0 'environment|-n|-u|--unknown=value|--' \
    "$SUDO" DARLING_SUDO_TEST=environment /bin/sh \
    -c 'printf %s "$DARLING_SUDO_TEST"; printf "|%s" "$@"' sh -n -u --unknown=value --
check "existing option" 0 option "$SUDO" -n DARLING_SUDO_TEST=option \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
# Exercise numeric option parsing without relying on host identity changes.
check "interspersed options and assignments" 0 'one|two' "$SUDO" -n \
    DARLING_SUDO_TEST=one -u 0 DARLING_SUDO_SECOND=two -g 0 \
    /bin/sh -c 'printf "%s|%s" "$DARLING_SUDO_TEST" "$DARLING_SUDO_SECOND"'
check "assignment before separator" 0 before "$SUDO" DARLING_SUDO_TEST=before -- \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "separator ends assignment parsing" 0 parent /usr/bin/env "PATH=$T:/bin:/usr/bin" \
    "$SUDO" -- DARLING_SUDO_TEST=command
check "command path containing equals" 0 parent "$SUDO" "$T/DARLING_SUDO_TEST=command"
check "relative command path containing equals" 0 parent /bin/sh \
    -c 'cd "$1" && "$2" ./DARLING_SUDO_TEST=command' sh "$T" "$SUDO"
# Native sudo resolves commands before applying command assignments, subject to
# policy such as secure_path. Darling preserves its inherited search path.
# https://github.com/sudo-project/sudo/blob/cc856f0fc2ce40994cf699b246dee57e4fcbb5aa/plugins/sudoers/sudoers.c
check "original PATH selects command, assigned PATH reaches child" 0 "original|$T/assigned-path" \
    /usr/bin/env "PATH=$T/original-path" "$SUDO" "PATH=$T/assigned-path" path-probe
# Preserve execvp's distinction between an absent PATH and an empty PATH.
# https://github.com/darlinghq/darling-Libc/blob/9f761b94f48513692bb0dd7c30b948ceb7bd6843/gen/FreeBSD/exec.c
check "absent original PATH uses default lookup" 0 "$T/assigned-path" /bin/sh \
    -c 'unset PATH; exec "$@"' sh "$SUDO" "PATH=$T/assigned-path" sh \
    -c 'printf %s "$PATH"'
check "empty original PATH searches current directory" 0 "original|$T/assigned-path" /bin/sh \
    -c 'cd "$1" && shift && PATH= exec "$@"' sh "$T/original-path" \
    "$SUDO" "PATH=$T/assigned-path" path-probe
check "ordinary list mode" 0 "$T/path-probe" /usr/bin/env "PATH=$T:/bin:/usr/bin" \
    "$SUDO" -l path-probe
check "list mode assignments preserve command lookup and arguments" 0 "$T/original-path/path-probe argument" \
    /usr/bin/env "PATH=$T/original-path:/bin:/usr/bin" "$SUDO" -l \
    "PATH=$T/assigned-path:/bin:/usr/bin" DARLING_SUDO_TEST=list path-probe argument
check "command exit status" 37 '' "$SUDO" DARLING_SUDO_TEST=value /bin/sh -c 'exit 37'

reject "empty assignment name" "$SUDO" '=invalid' /bin/sh -c 'printf unexpected'
reject "invalid assignment after a valid one" "$SUDO" DARLING_SUDO_TEST=value \
    '=invalid' /bin/sh -c 'printf unexpected'
reject "assignment without command" "$SUDO" DARLING_SUDO_TEST=value
reject "assignment and option without command" "$SUDO" DARLING_SUDO_TEST=value -n
reject "assignment and separator without command" "$SUDO" DARLING_SUDO_TEST=value --
reject "unknown option is not an assignment" "$SUDO" --unknown=value /bin/sh -c 'printf unexpected'
reject "missing executable" "$SUDO" DARLING_SUDO_TEST=value "$T/nonexistent"

# Darling's existing -u/-g interface accepts numeric IDs, not account names.
# Check both the ID range and strtoul overflow, with a command that must not run.
for option in -u -g; do
    reject "missing $option operand after assignment" "$SUDO" DARLING_SUDO_TEST=value "$option"
    for value in '' invalid +1 -1 ' 1' '1 ' 1x 4294967296 184467440737095516160; do
        reject "invalid $option operand <$value> after assignment" "$SUDO" \
            DARLING_SUDO_TEST=value "$option" "$value" /bin/sh -c 'printf unexpected'
    done
done

printf '%s checks, %s failures\n' "$checks" "$fails"
[ "$fails" -eq 0 ]
