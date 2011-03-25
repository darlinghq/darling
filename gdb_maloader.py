import gdb
import os
import re
import sys

def bt():
    # Find the newest frame.
    frame = gdb.selected_frame()
    while True:
        next = frame.newer()
        if not next:
            break
        frame = next

    pipe = os.popen('c++filt', 'w')

    i = 0
    while frame:
        s = gdb.execute('p dumpExportedSymbol((void*)0x%x)' % frame.pc(),
                        to_string=True)
        m = re.match(r'.*"(.*)"$', s)
        if m:
            pipe.write("#%-2d %s\n" % (i, m.group(1)))
        else:
            pipe.write("#%-2d 0x%x %s\n" % (i, frame.pc(), frame.function()))
        frame = frame.older()
        i += 1

    pipe.close()
