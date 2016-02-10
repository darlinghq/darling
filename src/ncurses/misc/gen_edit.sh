#!/bin/sh
##############################################################################
# Copyright (c) 2004 Free Software Foundation, Inc.                          #
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
# $Id: gen_edit.sh,v 1.1 2004/07/11 15:01:29 tom Exp $
# Generate a sed-script for converting the terminfo.src to the form which will
# be installed.
#
# Assumes:
#	The leaf directory names (lib, tabset, terminfo)
#

: ${ticdir=@TERMINFO@}
: ${xterm_new=@WHICH_XTERM@}

# If we're not installing into /usr/share/, we'll have to adjust the location
# of the tabset files in terminfo.src (which are in a parallel directory).
TABSET=`echo $ticdir | sed -e 's%/terminfo$%/tabset%'`
if test "x$TABSET" != "x/usr/share/tabset" ; then
cat <<EOF
s%/usr/share/tabset%$TABSET%g
EOF
fi

if test "$xterm_new" != "xterm-new" ; then
cat <<EOF
/^# This is xterm for ncurses/,/^$/{
	s/use=xterm-new,/use=$WHICH_XTERM,/
}
EOF
fi
