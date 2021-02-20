#!/usr/bin/env python

from __future__ import print_function

import cStringIO
import os
import os.path
import subprocess
import tempfile

import util


# 
# Utility Functions
# 

def _check_compression(filename, *args, **kwargs):
	with util.archive_created(filename, "/bin", *args, **kwargs) as path:
		with util.directory_created("extracted") as directory:
			subprocess.check_call(["xar", "-x", "-f", path, "-C", directory])
			util.assert_identical_directories("/bin", os.path.join(directory, "bin"))


# 
# Test Cases
# 

def no_compression(filename):
	_check_compression(filename, "--compression=none")

def default_compression(filename):
	_check_compression(filename)

def gzip_compression_long(filename):
	util.skip_if_no_compression_support("gzip")
	_check_compression(filename, "--compression=gzip")

def gzip_compression_short(filename):
	util.skip_if_no_compression_support("gzip")
	_check_compression(filename, "-z")

def bzip2_compression_long(filename):
	util.skip_if_no_compression_support("bzip2")
	_check_compression(filename, "--compression=bzip2")

def bzip2_compression_short(filename):
	util.skip_if_no_compression_support("bzip2")
	_check_compression(filename, "-j")

def lzma_compression_long(filename):
	util.skip_if_no_compression_support("lzma")
	_check_compression(filename, "--compression=lzma")

def lzma_compression_short(filename):
	util.skip_if_no_compression_support("lzma")
	_check_compression(filename, "-a")

TEST_CASES = (no_compression, default_compression,
              gzip_compression_long, bzip2_compression_long, lzma_compression_long,
              gzip_compression_short, bzip2_compression_short, lzma_compression_short)

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
		except util.TestCaseSkipError, e:
			print("SKIPPED: {f}: {m}".format(f=case.func_name, m=e.message))
