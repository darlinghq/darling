#!/bin/sh
# Copyright (C) 2026 Adam Burdett
#
# Check sudo's NAME=value command prefix and argument boundaries (issue #1388).
# Contract: https://github.com/sudo-project/sudo/blob/main/docs/sudo.man.in
#
# Usage: sh sudo-cli.sh /absolute/path/to/darling/sudo
# Run inside Darling with /usr/bin/sudo, or against a host test build whose
# setuid/setgid calls are stubbed. There is no default host sudo invocation.
# Set SUDO_TEST_KEEP=1 to retain fixtures for cleanup from the host.

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
    fi
}

cat >"$T/path-probe" <<'EOF'
#!/bin/sh
printf '%s' "$DARLING_SUDO_TEST"
EOF
chmod +x "$T/path-probe" || exit 2
cp "$T/path-probe" "$T/DARLING_SUDO_TEST=command" || exit 2

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
check "command argument is not an assignment" 0 'environment|DARLING_SUDO_TEST=argument' \
    "$SUDO" DARLING_SUDO_TEST=environment /bin/sh \
    -c 'printf "%s|%s" "$DARLING_SUDO_TEST" "$1"' sh DARLING_SUDO_TEST=argument
check "existing option" 0 option "$SUDO" -n DARLING_SUDO_TEST=option \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "assignment before separator" 0 before "$SUDO" DARLING_SUDO_TEST=before -- \
    /bin/sh -c 'printf %s "$DARLING_SUDO_TEST"'
check "separator ends assignment parsing" 0 parent /usr/bin/env "PATH=$T:/bin:/usr/bin" \
    "$SUDO" -- DARLING_SUDO_TEST=command
check "command path containing equals" 0 parent "$SUDO" "$T/DARLING_SUDO_TEST=command"
check "relative command path containing equals" 0 parent /bin/sh \
    -c 'cd "$1" && "$2" ./DARLING_SUDO_TEST=command' sh "$T" "$SUDO"
check "assigned PATH controls command lookup" 0 found "$SUDO" \
    "PATH=$T" DARLING_SUDO_TEST=found path-probe
check "ordinary list mode" 0 "$T/path-probe" /usr/bin/env "PATH=$T:/bin:/usr/bin" \
    "$SUDO" -l path-probe
check "command exit status" 37 '' "$SUDO" DARLING_SUDO_TEST=value /bin/sh -c 'exit 37'

reject "empty assignment name" "$SUDO" '=invalid' /bin/sh -c 'printf unexpected'
reject "invalid assignment after a valid one" "$SUDO" DARLING_SUDO_TEST=value \
    '=invalid' /bin/sh -c 'printf unexpected'
reject "assignment without command" "$SUDO" DARLING_SUDO_TEST=value
reject "assignment and option without command" "$SUDO" DARLING_SUDO_TEST=value -n
reject "assignment and separator without command" "$SUDO" DARLING_SUDO_TEST=value --
reject "unknown option is not an assignment" "$SUDO" --unknown=value /bin/sh -c 'printf unexpected'
reject "missing executable" "$SUDO" DARLING_SUDO_TEST=value "$T/nonexistent"

printf '%s checks, %s failures\n' "$checks" "$fails"
[ "$fails" -eq 0 ]
