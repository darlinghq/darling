#!/bin/sh
# Builds and runs every probe, one at a time, and writes results-<macOS version>.txt next to this script.
# Needs the stock Xcode command line tools. Usage:  sh run-all.sh        (PROBE_UI=1 also runs window-opening parts)
cd "$(dirname "$0")" || exit 1
VER=$(sw_vers -productVersion 2>/dev/null || echo unknown)
OUT="results-$VER.txt"
TMP=$(mktemp -d /tmp/probe-run.XXXXXX) || exit 1
: > "$OUT"

# Remove anything that identifies the person or the machine.
strip_identity() {
  H=$(hostname 2>/dev/null); HS=$(hostname -s 2>/dev/null)
  set -- -e "s|$HOME|~|g" -e 's|/Users/[^/ "]*|/Users/USER|g' -e 's|/private/var/folders/[^ "]*|/TMP|g' -e 's|/var/folders/[^ "]*|/TMP|g' -e "s|$TMP|/TMP|g"
  [ "${#USER}" -ge 3 ] && set -- "$@" -e "s|$USER|USER|g"
  [ "${#H}" -ge 3 ] && set -- "$@" -e "s|$H|HOST|g"
  [ "${#HS}" -ge 3 ] && set -- "$@" -e "s|$HS|HOST|g"
  sed "$@"
}

emit() { printf '%s\n' "$*" | strip_identity >> "$OUT"; }
limit() { secs=$1; shift; perl -e 'alarm shift; exec @ARGV' "$secs" "$@"; }

emit "# probe results, macOS $VER, machine $(uname -m), $(date -u +%Y-%m-%dT%H:%MZ)"
emit "# built with: $(clang --version 2>/dev/null | head -1)"

for d in 0012-control-metrics 0024-bindings-selection-and-errors 0033-scroller-geometry 0044-alert-layout 0050-tabview-geometry 0056-print-callback-and-help 0057-view-rects-outside-display 0108-menu-item-getter-lifetime 0109-imageio-metadata 0113-toolbar-button-metrics 0122-clip-edge-coverage 0201-no-window-server-run-loop 0215-ctfont-copy-memory-font 0350-document-app-launch 0352-file-reference-urls 0361-intersection-rect 0009-secitem-no-match item-a-calayer-iosurface-use-count item-b-textfinder-matching item-c-preferred-font-text-style item-d-cgpdf-object-type item-e-ats-fonts-path-timing item-f-menu-effective-appearance; do
  [ -d "$d" ] || continue
  if ! (cd "$d" && sh build.txt) > "$TMP/build.log" 2>&1; then
    emit "PROBE $d BUILD-FAILED -> $(tail -n 3 "$TMP/build.log" | tr '\n' ' ')"
    continue
  fi
  extra=""
  if [ "$PROBE_UI" = 1 ]; then
    case "$d" in 0056*) extra="--ui" ;; 0113*) extra="--front" ;; esac
  fi
  t=180
  case "$d" in 0201*) t=60 ;; esac
  if [ -x "$d/run.sh" ]; then
    (cd "$d" && limit 240 sh ./run.sh) > "$TMP/run.log" 2>&1
  else
    (cd "$d" && limit "$t" ./probe $extra) > "$TMP/run.log" 2>&1
  fi
  rc=$?
  grep '^PROBE' "$TMP/run.log" | strip_identity >> "$OUT"
  if [ "$rc" -ne 0 ]; then
    emit "PROBE $d RUN-EXIT -> status $rc (a probe may legitimately end non-zero; check the lines above)"
  fi
done

rm -rf "$TMP"
echo "wrote $OUT ($(wc -l < "$OUT" | tr -d ' ') lines)"
