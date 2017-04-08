# This file is part of Darling.
#
# Copyright (C) 2017 Lubos Dolezel
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

import sys, subprocess

def usage():
    print("Usage: %s <Mach-O> <output directory>" % sys.argv[0])

if len(sys.argv) != 3:
    usage()

macho = sys.argv[1]
dest = sys.argv[2]

out = subprocess.check_output(["nm", "-Ug", macho])

functions = []
for line in out.splitlines():
    if line == "":
        continue
    address, id, name = line.split(" ")
    # Remove the underscore
    name = name[1 : ]

    if id == "T":
        functions.append(name)

header = open(dest + "/functions.h", "w")
source = open(dest + "/functions.c", "w")

copyright ="""/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel
    
Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

"""

header.write(copyright)
source.write(copyright)

for funcname in functions:
    header.write("void %s(void);\n" % funcname)
    source.write("void %s(void) { }\n" % funcname)
