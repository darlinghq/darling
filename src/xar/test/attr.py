#!/usr/bin/env python

from __future__ import print_function

import os
import os.path
import shutil
import subprocess
import xattr

import util

# Notes:
# * Due to internal buffer sizing, "large" should be at least 4096 bytes for
#   uncompressed archives and archives with individual file compression, and
#   and at least 32768 bytes for archives with compressed heaps.
# * For large files, we intentionally use a non-base-2 file size to ensure we
#   catch file extraction bugs for files with a size not equal to the internal
#   read buffer size (which is base-2).
# * For large extended attributes, we do the same thing.

# 
# Utility Functions
# 

class MissingExtendedAttributeError(AssertionError):
	pass

def _random_big_data(bytes=65536, path="/dev/random"):
	"""
	Returns a random string with the number of bytes requested. Due to xar
	implementation details, this should be greater than 4096 (32768 for
	compressed heap testing).
	
	"""
	with open(path, "r") as f:
		return f.read(bytes)

def _test_xattr_on_file_with_contents(filename, file_contents, xattrs=[], xar_create_flags=[], xar_extract_flags=[]):
	try:
		# Write file out
		with open(filename, "w") as f:
			f.write(file_contents)
			for (key, value) in xattrs:
				xattr.setxattr(f, key, value)
		
		# Store it into a xarchive
		archive_name = "{f}.xar".format(f=filename)
		with util.archive_created(archive_name, filename, *xar_create_flags) as path:
			# Extract xarchive
			with util.directory_created("extracted") as directory:
				# Validate resulting xattrs
				subprocess.check_call(["xar", "-x", "-C", directory, "-f", path] + xar_extract_flags)
				for (key, value) in xattrs:
					try:
						assert xattr.getxattr(os.path.join(directory, filename), key) == value, "extended attribute \"{n}\" has incorrect contents after extraction".format(n=key)
					except KeyError:
						raise MissingExtendedAttributeError("extended attribute \"{n}\" missing after extraction".format(n=key))
				
				# Validate file contents
				with open(os.path.join(directory, filename), "r") as f:
					if f.read() != file_contents:
						raise MissingExtendedAttributeError("archived file \"{f}\" has has incorrect contents after extraction".format(f=filename))
	finally:
		os.unlink(filename)


# 
# Test Cases
# 

# Note: xar currently drops empty extended attributes (and any xar_prop_t with empty contents, actually). The empty
#       tests are commented out awaiting a day when this might be different.

# def empty_xattr_empty_file(filename):
# 	_test_xattr_on_file_with_contents(filename, "", xattrs=[("foo", "")])

def small_xattr_empty_file(filename):
	_test_xattr_on_file_with_contents(filename, "", xattrs=[("foo", "1234")])

def large_xattr_empty_file(filename):
	_test_xattr_on_file_with_contents(filename, "", xattrs=[("foo", _random_big_data(5000))])

# def empty_xattr_small_file(filename):
# 	_test_xattr_on_file_with_contents(filename, "small.file.contents", xattrs=[("foo", "")])

def small_xattr_small_file(filename):
	_test_xattr_on_file_with_contents(filename, "small.file.contents", xattrs=[("foo", "1234")])

def large_xattr_small_file(filename):
	_test_xattr_on_file_with_contents(filename, "small.file.contents", xattrs=[("foo", _random_big_data(4567))])

# def empty_xattr_large_file(filename):
# 	_test_xattr_on_file_with_contents(filename, _random_big_data(10000000), xattrs=[("foo", "")])

def small_xattr_large_file(filename):
	_test_xattr_on_file_with_contents(filename, _random_big_data(5000000), xattrs=[("foo", "1234")])

def large_xattr_large_file(filename):
	_test_xattr_on_file_with_contents(filename, _random_big_data(9876543), xattrs=[("foo", _random_big_data(6543))])

def multiple_xattrs(filename):
	_test_xattr_on_file_with_contents(filename, "", xattrs=[("foo", "bar"), ("baz", "1234"), ("quux", "more")]) # ("empty", "")

def distribution_create(filename):
	try:
		_test_xattr_on_file_with_contents(filename, "dummy", xattrs=[("foo", "bar")], xar_create_flags=["--distribution"])
	except MissingExtendedAttributeError:
		pass
	else:
		raise AssertionError("no error encountered")

# Note: xar currently has no way to exclude a property on extraction. This test is commented out awaiting the day
#       when it can.

# def distribution_extract(filename):
# 	try:
# 		_test_xattr_on_file_with_contents(filename, "dummy", xattrs=[("foo", "bar")], xar_extract_flags=["--distribution"])
# 	except MissingExtendedAttributeError:
# 		pass
# 	else:
# 		raise AssertionError("no error encountered")


TEST_CASES = (small_xattr_empty_file, large_xattr_empty_file,
              small_xattr_small_file, large_xattr_small_file,
              small_xattr_large_file, large_xattr_large_file,
              multiple_xattrs, distribution_create)

if __name__ == "__main__":
	for case in TEST_CASES:
		try:
			case(case.func_name)
			print("PASSED: {f}".format(f=case.func_name))
		except (AssertionError, IOError, subprocess.CalledProcessError):
			import sys, os
			print("FAILED: {f}".format(f=case.func_name))
			sys.excepthook(*sys.exc_info())
			print("")
