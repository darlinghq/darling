#!/bin/sh
#
# Copyright (c) 1999-2008 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
#
# "Portions Copyright (c) 1999, 2008 Apple Inc.  All Rights
# Reserved.  This file contains Original Code and/or Modifications of
# Original Code as defined in and that are subject to the Apple Public
# Source License Version 1.0 (the 'License').  You may not use this file
# except in compliance with the License.  Please obtain a copy of the
# License at http://www.apple.com/publicsource and read it before using
# this file.
#
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
# License for the specific language governing rights and limitations
# under the License."
#
# @APPLE_LICENSE_HEADER_END@
#
# Mach Operating System
# Copyright (c) 1991,1990 Carnegie Mellon University
# All Rights Reserved.
#
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
#
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#
# Carnegie Mellon requests users of this software to return to
#
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
#
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#

realpath()
{
	local FILE="$1"
	local PARENT=$(dirname "$FILE")
	local BASE=$(basename "$FILE")
	pushd "$PARENT" >/dev/null 2>&1 || return 0
	local DIR=$(pwd -P)
	popd >/dev/null
	if [ "$DIR" == "/" ]; then
		echo "/$BASE"
	else
		echo "$DIR/$BASE"
	fi
	return 1
}

scriptPath=$(realpath "$0")
scriptRoot=$(dirname "$scriptPath")
migcomPath=$(realpath "${scriptRoot}/../libexec/migcom")

if [ -n "${SDKROOT}" ]; then
	sdkRoot="${SDKROOT}";
fi

if [ -z "${MIGCC}" ]; then
  xcrunPath="/usr/bin/xcrun"
  if [ -x "${xcrunPath}" ]; then
    MIGCC=`"${xcrunPath}" -sdk "$sdkRoot" -find cc`
  else
    MIGCC=$(realpath "${scriptRoot}/cc")
  fi
fi

C=${MIGCC}
M=${MIGCOM-${migcomPath}}

cppflags="-D__MACH30__"

files=
arch=`/usr/bin/arch`

WORKTMP=`/usr/bin/mktemp -d "${TMPDIR:-/tmp}/mig.XXXXXX"`
if [ $? -ne 0 ]; then
      echo "Failure creating temporary work directory: ${WORKTMP}"
      echo "Exiting..."
      exit 1
fi

# parse out the arguments until we hit plain file name(s)

until [ $# -eq 0 ]
do
    case "$1" in
	-[dtqkKQvVtTrRsSlLxXnN] ) migflags=( "${migflags[@]}" "$1" ); shift;;
	-i	) sawI=1; migflags=( "${migflags[@]}" "$1" "$2" ); shift; shift;;
	-user   )  user="$2"; if [ ! "${sawI-}" ]; then migflags=( "${migflags[@]}" "$1" "$2" ); fi; shift; shift;;
	-server )  server="$2";  migflags=( "${migflags[@]}" "$1" "$2" ); shift; shift;;
	-header )  header="$2";  migflags=( "${migflags[@]}" "$1" "$2"); shift; shift;;
	-sheader ) sheader="$2"; migflags=( "${migflags[@]}" "$1" "$2"); shift; shift;;
	-iheader ) iheader="$2"; migflags=( "${migflags[@]}" "$1" "$2"); shift; shift;;
	-dheader ) dheader="$2"; migflags=( "${migflags[@]}" "$1" "$2"); shift; shift;;
	-arch ) arch="$2"; shift; shift;;
	-maxonstack ) migflags=( "${migflags[@]}" "$1" "$2"); shift; shift;;
	-split ) migflags=( "${migflags[@]}" "$1" ); shift;;
	-novouchers ) migflags=( "${migflags[@]}" "$1" ); shift;;
	-MD ) sawMD=1; cppflags=( "${cppflags[@]}" "$1"); shift;;
	-cpp) shift; shift;;
	-cc) C="$2"; shift; shift;;
	-migcom) M="$2"; shift; shift;;
	-isysroot) sdkRoot=$(realpath "$2"); shift; shift;;
	-* ) cppflags=( "${cppflags[@]}" "$1"); shift;;
	* ) break;;
    esac
done

# process the rest as files
until [ $# -eq 0 ]
do
    case "$1" in
	-[dtqkKQvVtTrRsSlLxXnN] ) echo "warning: option \"$1\" after filename(s) ignored"; shift; continue;;
	-i	) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-user   ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-server ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-header ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-sheader ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-iheader ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-dheader ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-arch ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift ; shift; continue;;
	-maxonstack ) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-split ) echo "warning: option \"$1\" after filename(s) ignored"; shift; continue;;
	-novouchers ) echo "warning: option \"$1\" after filename(s) ignored"; shift; continue;;
	-MD ) echo "warning: option \"$1\" after filename(s) ignored"; shift; continue;;
	-cpp) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-cc) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-migcom) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-isysroot) echo "warning: option \"$1 $2\" after filename(s) ignored"; shift; shift; continue;;
	-* ) echo "warning: option \"$1\" after filename(s) ignored"; shift; continue;;
        * ) file="$1"; shift;;
    esac
    base="$(basename "${file}" .defs)"
    temp="${WORKTMP}/${base}.$$"
    sourcedir="$(dirname "${file}")"
    if [ -n "${sdkRoot}" ]
    then
      iSysRootParm=( "-isysroot" "${sdkRoot}" )
    fi
    if [ ! -r "${file}" ]
    then
      echo "error: cannot read file ${file}"
      rm -rf ${WORKTMP}
      exit 1
    fi
    rm -f "${temp}.c" "${temp}.d"
    (echo '#line 1 '\"${file}\" ; cat "${file}" ) > "${temp}.c"
    "$C" -E -arch ${arch} "${cppflags[@]}" -I "${sourcedir}" "${iSysRootParm[@]}" "${temp}.c" | "$M" "${migflags[@]}"
    if [ $? -ne 0 ]
    then
      rm -rf "${temp}.c" "${temp}.d" "${WORKTMP}"
      exit 1
    fi
    if [ "${sawMD}" -a -f "${temp}.d" ]
    then
	deps=
	s=
	rheader="${header-${base}.h}"
	if [ "${rheader}" != /dev/null ]; then
		deps="${deps}${s}${rheader}"; s=" "
	fi
	ruser="${user-${base}User.c}"
	if [ "${ruser}" != /dev/null ]; then
		deps="${deps}${s}${ruser}"; s=" "
	fi
	rserver="${server-${base}Server.c}"
	if [ "${rserver}" != /dev/null ]; then
		deps="${deps}${s}${rserver}"; s=" "
	fi
	rsheader="${sheader-/dev/null}"
	if [ "${rsheader}" != /dev/null ]; then
		deps="${deps}${s}${rsheader}"; s=" "
	fi
	riheader="${iheader-/dev/null}"
	if [ "${riheader}" != /dev/null ]; then
		deps="${deps}${s}${riheader}"; s=" "
	fi
	rdheader="${dheader-/dev/null}"
	if [ "${rdheader}" != /dev/null ]; then
		deps="${deps}${s}${rdheader}"; s=" "
	fi
	for target in "${deps}"
	do
		sed -e 's;^'"${temp}"'.o[ 	]*:;'"${target}"':;' \
		    -e 's;: '"${temp}"'.c;: '"$file"';' \
		< "${temp}.d" > "${target}.d"
	done
	rm -f "${temp}.d"
    fi
    rm -f "${temp}.c"
done

/bin/rmdir "${WORKTMP}"
exit 0

