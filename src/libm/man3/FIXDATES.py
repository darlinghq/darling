#!/usr/bin/env python3

# Always check in results with a message that includes the tag [ignoreRevisionForDatingFiles]
# so that this tool will ignore its own updates

import subprocess
import sys
import time

for filename in sys.argv[1:]:
    print("$ svn log --limit 1 " + filename)
    svnlog = subprocess.getoutput("svn log --limit 1 " + filename)
    print(svnlog)
    loglines = svnlog.split("\n")
    if len(loglines) < 2:
        continue

    if loglines[3].find("[ignoreRevisionForDatingFiles]") != -1:
        continue

    infoline = loglines[1].split()
    datestring = time.strftime("%B %d, %Y", time.strptime(infoline[4], "%Y-%m-%d"))
    editor = infoline[2]
    version = infoline[0]
    edittime = ' '.join(infoline[4:6])

    infile = open(filename, "r")
    lines = infile.readlines()
    for i in range(len(lines)):
        if (lines[i][:3] == ".Dd"):
            lines[i] = ".Dd " + datestring + "\n"
    infile.close()

    outfile = open(filename, "w")
    outfile.writelines(lines)
    outfile.close()
