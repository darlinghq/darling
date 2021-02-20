#!/usr/bin/env python

from __future__ import print_function

import os
import os.path
import subprocess

import util

# 
# Utility Functions
# 

def _assert_same_inodes(path1, path2):
	assert os.path.exists(path1), "missing hard-linked-file {p}".format(p=path1)
	assert os.path.exists(path1), "missing hard-linked-file {p}".format(p=path2)
	stat1 = os.lstat(path1)
	stat2 = os.lstat(path2)
	assert stat1.st_ino == stat2.st_ino, "inodes do not match for hard-linked files \"{p1}\" and \"{p2}\"".format(p1=path1, p2=path2)


# 
# Test Cases
# 

def hard_link_in_directory(filename):
	with util.directory_created("hardlink_scratch") as directory:
		util.touch(os.path.join(directory, "a"))
		os.link(os.path.join(directory, "a"), os.path.join(directory, "b"))
		
		with util.archive_created("hardlink_archive.xar", "hardlink_scratch") as path:
			with util.directory_created("hardlink_extracted") as extracted:
				subprocess.check_call(["xar", "-x", "-C", extracted, "-f", path])
				_assert_same_inodes(os.path.join(extracted, "hardlink_scratch", "a"), os.path.join(extracted, "hardlink_scratch", "b"))

def hard_link_in_cwd(filename):
	with util.directory_created("hardlink_scratch") as directory:
		with util.chdir(directory):
			util.touch("a")
			os.link("a", "b")
			with util.archive_created(os.path.join("..", "hardlink_archive.xar"), ".") as path:
				with util.directory_created(os.path.join("..", "hardlink_extracted")) as extracted:
					subprocess.check_call(["xar", "-x", "-C", extracted, "-f", path])
					_assert_same_inodes(os.path.join(extracted, "a"), os.path.join(extracted, "b"))

def hard_link_identical_files(filename):
	with util.directory_created("hardlink_scratch") as directory:
		with open(os.path.join(directory, "a"), "w") as f:
			f.write("1234samecontent")
		with open(os.path.join(directory, "b"), "w") as f:
			f.write("1234samecontent")
		with util.archive_created("hardlink_archive.xar", "hardlink_scratch", "--link-same") as path:
			with util.directory_created("hardlink_extracted") as extracted:
				subprocess.check_call(["xar", "-x", "-C", extracted, "-f", path])
				_assert_same_inodes(os.path.join(extracted, "hardlink_scratch", "a"), os.path.join(extracted, "hardlink_scratch", "b"))

TEST_CASES = (hard_link_in_directory, hard_link_in_cwd, hard_link_identical_files)

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
