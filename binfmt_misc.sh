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
# Setup binfmt_misc for Mach-O binaries.
#
# Usage:
#
#   % ./binfmt_misc.sh
#   % ./binfmt_misc.sh stop

cmd=$1
if [ "x$cmd" = "x" ]; then
    cmd=start
fi

if [ "x$2" = "x" ]; then
    ld_mac_dir=$(cd $(dirname $0); pwd)
    ld_mac=$ld_mac_dir/ld-mac
else
    ld_mac="$2"
fi

case $cmd in
    start)
        sudo sh -c "
echo :Mach-O '64bit:M::\\xcf\\xfa\\xed\\xfe::$ld_mac:' > /proc/sys/fs/binfmt_misc/register
echo :Mach-O '32bit:M::\\xce\\xfa\\xed\\xfe::$ld_mac:' > /proc/sys/fs/binfmt_misc/register
echo :Mach-O 'fat:M::\\xca\\xfe\\xba\\xbe::$ld_mac:' > /proc/sys/fs/binfmt_misc/register
"
        echo "binfmt_misc for mach-o was registered"
    ;;
    stop)
        sudo sh -c '
echo -1 > /proc/sys/fs/binfmt_misc/Mach-O\ 64bit
echo -1 > /proc/sys/fs/binfmt_misc/Mach-O\ 32bit
echo -1 > /proc/sys/fs/binfmt_misc/Mach-O\ fat
'
        echo "binfmt_misc for mach-o was unregistered"
        ;;
    *)
        echo "unknown command: $1"
        exit 1
        ;;
esac
