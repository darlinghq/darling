#!/usr/bin/env python

from __future__ import print_function

import glob
import os
import os.path
import re
import shutil
import subprocess

import util


# 
# Utility Functions
# 

def _file_offsets_for_archive(path, xsl_path):
	subprocess.check_call(["xar", "--dump-toc=heap_toc.xml", "-f", path])
	try:
		offsets = subprocess.check_output(["xsltproc", xsl_path, "heap_toc.xml"])
		matches = [re.match("^(.+)\s([^\s]+)$", offset) for offset in offsets.splitlines()]
		offsets = [(match.groups()[0], int(match.groups()[1])) for match in matches]
		return offsets
	finally:
		os.unlink("heap_toc.xml")

# 
# Test Cases
# 

XSL_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), "heap1.xsl")

def normal_heap(filename):
	with util.directory_created("scratch") as directory:
		shutil.copy("/bin/ls", os.path.join(directory, "ls"))
		shutil.copy(os.path.join(directory, "ls"), os.path.join(directory, "foo"))
		with util.chdir(directory):
			with util.archive_created(os.path.join("..", "heap.xar"), ".") as path:
				# Verify file offsets are as we expect
				offsets = _file_offsets_for_archive(path, XSL_PATH)
				(f1, o1) = offsets[0]
				(f2, o2) = offsets[1]
				
				assert o1 < o2, "offset for first file \"{f1}\" ({o1}) greater than or equal to offset for last file \"{f2}\" ({o2})".format(f1=f1, o1=o1, f2=f2, o2=o2)
				
				# Make sure extraction goes all right
				with util.directory_created("extracted") as extracted:
					subprocess.check_call(["xar", "-x", "-f", path, "-C", extracted])

def coalesce_heap(filename):
	with util.directory_created("scratch") as directory:
		shutil.copy("/bin/ls", os.path.join(directory, "ls"))
		shutil.copy(os.path.join(directory, "ls"), os.path.join(directory, "foo"))
		with util.chdir(directory):
			with util.archive_created(os.path.join("..", "heap.xar"), ".", "--coalesce-heap") as path:
				# Verify file offsets are as we expect
				offsets = _file_offsets_for_archive(path, XSL_PATH)
				(f1, o1) = offsets[0]
				(f2, o2) = offsets[1]
				
				# Make sure extraction goes all right
				with util.directory_created("extracted") as extracted:
					subprocess.check_call(["xar", "-x", "-f", path, "-C", extracted])

TEST_CASES = (normal_heap, coalesce_heap)

if __name__ == "__main__":
	for case in TEST_CASES:
		try:
			case("{f}.xar".format(f=case.func_name))
			print("PASSED: {f}".format(f=case.func_name))
		except (AssertionError, IOError, subprocess.CalledProcessError):
			import sys, os
			print("FAILED: {f}".format(f=case.func_name))
			sys.excepthook(*sys.exc_info())
			print("")
