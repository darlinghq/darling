#!/bin/sh
#
# Copyright 2011 Shinichiro Hamaji. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of  conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials
#      provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# Usage:
#
#   %./unpack_xcode.sh xcode_3.2.6_and_ios_sdk_4.3__final.dmg
#
# The above commandline will put CLI tools in the dmg package into
# ./xcode_3.2.6_and_ios_sdk_4.3__final/root .
#
# This script was inspired by this document:
# http://devs.openttd.org/~truebrain/compile-farm/apple-darwin9.txt

set -e

PKGS="MacOSX10.6 gcc4.2 gcc4.0 llvm-gcc4.2 DeveloperToolsCLI"

dmg=$1
dir=`basename $dmg .dmg`

rm -fr $dir
mkdir $dir
cd $dir

7z x ../$dmg
7z x 5.hfs

for pkg in $PKGS; do
  7z x -y */Packages/$pkg.pkg
  7z x -y Payload
  mkdir -p $pkg
  cd $pkg
  cpio -i < ../Payload~
  cd ..
  rm -f Payload*
done

rm -fr root
mkdir root
for pkg in $PKGS; do
  if [ $pkg = "MacOSX10.6" ]; then
    cp -R $pkg/SDKs/*/* root
  else
    cd $pkg
    tar -c * | tar -xC ../root
    cd ..
  fi
done
ln -sf root/System/Library/Frameworks root/Library/Frameworks

echo "The package was unpacked into $dir/root"
