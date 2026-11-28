# Generated installation data for the Darling guest CPython 3 framework.
#
# CPython's sysconfig module imports _sysconfigdata_<abi>_<platform> at run
# time. A framework build has an empty ABI tag and uses "darwin" as its
# platform, so the module name below is fixed by the platform ABI rather than
# by any application.

_framework = "/System/Library/Frameworks/Python.framework/Versions/3.13"

build_time_vars = {
    "abiflags": "",
    "BINDIR": _framework + "/bin",
    "CC": "clang",
    "CFLAGS": "-fno-common -Wall -O3",
    "CONFIG_ARGS": "--without-ensurepip --disable-test-modules",
    "exec_prefix": _framework,
    "prefix": _framework,
    "BINLIBDEST": _framework + "/lib/python3.13",
    "EXE": "",
    "EXT_SUFFIX": ".so",
    "INCLUDEPY": _framework + "/include/python3.13",
    "LIBDEST": _framework + "/lib/python3.13",
    "LIBDIR": _framework + "/lib",
    "LIBPL": _framework + "/lib/config-3.13-darwin",
    "LIBPYTHON": "",
    "LDLIBRARY": _framework + "/Python",
    "LIBRARY": _framework + "/Python",
    "LDFLAGS": "",
    "LIBS": "-lpthread -ldl",
    "MACHDEP": "darwin",
    "MULTIARCH": "",
    "platlibdir": "lib",
    "py_version": "3.13.18",
    "py_version_short": "3.13",
    "py_version_nodot": "313",
    "PY_VERSION": "3.13.18",
    "PYTHONFRAMEWORK": "Python",
    "PYTHONFRAMEWORKDIR": "Python.framework",
    "PYTHONFRAMEWORKINSTALLDIR": "/System/Library/Frameworks/Python.framework",
    "PYTHONFRAMEWORKPREFIX": "/System/Library/Frameworks",
    "SHLIB_SUFFIX": ".so",
    "SOABI": "",
    "srcdir": _framework,
    "SYSLIBS": "-lm",
    "TZPATH": "/usr/share/zoneinfo:/usr/lib/zoneinfo:/usr/share/lib/zoneinfo:/etc/zoneinfo",
    "VERSION": "3.13",
    "WITH_DYLD": 1,
}
