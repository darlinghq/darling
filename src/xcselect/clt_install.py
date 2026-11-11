#!/usr/bin/python2.7
# This file is part of Darling.
# 
# Copyright (C) 2020 Lubos Dolezel
# 
# Darling is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Darling is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Darling.  If not, see <http://www.gnu.org/licenses/>.

import urllib2
import json
import tempfile
import os
import subprocess
import sys

# CLT 13.4 packages from Apple CDN (compatible with Darwin 20.x / macOS 11 Big Sur)
# These are the official Apple Command Line Tools for Xcode 13.4
CLT_13_4_BASE = "https://swcdn.apple.com/content/downloads/24/42/002-83793-A_74JRE8GVAT/rlnkct919wgc5c0pjq986z5bb9h62uvni2"
CLT_13_4_PACKAGES = [
	CLT_13_4_BASE + "/CLTools_Executables.pkg",
	CLT_13_4_BASE + "/CLTools_macOS_SDK.pkg",
	CLT_13_4_BASE + "/CLTools_macOSNMOS_SDK.pkg",
	CLT_13_4_BASE + "/CLTools_SwiftBackDeploy.pkg",
]

# Fallback: darlinghq distribution cache (older CLT 12.x)
DARLINGHQ_API = "https://swdistcache.darlinghq.org/api/v1/products/by-tag?tag=DTCommandLineTools"

def download_file(url, dest):
	"""Download a file with progress indication."""
	f = urllib2.urlopen(url)
	info = f.info()
	total = int(info.get("Content-Length", 0))
	downloaded = 0
	block_size = 64 * 1024

	with open(dest, "wb") as localfile:
		while True:
			chunk = f.read(block_size)
			if not chunk:
				break
			localfile.write(chunk)
			downloaded += len(chunk)
			if total > 0:
				pct = downloaded * 100 / total
				sys.stdout.write("\r  %d%% (%d / %d bytes)" % (pct, downloaded, total))
				sys.stdout.flush()
	sys.stdout.write("\n")

def get_cache_dir():
	home = os.environ.get("HOME", "")
	if not home or not os.path.exists(home):
		user = os.environ.get("USER", "darling")
		if os.path.exists("/Volumes/SystemRoot/home/" + user):
			home = "/Volumes/SystemRoot/home/" + user
		else:
			home = "/Users/" + user
	return os.path.join(home, ".cache", "darling")

def install_from_urls(package_urls):
	"""Download and install a list of .pkg URLs."""
	tempdir = tempfile.mkdtemp()
	cache_dir = get_cache_dir()

	if not os.path.exists(cache_dir):
		try:
			os.makedirs(cache_dir)
		except Exception as e:
			print("Could not create cache directory " + cache_dir + ": " + str(e))

	for url in package_urls:
		fname = os.path.basename(url)
		fullpath = tempdir + "/" + fname
		cachepath = cache_dir + "/" + fname

		if os.path.exists(cachepath):
			print("Found " + fname + " in local cache.")
			import shutil
			shutil.copy2(cachepath, fullpath)
		else:
			print("Downloading " + fname + "...")
			try:
				download_file(url, fullpath)
				if os.path.exists(cache_dir):
					print("Caching " + fname + "...")
					import shutil
					shutil.copy2(fullpath, cachepath)
			except Exception as e:
				print("Download failed: " + str(e))
				return 1

		print("Installing " + fname + "...")
		exitCode = subprocess.call(["sudo", "installer", "-pkg", fullpath, "-target", "/"])

		if exitCode != 0:
			print("Installation failed with exit code " + str(exitCode))
			return 1

		os.remove(fullpath)

	os.rmdir(tempdir)
	return 0

def install_from_darlinghq():
	"""Fallback: fetch package list from darlinghq distribution cache."""
	try:
		resp = urllib2.urlopen(DARLINGHQ_API)
		products = json.loads(resp.read())
		urls = [pkg['url'] for pkg in products[0]['packages']]
		return install_from_urls(urls)
	except Exception as e:
		print("darlinghq cache fetch failed: " + str(e))
		return 1

# ---- main ----

print("You are about to download and install Apple Command Line Tools (Xcode 13.4)")
print("covered by the following license:")
print("https://www.apple.com/legal/sla/docs/xcode.pdf\n")

if os.environ.get("AUTO_ACCEPT_LICENSE") == "1":
	print("License auto-accepted via AUTO_ACCEPT_LICENSE environment variable.")
else:
	while True:
		resp = raw_input("Do you agree with the terms of the license? (y/n) ")
		if resp == "y":
			break
		elif resp == "n":
			exit(1)

print("\nDownloading Command Line Tools for Xcode 13.4 from Apple CDN...")
rc = install_from_urls(CLT_13_4_PACKAGES)

if rc != 0:
	print("\nApple CDN download failed. Falling back to darlinghq distribution cache...")
	rc = install_from_darlinghq()

if rc == 0:
	print("\nInstallation complete!")
else:
	print("\nInstallation failed.")
	exit(1)

