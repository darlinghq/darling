#! /bin/bash -
#
# Copyright (c) 2010 Apple Inc. All rights reserved.
#
# @APPLE_OSREFERENCE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_OSREFERENCE_LICENSE_HEADER_END@
#

function usage() {
    echo "Usage: $0 <sdk> <output>" 1>&2
    exit 1
}

if [ $# -ne 2 ]; then
    usage
fi

SDKROOT="$1"
OUTPUT="$2"

if [ ! -x "${SDKROOT}/usr/local/libexec/availability.pl" ] ; then
    echo "Unable to locate ${SDKROOT}/usr/local/libexec/availability.pl (or not executable)" >&2
    exit 1
fi
	    
{
cat <<EOF
/* Copyright (c) 2010 Apple Inc. All rights reserved.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _CDEFS_H_
# error "Never use <sys/_symbol_aliasing.h> directly.  Use <sys/cdefs.h> instead."
#endif

EOF

for ver in $(${SDKROOT}/usr/local/libexec/availability.pl --ios) ; do
    ver_major=${ver%.*}
    ver_minor=${ver#*.}
    value=$(printf "%d%02d00" ${ver_major} ${ver_minor})
    str=$(printf "__IPHONE_%d_%d" ${ver_major} ${ver_minor})
    echo "#if defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= ${value}"
    echo "#define __DARWIN_ALIAS_STARTING_IPHONE_${str}(x) x"
    echo "#else"
    echo "#define __DARWIN_ALIAS_STARTING_IPHONE_${str}(x)"
    echo "#endif"
    echo ""
done

for ver in $(${SDKROOT}/usr/local/libexec/availability.pl --macosx) ; do
    ver_major=${ver%.*}
    ver_minor=${ver#*.}
    value=$(printf "%d%d0" ${ver_major} ${ver_minor})
    str=$(printf "__MAC_%d_%d" ${ver_major} ${ver_minor})
    echo "#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= ${value}"
    echo "#define __DARWIN_ALIAS_STARTING_MAC_${str}(x) x"
    echo "#else"
    echo "#define __DARWIN_ALIAS_STARTING_MAC_${str}(x)"
    echo "#endif"
    echo ""
done
} > "$OUTPUT"

