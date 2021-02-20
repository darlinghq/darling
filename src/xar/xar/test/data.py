#!/usr/bin/env python

from __future__ import print_function

import contextlib
import os
import os.path
import subprocess
import util


# 
# Utility Functions
# 

@contextlib.contextmanager
def _data_test(filename, *args, **kwargs):
	with util.directory_created("data_scratch") as directory:
		yield directory
		# Files are now created
		with util.archive_created("data.xar", "./data_scratch", *args, **kwargs) as path:
			_process_toc(path)
			with util.directory_created("data_extracted") as extracted:
				subprocess.check_call(["xar", "-x", "-f", path, "-C", extracted])
				util.assert_identical_directories(directory, os.path.join(extracted, "data_scratch"))

def _process_toc(archive_path):
	subprocess.check_call(["xar", "-f", archive_path, "--dump-toc=data_toc.xml"])
	try:
		result = subprocess.check_output(["xsltproc", "-o", "-", os.path.realpath(os.path.join(__file__, "..", "data.xsl")), "data_toc.xml"])
		assert result == "", "expected no data offset, but instead found:{o}".format(o=result)
	finally:
		os.unlink("data_toc.xml")

def _zero_length(filename, *args, **kwargs):
	with _data_test(filename, *args, **kwargs) as directory:
		util.touch(os.path.join(directory, "empty"))


# 
# Test Cases
# 

def zero_length_default_compression(filename):
	_zero_length(filename)

def zero_length_no_compression(filename):
	_zero_length(filename, "--compression=none")

def zero_length_gzip_compression(filename):
	util.skip_if_no_compression_support("gzip")
	_zero_length(filename, "--compression=gzip")

def zero_length_bzip2_compression(filename):
	util.skip_if_no_compression_support("bzip2")
	_zero_length(filename, "--compression=bzip2")

def zero_length_lzma_compression(filename):
	util.skip_if_no_compression_support("lzma")
	_zero_length(filename, "--compression=lzma")

def _mixed_length(filename, *args, **kwargs):
	with _data_test(filename, *args, **kwargs) as directory:
		util.touch(os.path.join(directory, "mixed_empty"))
		with open(os.path.join(directory, "mixed_small"), "w") as f:
			f.write("1234")

def mixed_length_no_compression(filename):
	_mixed_length(filename, "--compression=none")

def mixed_length_default_compression(filename):
	_mixed_length(filename)

def mixed_length_gzip_compression(filename):
	util.skip_if_no_compression_support("gzip")
	_mixed_length(filename, "--compression=gzip")

def mixed_length_bzip2_compression(filename):
	util.skip_if_no_compression_support("bzip2")
	_mixed_length(filename, "--compression=bzip2")

def mixed_length_lzma_compression(filename):
	util.skip_if_no_compression_support("lzma")
	_mixed_length(filename, "--compression=lzma")


TEST_CASES = (zero_length_default_compression, zero_length_no_compression,
              zero_length_gzip_compression, zero_length_bzip2_compression, zero_length_lzma_compression,
              mixed_length_no_compression, mixed_length_default_compression,
              mixed_length_gzip_compression, mixed_length_bzip2_compression, mixed_length_lzma_compression)

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
