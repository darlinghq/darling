#!/bin/sh
##############################################################################
# Copyright (c) 2003,2006 Free Software Foundation, Inc.                     #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
# $Id: listused.sh,v 1.7 2006/06/03 16:39:37 tom Exp $
# A very simple script to list entrypoints that are used by either a test
# program, or within the libraries.  This relies on the output format of 'nm',
# and assumes that the libraries are configured with TRACE defined, and using
# these options:
#	--disable-macros
#	--enable-widec
# Static libraries are used, to provide some filtering based on internal usage
# of the different symbols.

# keep the sorting independent of locale:
if test "${LANGUAGE+set}"    = set; then LANGUAGE=C;    export LANGUAGE;    fi
if test "${LANG+set}"        = set; then LANG=C;        export LANG;        fi
if test "${LC_ALL+set}"      = set; then LC_ALL=C;      export LC_ALL;      fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi
if test "${LC_COLLATE+set}"  = set; then LC_COLLATE=C;  export LC_COLLATE;  fi

NM_OPTS=

if test ! -d ../objects ; then
	echo "? need objects to run this script"
	exit 1
elif test ! -d ../lib ; then
	echo "? need libraries to run this script"
	exit 1
fi

PROGS=
for name in `(echo "test:";sort modules; echo "progs:";sort ../progs/modules) |sed -e 's/[ 	].*//' -e '/^[#@]/d'`
do
	case $name in
	*:)
		PROGS="$PROGS $name"
		;;
	*)
		NAME=../objects/${name}.o
		if test -f $NAME
		then
			PROGS="$PROGS $NAME"
		fi
		;;
	esac
done

# For each library -
for lib in ../lib/*.a
do
	LIB=`basename $lib .a`
	case $LIB in
	*_*|*+*)
		continue
		;;
	esac

	tmp=`echo $LIB|sed -e 's/w$//'`
	echo
	echo "${tmp}:"
	echo $tmp |sed -e 's/./-/g'
	# Construct a list of public externals provided by the library.
	WANT=`nm $NM_OPTS $lib |\
		sed	-e 's/^[^ ]*//' \
			-e 's/^ *//' \
			-e '/^[ a-z] /d' \
			-e '/:$/d' \
			-e '/^$/d' \
			-e '/^U /d' \
			-e 's/^[A-Z] //' \
			-e '/^_/d' |\
		sort -u`
	# List programs which use that external.
	for name in $WANT
	do
		HAVE=
		tags=
		last=
		for prog in $PROGS
		do
			case $prog in
			*:)
				tags=$prog
				;;
			*)
				TEST=`nm $NM_OPTS $prog |\
					sed	-e 's/^[^ ]*//' \
						-e 's/^ *//' \
						-e '/^[ a-z] /d' \
						-e '/:$/d' \
						-e '/^$/d' \
						-e 's/^[A-Z] //' \
						-e '/^_/d' \
						-e 's/^'${name}'$/_/' \
						-e '/^[^_]/d'`
				if test -n "$TEST"
				then
					have=`basename $prog .o`
					if test -n "$HAVE"
					then
						if test "$last" = "$tags"
						then
							HAVE="$HAVE $have"
						else
							HAVE="$HAVE $tags $have"
						fi
					else
						HAVE="$tags $have"
					fi
					last="$tags"
				fi
				;;
			esac
		done
		# if we did not find a program using it directly, see if it
		# is used within a library.
		if test -z "$HAVE"
		then
			for tmp in ../lib/*.a
			do 
				case $tmp in
				*_*|*+*)
					continue
					;;
				esac
				TEST=`nm $NM_OPTS $tmp |\
					sed	-e 's/^[^ ]*//' \
						-e 's/^ *//' \
						-e '/^[ a-z] /d' \
						-e '/:$/d' \
						-e '/^$/d' \
						-e '/^[A-TV-Z] /d' \
						-e 's/^[A-Z] //' \
						-e '/^_/d' \
						-e 's/^'${name}'$/_/' \
						-e '/^[^_]/d'`
				if test -n "$TEST"
				then
					tmp=`basename $tmp .a |sed -e 's/w$//'`
					HAVE=`echo $tmp | sed -e 's/lib/lib: /'`
					break
				fi
			done
		fi
		test -z "$HAVE" && HAVE="-"
		lenn=`expr 39 - length $name`
		lenn=`expr $lenn / 8`
		tabs=
		while test $lenn != 0
		do
			tabs="${tabs}	"
			lenn=`expr $lenn - 1`
		done
		echo "${name}${tabs}${HAVE}"
	done
done
