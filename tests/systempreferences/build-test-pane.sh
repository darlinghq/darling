#!/bin/bash
# build-test-pane.sh <output directory> [-a arch]...
# Builds TestPane.prefPane (the pane the System Preferences test installs copies of) with
# the Darling guest toolchain script of the darling-skills repository (guest-cc.sh).
set -e
here=$(cd "$(dirname "$0")" && pwd)
out=$1; shift
arch=x86_64
[ "$1" = -a ] && arch=$2
pane=$out/TestPane.prefPane
mkdir -p "$pane/Contents/MacOS" "$pane/Contents/Resources"
guest-cc.sh -a "$arch" -o "$pane/Contents/MacOS/TestPane" "$here/TestPane.m" -bundle -framework Cocoa -framework PreferencePanes -fobjc-arc
cp "$here/Info.plist" "$pane/Contents/Info.plist"
cp "$here/Main.nib" "$pane/Contents/Resources/Main.nib"
