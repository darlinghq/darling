#!/bin/sh
# Builds and runs the group B probes one at a time; writes results-B-<macOS version>.txt next to this script.
# Usage: sh run-all-B.sh          (EXTRA="--synth" sh run-all-B.sh passes --synth to 649 and 650; --warp is never passed)
cd "$(dirname "$0")" || exit 1
VER=$(sw_vers -productVersion 2>/dev/null || echo unknown)
OUT="results-B-$VER.txt"
TMP=$(mktemp -d /tmp/probe-run.XXXXXX) || exit 1
: > "$OUT"
strip_identity() {
  H=$(hostname 2>/dev/null); HS=$(hostname -s 2>/dev/null)
  set -- -e "s|$HOME|~|g" -e 's|/Users/[^/ "]*|/Users/USER|g' -e 's|/private/var/folders/[^ "]*|/TMP|g' -e 's|/var/folders/[^ "]*|/TMP|g' -e "s|$TMP|/TMP|g"
  [ "${#USER}" -ge 3 ] && set -- "$@" -e "s|$USER|USER|g"
  [ "${#H}" -ge 3 ] && set -- "$@" -e "s|$H|HOST|g"
  [ "${#HS}" -ge 3 ] && set -- "$@" -e "s|$HS|HOST|g"
  sed "$@"
}
limit() { secs=$1; shift; perl -e 'alarm shift; exec @ARGV' "$secs" "$@"; }
for d in 642-menu-appearance 643-sharing-picker 649-menu-highlight-delegate 650-popup-menu-delegate 651-menu-key-equivalent 652-alert-sheet-lifetime 665-type-for-contents 667-untitled-document-error 669-make-document-errors 670-default-type 671-border-while-inside 674-window-size-limits 668-box-clip-browsercell; do
  if ! (cd "$d" && sh build.txt) > "$TMP/build.log" 2>&1; then
    echo "PROBE $d BUILD-FAILED -> $(tail -n 3 "$TMP/build.log" | tr '\n' ' ')" | strip_identity >> "$OUT"; continue
  fi
  extra=""
  case "$d" in 649*|650*) extra="$EXTRA" ;; esac
  # shellcheck disable=SC2086
  (cd "$d" && limit 300 ./probe $extra) > "$TMP/run.log" 2>&1
  rc=$?
  grep '^PROBE' "$TMP/run.log" | strip_identity >> "$OUT"
  [ "$rc" -ne 0 ] && echo "PROBE $d RUN-EXIT -> status $rc" >> "$OUT"
done
rm -rf "$TMP"
echo "wrote $OUT ($(wc -l < "$OUT" | tr -d ' ') lines)"
