#!/usr/bin/env python2

import py_compile
import sys

py_compile.compile(sys.argv[1], cfile=sys.argv[2], doraise=True)

