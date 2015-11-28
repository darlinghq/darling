#!/usr/bin/env python

import contextlib
import hashlib
import os
import os.path
import shutil
import stat
import subprocess
import sys
import xattr

class TestCaseSkipError(Exception):
	pass

def skip_if_no_compression_support(type):
	"""
	Raises TestCaseSkipError if the type is "lzma" and the test is running on
	darwin (OS X). In the future, we should add a hidden debugging flag to xar
	to determine valid compression types. This will skip incorrectly if a
	custom xar is used on OS X, or if a custom xar on another platform is
	built without bzip2 or lzma.
	
	"""
	if sys.platform == "darwin" and type == "lzma":
		raise TestCaseSkipError("{t} support not compiled in".format(t=type))

@contextlib.contextmanager
def directory_created(directory_path):
	"""
	Creates the named directory and provides the path to the directory to the
	calling code. Automatically removes the directory when finished.
	
	Usage:
	with directory_created("foobar") as path:
		do_stuff_with_path
	
	"""
	os.mkdir(directory_path)
	try:
		yield os.path.realpath(directory_path)
	finally:
		if os.path.exists(directory_path):
			shutil.rmtree(directory_path)

@contextlib.contextmanager
def archive_created(archive_path, content_path, *extra_args, **extra_kwargs):
	"""
	Creates a named xar archive of the specified content path, returning the
	path to the archive. Automatically removes the archive when finished.
	
	Usage:
	with archive_created("/bin", "bin.xar") as path:
		do_stuff_with(path)
	
	"""
	cmd = ["xar", "-c", "-f", archive_path, content_path]
	if extra_args:
		cmd += list(extra_args)
	try:
		subprocess.check_call(cmd, **extra_kwargs)
		assert os.path.exists(archive_path), "failed to create archive \"{p}\" but xar did not report an error".format(p=archive_path)
		yield os.path.realpath(archive_path)
	finally:
		if os.path.exists(archive_path):
			os.unlink(archive_path)

HASH_CHUNK_SIZE = 32768

def _md5_path(path):
	with open(path, "r") as f:
		h = hashlib.md5()
		while True:
			last = f.read(HASH_CHUNK_SIZE)
			if not last:
				break
			h.update(last)
		return h.digest()

def assert_identical_directories(path1, path2):
	"""
	Verifies two directories have identical contents. Checks file type (via
	the high byte of the mode), size, atime, and mtime, but does not check
	other attributes like uid and gid, since they can be expected to change.
	
	"""
	seen = set([])
	for file1 in os.listdir(path1):
		seen.add(file1)
		entry1 = os.path.join(path1, file1)
		entry2 = os.path.join(path2, file1)
		assert os.path.exists(entry2), "\"{f1}\" exists in \"{p1}\" but not \"{p2}\"".format(f1=file1, p1=path1, p2=path2)
		
		# Extended attributes
		xattr1 = xattr.xattr(entry1)
		xattr2 = xattr.xattr(entry2)
		assert set(xattr1.list()) == set(xattr2.list()), "list of extended attributes on \"{f1}\" ({l1}) differs from \"{f2}\" ({l2})".format(f1=entry1, l1=xattr1.list(), f2=entry2, l2=xattr2.list())
		for attribute in xattr1.list():
			assert xattr1.get(attribute) == xattr2.get(attribute), "extended attribute \"{a1}\" on \"{f1}\" doesn't match value from \"{f2}\"".format(a1=attribute, f1=entry1, f2=entry2)
		
		# Why do it this way? We want to lstat() instead of stat(), so we can't use os.path.isdir() and friends
		stat1 = os.lstat(entry1)
		stat2 = os.lstat(entry2)
		
		# Modes
		mode1 = stat1.st_mode
		mode2 = stat2.st_mode
		if stat.S_ISREG(mode1):
			assert stat.S_ISREG(mode2)
		if stat.S_ISDIR(mode1):
			assert stat.S_ISDIR(mode2)
		if stat.S_ISLNK(mode1):
			assert stat.S_ISLNK(mode2)
		if stat.S_ISCHR(mode1):
			assert stat.S_ISCHR(mode2)
		if stat.S_ISBLK(mode1):
			assert stat.S_ISBLK(mode2)
		if stat.S_ISFIFO(mode1):
			assert stat.S_ISFIFO(mode2)
		if stat.S_ISSOCK(mode1):
			assert stat.S_ISSOCK(mode2)
		
		# Sizes and the like
		assert stat1.st_size == stat2.st_size, "size mismatch for \"{e1}\" ({s1}) and \"{e2}\" ({s2})".format(e1=entry1, s1=stat1.st_size, e2=entry2, s2=stat2.st_size)
		assert stat1.st_mtime == stat2.st_mtime, "mtime mismatch for \"{e1}\" and \"{e2}\"".format(e1=entry1, e2=entry2)
		assert _md5_path(entry1) == _md5_path(entry2), "md5 hash mismatch for \"{e1}\" and \"{e2}\"".format(e1=entry1, e2=entry2)
		if os.path.isdir(entry1):
			assert_identical_directories(entry1, entry2)
	for file2 in os.listdir(path2):
		assert file2 in seen, "\"{f2}\" exists in \"{p2}\" but not \"{p1}\"".format(f2=file2, p1=path1, p2=path2)

def touch(path):
	if not os.path.exists(path):
		with open(path, "w"):
			pass
	os.utime(path, None)

@contextlib.contextmanager
def chdir(*args, **kwargs):
	cwd = os.getcwd()
	os.chdir(*args, **kwargs)
	try:
		yield os.getcwd()
	finally:
		os.chdir(cwd)
