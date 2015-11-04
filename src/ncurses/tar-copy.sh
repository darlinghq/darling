#!/bin/sh
# $Id: tar-copy.sh,v 1.5 2003/10/25 14:40:07 tom Exp $
##############################################################################
# Copyright (c) 1998,2003 Free Software Foundation, Inc.                     #
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
#
# Author: Thomas E. Dickey
#
# Copy a collection of files using 'tar', so that their dates and links are
# preserved
#
# Parameters:
#	$1 = files to copy
#	$2 = source directory
#	$3 = destination directory
#
#DOIT=echo
DOIT=eval

if test $# != 3 ; then
	echo "Usage: $0 files source target"
	exit 1
elif test ! -d "$2" ; then
	echo "Source directory not found: $2"
	exit 1
elif test ! -d "$3" ; then
	echo "Target directory not found: $3"
	exit 1
fi

WD=`pwd`

TMP=$WD/copy$$

cd $2
TEST=`ls -d $1 2>/dev/null`
if test -z "$TEST"
then
	echo "... no match for \"$1\" in $2"
else
	echo "... installing files matching \"$1\" in $2"
	trap "rm -f $TMP" 0 1 2 5 15
	if ( tar cf $TMP $1 )
	then
		cd $3
		LIST=`tar tf $TMP 2>&1`
		$DOIT rm -rf $LIST 2>/dev/null
		$DOIT tar xvf $TMP
	else
		echo "Cannot create tar of $1 files"
		exit 1
	fi
fi
