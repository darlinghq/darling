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
# Run ld-mac or ld-mac32 based on $LD_MAC_BITS or the target binary
#
# Usage:
#
#   % ./ld-mac.sh hello32
#   % ./ld-mac.sh hello64
#
# See README for more detail

set -e

ld_mac_dir=$(cd $(dirname $0) && pwd)

if [ "x" != "x$LD_MAC_LOG" ]; then
    set -x
fi

if [ "x" = "x$LD_MAC_BITS" ]; then
    ld_mac_binary="$ld_mac_dir/ld-mac"
    if file $1 | grep i386 2>&1 > /dev/null; then
        ld_mac_binary="$ld_mac_dir/ld-mac32"
    fi
else
    ld_mac_binary="$ld_mac_dir/ld-mac$LD_MAC_BITS"
    if [ ! -x $ld_mac_binary ]; then
        echo "Cannot run $ld_mac_binary"
        exit 1
    fi
fi

exec $ld_mac_binary "$@"
