#!/bin/sh

#  force_libc_to_build.sh
#  Libc
#
#  Created by Matt Wright on 10/4/11.
#  Copyright (c) 2011 None. All rights reserved.

if [ "x$ACTION" == "xinstallhdrs" ]; then exit 0; fi

DEPS_C="$BUILT_PRODUCTS_DIR/deps.c"

[ -e "$DEPS_C" ] && rm -f "$DEPS_C"
