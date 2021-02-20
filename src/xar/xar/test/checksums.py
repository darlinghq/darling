#!/usr/bin/env python

from __future__ import print_function

import hashlib
import os
import os.path
import re
import shutil
import struct
import subprocess

import util


# 
# Utility Functions
# 

def _get_numeric_value_from_header(archive_name, key):
	"""
	Dumps the header of the specified xar archive and extracts the header
	size from the output, in bytes.
	
	"""
	header = subprocess.check_output(["xar", "--dump-header", "-f", archive_name])
	for line in header.splitlines():
		matchdata = re.match("^(.+):\s+(.+)$", line) # magic: 0x78617221 (OK)
		assert matchdata, "unexpected output from `xar --dump-header`:\n{h}".format(h=header)
		if matchdata.groups()[0] == key:
			return int(matchdata.groups()[1])
	raise AssertionError("no \"{k}\" found for archive \"{n}\":\n{h}".format(k=key, n=archive_name, h=header))

def _get_header_size(archive_name):
	return _get_numeric_value_from_header(archive_name, "size")

def _get_toc_size(archive_name):
	return _get_numeric_value_from_header(archive_name, "Compressed TOC length")

def _clobber_bytes_at(clobber_range, path):
	with open(path, "r+") as f:
		f.seek(clobber_range[0])
		with open("/dev/random", "r") as r:
			random_bytes = r.read(len(clobber_range))
			f.write(random_bytes)

def _verify_extraction_failed(filename):
	with util.directory_created("extracted") as directory:
		try:
			with open("/dev/null", "w") as n:
				returncode = subprocess.call(["xar", "-x", "-C", directory, "-f", filename], stdout=n, stderr=n)
			assert returncode != 0, "xar reported success extracting an archive with a broken TOC"
		finally:
			if os.path.exists(directory):
				shutil.rmtree(directory)

def _hex_digest_string(raw_digest):
	unpack_string = "B" * len(raw_digest)
	format_string = "%02x" * len(raw_digest)
	return format_string % struct.unpack(unpack_string, raw_digest)

def _verify_header_checksum(filename, algorithm):
	header_size = _get_header_size(filename)
	toc_length = _get_toc_size(filename)
	
	with open(filename, "r") as f:
		f.seek(header_size)
		h = hashlib.new(algorithm, f.read(toc_length))
		computed_digest = h.digest()
		# We intentionally ignore the TOC-specified offset. We should build a variant of this that uses the TOC
		# offset so we check both.
		stored_digest = f.read(len(computed_digest))
		assert computed_digest == stored_digest, "Digests don't match: expected value {d1} != stored value {d2}".format(d1=_hex_digest_string(computed_digest), d2=_hex_digest_string(stored_digest))
# 
# Test Cases
# 

def default_toc_checksum_validity(filename):
	with util.archive_created(filename, "/bin") as path:
		_verify_header_checksum(path, "sha1")

def sha1_toc_checksum_validity(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha1") as path:
		_verify_header_checksum(path, "sha1")

def sha256_toc_checksum_validity(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha256") as path:
		_verify_header_checksum(path, "sha256")

def sha512_toc_checksum_validity(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha512") as path:
		_verify_header_checksum(path, "sha512")

def broken_toc_default_checksum(filename):
	with util.archive_created(filename, "/bin") as path:
		# Mess up the archive
		toc_start = _get_header_size(path)
		_clobber_bytes_at(range(toc_start + 4, toc_start + 4 + 100), path) # Why did the original test specify 4? No idea.
		
		# Try to extract it
		_verify_extraction_failed(filename)
		
def broken_toc_sha1_checksum(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha1") as path:
		# Mess up the archive
		toc_start = _get_header_size(path)
		_clobber_bytes_at(range(toc_start + 4, toc_start + 4 + 100), path) # Why did the original test specify 4? No idea.
		
		# Try to extract it
		_verify_extraction_failed(filename)

def broken_toc_sha256_checksum(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha256") as path:
		# Mess up the archive
		toc_start = _get_header_size(path)
		_clobber_bytes_at(range(toc_start + 4, toc_start + 4 + 100), path) # Why did the original test specify 4? No idea.

		# Try to extract it
		_verify_extraction_failed(filename)

def broken_toc_sha512_checksum(filename):
	with util.archive_created(filename, "/bin", "--toc-cksum", "sha512") as path:
		# Mess up the archive
		toc_start = _get_header_size(path)
		_clobber_bytes_at(range(toc_start + 4, toc_start + 4 + 100), path) # Why did the original test specify 4? No idea.

		# Try to extract it
		_verify_extraction_failed(filename)

def broken_heap_default_checksum(filename):
	with util.archive_created(filename, "/bin") as path:
		# Mess up the archive
		toc_start = _get_header_size(path)
		toc_size = _get_toc_size(path)
		# Why 32? That's the size of the default sha256 checksum, which is stored before the heap.
		_clobber_bytes_at(range(toc_start + toc_size + 32, toc_start + toc_size + 32 + 100), path)
		
		# Try to extract it
		_verify_extraction_failed(filename)

def default_checksum_algorithm(filename):
	with util.archive_created(filename, "/bin") as path:
		header = subprocess.check_output(["xar", "--dump-header", "-f", path])
		found = False
		for line in header.splitlines():
			matchdata = re.match("^Checksum algorithm:\s+(\d+)\s+\\((\w+)\\)$", line)
			if not matchdata:
				continue
			found = True
			algorithm = matchdata.groups()[1]
			assert algorithm == "SHA1", "unexpected checksum algorithm default: received {a}, expected SHA1".format(a=algorithm)
		assert found, "unexpected output from `xar --dump-header`:\n{h}".format(h=header)


# Apparently, xar doesn't currently fail when given an invalid checksum algorithm. Something to fix later.
# 
# def invalid_checksum_algorithm(filename):
# 	try:
# 		with util.archive_created(filename, "/bin", "--toc-cksum", "invalid-algorithm") as path:
# 			raise AssertionError("xar succeeded when it should have failed")
# 	except subprocess.CalledProcessError:
# 		pass

# It does fail for md5 explicitly, however
def md5_toc_checksum_failure(filename):
	try:
		with open("/dev/null", "a") as devnull:
			with util.archive_created(filename, "/bin", "--toc-cksum", "md5", stderr=devnull) as path:
				raise AssertionError("xar succeeded when it should have failed")
	except subprocess.CalledProcessError:
		pass

def md5_file_checksum_failure(filename):
	try:
		with open("/dev/null", "a") as devnull:
			with util.archive_created(filename, "/bin", "--file-cksum", "md5", stderr=devnull) as path:
				raise AssertionError("xar succeeded when it should have failed")
	except subprocess.CalledProcessError:
		pass

def _verify_checksum_algorithm(filename, algorithm):
	additional_args = []
	if algorithm:
		additional_args = ["--file-cksum", algorithm]
	else:
		algorithm = "sha1"
	
	with util.archive_created(filename, "/bin", *additional_args) as path:
		toc = subprocess.check_output(["xar", "--dump-toc=-", "-f", path])
		found = False
		for line in toc.splitlines():
			if '<unarchived-checksum style="{a}">'.format(a=algorithm) in line or '<archived-checksum style="{a}">'.format(a=algorithm) in line:
				break
		else:
			raise AssertionError("unexpected output from `xar --dump-toc=-`:\n{t}".format(t=toc))

def default_file_checksum_algorithm(filename):
	_verify_checksum_algorithm(filename, None)

def sha1_file_checksum_algorithm(filename):
	_verify_checksum_algorithm(filename, "sha1")

def sha256_file_checksum_algorithm(filename):
	_verify_checksum_algorithm(filename, "sha256")

def sha512_file_checksum_algorithm(filename):
	_verify_checksum_algorithm(filename, "sha512")


TEST_CASES = (default_toc_checksum_validity, sha1_toc_checksum_validity, sha256_toc_checksum_validity, sha512_toc_checksum_validity,
              broken_toc_default_checksum, broken_toc_sha1_checksum, broken_toc_sha256_checksum, broken_toc_sha512_checksum,
              broken_heap_default_checksum, default_checksum_algorithm,
              default_file_checksum_algorithm, sha1_file_checksum_algorithm, sha256_file_checksum_algorithm, sha512_file_checksum_algorithm,
              md5_toc_checksum_failure, md5_file_checksum_failure,)

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
