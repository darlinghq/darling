#!/bin/sh
# Runs both bundle variants under every combination of the user default and the delegate answer, with and
# without a file argument. Each run lasts about 2 seconds.
cd "$(dirname "$0")" || exit 1
TMP=$(mktemp -d)
printf 'probe\n' > "$TMP/sample.txt"
for variant in plain ubiq; do
  exe="probe0350-$variant.app/Contents/MacOS/probe0350"
  for default in unset YES NO; do
    for refuse in NO YES; do
      args=""
      [ "$default" != unset ] && args="-NSShowAppCentricOpenPanelInsteadOfUntitledFile $default"
      echo "PROBE 0350.run variant=$variant default=$default delegateRefusesUntitled=$refuse file=none -> (see lines below)"
      # shellcheck disable=SC2086
      "$exe" $args -probeRefuseUntitled "$refuse" 2>&1 | grep '^PROBE'
    done
    echo "PROBE 0350.run variant=$variant default=$default delegateRefusesUntitled=NO file=one-text-file -> (see lines below)"
    # shellcheck disable=SC2086
    "$exe" $args -probeRefuseUntitled NO "$TMP/sample.txt" 2>&1 | grep '^PROBE'
  done
done
rm -rf "$TMP"
