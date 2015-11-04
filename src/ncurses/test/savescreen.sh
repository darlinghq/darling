#!/bin/sh
##############################################################################
# Copyright (c) 2007 Free Software Foundation, Inc.                          #
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
# $Id: savescreen.sh,v 1.3 2007/07/14 21:50:26 tom Exp $
#
# Use this script to exercise "savescreen".
# It starts by generating a series of temporary-filenames, which are passed
# to the test-program.  Loop as long as the first file named exists.
PARAMS=
NFILES=4
PREFIX=savescreen-$$
n=0
BEGINS=$PREFIX-$n.tmp
while test $n != $NFILES
do
	LATEST=$PREFIX-$n.tmp
	PARAMS="$PARAMS $LATEST"
	n=`expr $n + 1`
done

./savescreen $PARAMS
if test -f $BEGINS
then
	while test -f $BEGINS
	do
		./savescreen -r $PARAMS
	done
else
	echo "No screens were saved"
fi
