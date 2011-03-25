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
            pipe.write("%s\n" % m.group(1))
        else:
            pipe.write("0x%x\n" % frame.pc())
        frame = frame.older()
        i += 1

    pipe.close()
