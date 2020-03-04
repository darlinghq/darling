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

print("You are about to download and install Apple Command Line Tools covered by the following license:")
print("https://www.apple.com/legal/sla/docs/xcode.pdf\n")

while True:
	resp = raw_input("Do you agree with the terms of the license? (y/n) ")

	if resp == "y":
		break
	elif resp == "n":
		exit(1)

packagesResp = urllib2.urlopen("https://swdistcache.darlinghq.org/api/v1/products/by-tag?tag=DTCommandLineTools")
packages = json.loads(packagesResp.read())

tempdir = tempfile.mkdtemp()

print("Downloading packages...")

for package in packages[0]['packages']:
	fname = os.path.basename(package['url'])
	print("Downloading " + fname + "...")

	f = urllib2.urlopen(package['url'])

	fullpath = tempdir + "/" + fname
	with open(fullpath, "wb") as localfile:
		for chunk in iter(lambda: f.read(16*1024), ''):
			localfile.write(chunk)

	print("Installing...")
	exitCode = subprocess.call(["sudo", "installer", "-pkg", fullpath, "-target", "/"])

	if exitCode != 0:
		print("Installation failed with exit code " + str(exitCode))
		exit(1)

	os.remove(fullpath)

os.rmdir(tempdir)

print("Installation complete!")
