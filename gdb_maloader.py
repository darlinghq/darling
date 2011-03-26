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

import gdb
import os
import re
import sys

def bt(demangle=True):
    # Find the newest frame.
    frame = gdb.selected_frame()
    while True:
        next = frame.newer()
        if not next:
            break
        frame = next

    if demangle:
        pipe = os.popen('c++filt', 'w')
    else:
        pipe = sys.stdout

    i = 0
    while frame:
        s = gdb.execute('p dumpSymbol((void*)0x%x)' % frame.pc(),
                        to_string=True)
        m = re.match(r'.*"(.*)"$', s)
        if m:
            pipe.write("#%-2d %s\n" % (i, m.group(1)))
        else:
            sal = frame.find_sal()
            lineno = ''
            if sal.symtab:
                lineno = ' at %s:%d' % (sal.symtab, sal.line)
            pipe.write("#%-2d 0x%x %s%s\n" %
                       (i, frame.pc(), frame.name(), lineno))
        frame = frame.older()
        i += 1

    pipe.close()
