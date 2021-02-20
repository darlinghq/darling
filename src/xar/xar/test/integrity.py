#!/usr/bin/env python

from __future__ import print_function

import os
import os.path
import subprocess

import util

# 
# Utility Functions
# 

def _test_truncation(filename, path_to_be_archived, bytes_to_chop, *args):
	with util.archive_created(filename, path_to_be_archived) as path:
		with open("/dev/null", "w") as bitbucket:
			size = os.stat(path).st_size
			while size > 0:
				last_size = size
				size = max(0, size - bytes_to_chop)
				with open(path, "w+") as f:
					f.truncate(size)
			
				with util.directory_created("scratch") as directory:
					returncode = subprocess.call(["xar", "-x", "-f", path, "-C", directory], stderr=bitbucket)
					assert returncode != 0, "xar claimed to succeed when extracting a truncated archive"

# 
# Test Cases
#

def large_uncompressed(filename):
	_test_truncation(filename, "/usr/share/man/man1", 1024 * 1024, "--compression=none")
	
def large_default_compression(filename):
	_test_truncation(filename, "/usr/share/man/man1", 1024 * 1024)

def large_gzip_compressed(filename):
	util.skip_if_no_compression_support("gzip")
	_test_truncation(filename, "/usr/share/man/man1", 1024 * 1024, "--compression=gzip")

def large_bzip2_compressed(filename):
	util.skip_if_no_compression_support("bzip2")
	_test_truncation(filename, "/usr/share/man/man1", 1024 * 1024, "--compression=bzip2")

def large_lzma_compressed(filename):
	util.skip_if_no_compression_support("lzma")
	_test_truncation(filename, "/usr/share/man/man1", 1024 * 1024, "--compression=lzma")

# "small" variants use a non-base-2 size to try to catch issues that occur on uneven boundaries

def small_uncompressed(filename):
	_test_truncation(filename, "/bin", 43651, "--compression=none")

def small_default_compression(filename):
	_test_truncation(filename, "/bin", 43651)

def small_gzip_compressed(filename):
	util.skip_if_no_compression_support("gzip")
	_test_truncation(filename, "/bin", 43651, "--compression=gzip")

def small_bzip2_compressed(filename):
	util.skip_if_no_compression_support("bzip2")
	_test_truncation(filename, "/bin", 43651, "--compression=bzip2")

def small_lzma_compressed(filename):
	util.skip_if_no_compression_support("lzma")
	_test_truncation(filename, "/bin", 43651, "--compression=lzma")


TEST_CASES = (large_uncompressed, large_default_compression,
              large_gzip_compressed, large_bzip2_compressed, large_lzma_compressed,
              small_uncompressed, small_default_compression,
              small_gzip_compressed, small_bzip2_compressed, small_lzma_compressed)

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
