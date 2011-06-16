#!/bin/sh

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
