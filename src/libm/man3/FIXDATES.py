#!/usr/bin/python

# Always check in results with a message that includes the tag [ignoreRevisionForDatingFiles]
# so that this tool will ignore its own updates

import commands
import sys
import time
import string

for filename in sys.argv[1:]:
	print "$ svn log --limit 1 " + filename
	svnlog = commands.getoutput("svn log --limit 1 " + filename)
	print svnlog
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
	
	infile = open(filename,"r")
	lines = infile.readlines()
	for i in range(len(lines)):
		if (lines[i][:3] == ".Dd"):
			lines[i] = ".Dd " + datestring + "\n"
	infile.close()
			
	outfile = open(filename,"w")
	outfile.writelines(lines)
	outfile.close()
	