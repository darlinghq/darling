#!/usr/bin/env python3
"""Builds libdarling-fexcore.so the way WITH_FEXCORE does, without configuring Darling.

The source list, include directories, compile options, link libraries and link options are read
from the darling-fexcore target in src/startup/mldr/CMakeLists.txt, so this cannot drift from the
package build. The full Darling configure needs every submodule and is far heavier than the shim.

    tests/mldr/build-fexcore-shim.py [--check] OUT_DIR

--check only validates (no compiling, no OUT_DIR): the CMake target, FEX's build cache and the
FEX_VERSION pin. It fails if darling-fexcore is changed by a CMake command this script does not
read, so a new definition, source, option or property cannot be silently left out; the commands
it reads may each appear once. It does not reproduce the directory-level -DDARLING, -DMLDR_BUILD,
-D_GNU_SOURCE and INSTALL_PREFIX of the mldr target, which the shim sources do not use, and its
defines differ a little from FEX's own compile_commands for the shared FEX sources (it adds
-DJIT_ARM64 and drops -DNDEBUG); none of that changes a header layout.

Environment: FEXCORE_SOURCE_DIR (a FEX checkout) and FEXCORE_BUILD_DIR (its build tree, for
example the result of tests/mldr/build-fex-for-shim.sh) are required, as in CMake. CXX overrides
clang++, JOBS the compile parallelism. Besides linking with --no-undefined, it checks that the
library exports every symbol fex_engine.cpp looks up with dlsym. Exits 1 on any failure.
"""
import concurrent.futures
import os
import re
import subprocess
import sys

ROOT = os.path.realpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
MLDR = os.path.join(ROOT, "src", "startup", "mldr")
TARGET = "darling-fexcore"


def die(message):
    print(f"build-fexcore-shim.py: {message}", file=sys.stderr)
    sys.exit(1)


# Commands that set up darling-fexcore and are read by main(), and commands that name it without
# changing how it is compiled or linked.
READ_COMMANDS = ("add_library", "target_include_directories", "target_compile_options",
                 "target_link_libraries", "target_link_options")
OTHER_COMMANDS = ("add_dependencies", "add_test", "install", "set_tests_properties", "option",
                  "message")


def check_target_commands(cmake_text):
    """Fails unless every CMake command naming the target is read here (once) or harmless."""
    text = re.sub(r"#.*", "", cmake_text)
    starts = [(m.start(), m.group(1)) for m in re.finditer(r"^[ \t]*([A-Za-z_]\w*)[ \t]*\(", text, re.M)]
    counts = {}
    for mention in re.finditer(rf"(?<![\w-]){re.escape(TARGET)}(?![\w-])", text):
        command = [name for start, name in starts if start < mention.start()][-1]
        counts[command] = counts.get(command, 0) + 1
    for command, count in counts.items():
        if command in READ_COMMANDS and count != 1:
            die(f"{command}() names {TARGET} {count} times in src/startup/mldr/CMakeLists.txt; "
                "this script reads exactly one such call per command")
        if command not in READ_COMMANDS + OTHER_COMMANDS:
            die(f"{command}() changes {TARGET} in src/startup/mldr/CMakeLists.txt but this script "
                "does not read it; teach tests/mldr/build-fexcore-shim.py about it")


def check_fex_pin():
    """FEX_VERSION must be a full commit id, and THIRD-PARTY-NOTICES.md must name the same one."""
    with open(os.path.join(MLDR, "fex", "FEX_VERSION")) as pin:
        version = pin.read().strip()
    if not re.fullmatch(r"[0-9a-f]{40}", version):
        die(f"src/startup/mldr/fex/FEX_VERSION must hold a 40 digit commit id, not {version!r}")
    with open(os.path.join(ROOT, "THIRD-PARTY-NOTICES.md")) as notices:
        noted = re.search(r"FEX-Emu/FEX>, commit ([0-9a-f]+)", notices.read())
    if not noted or not version.startswith(noted.group(1)):
        die("THIRD-PARTY-NOTICES.md names FEX commit "
            f"{noted.group(1) if noted else '(none)'}, but FEX_VERSION is {version}")


def command_args(cmake_text, command, first_arg):
    """Tokens of the call `command(first_arg ...)`, with quotes removed and \\" unescaped."""
    match = re.search(rf"^\s*{command}\(\s*{re.escape(first_arg)}\b", cmake_text, re.M)
    if not match:
        die(f"{command}({first_arg} ...) not found in src/startup/mldr/CMakeLists.txt")
    tokens, token, quoted, i = [], None, False, match.end()
    while True:
        if i >= len(cmake_text):
            die(f"unterminated {command}({first_arg} ...)")
        c = cmake_text[i]
        if quoted:
            if c == "\\" and cmake_text[i + 1] == '"':
                token += '"'
                i += 1
            elif c == '"':
                quoted = False
            else:
                token += c
        elif c == '"':
            quoted, token = True, token or ""
        elif c == "#":
            i = cmake_text.index("\n", i)
            continue
        elif c in " \t\n)":
            if token is not None:
                tokens.append(token)
                token = None
            if c == ")":
                return tokens
        else:
            token = (token or "") + c
        i += 1


def main():
    args = sys.argv[1:]
    check_only = args == ["--check"]
    if not check_only and (len(args) != 1 or args[0].startswith("-")):
        die("usage: build-fexcore-shim.py [--check] OUT_DIR")
    out = None if check_only else os.path.abspath(args[0])
    cxx = os.environ.get("CXX", "clang++")
    jobs = int(os.environ.get("JOBS", os.cpu_count() or 2))

    fex_src = os.environ.get("FEXCORE_SOURCE_DIR")
    fex_build = os.environ.get("FEXCORE_BUILD_DIR")
    if not fex_src or not fex_build:
        die("set FEXCORE_SOURCE_DIR and FEXCORE_BUILD_DIR")

    # FEX's cache holds what its build decided; the shim's hard-coded options must agree with it.
    try:
        with open(os.path.join(fex_build, "CMakeCache.txt")) as cache:
            cache_text = cache.read()
    except OSError as error:
        die(f"cannot read FEX's build cache: {error}")
    # The shim compiles with -DFEX_HAS_PRESERVE_ALL_ATTR=1 and -DFEXCORE_HAS_PRESERVE_ALL_ATTR=1,
    # and FEX sets both from this one check. If it failed (an old compiler), the archives use the
    # default calling convention, and a shim using preserve_all would link but misbehave.
    if not re.search(r"^HAS_CLANG_PRESERVE_ALL:\w+=1$", cache_text, re.M):
        die(f"{fex_build}/CMakeCache.txt does not have HAS_CLANG_PRESERVE_ALL=1: FEX was built "
            "without preserve_all, which the shim's options assume; build FEX with a newer clang")
    # CMake locates fmt with find_library; the shim must link the fmt FEX was built against, not
    # a distro one of another version (tests/mldr/build-fex-for-shim.sh explains why).
    found = re.search(r"^fmt_DIR:\w+=(.*)/cmake/fmt$", cache_text, re.M)
    fmt_library = os.path.join(found.group(1), "libfmt.so") if found else None
    if not fmt_library or not os.path.exists(fmt_library):
        die(f"cannot find libfmt.so from fmt_DIR in {fex_build}/CMakeCache.txt "
            f"({fmt_library or 'fmt_DIR is not set'}); build FEX with tests/mldr/build-fex-for-shim.sh")
    xxhash_library = "-lxxhash"
    variables = {"FEXCORE_SOURCE_DIR": fex_src, "FEXCORE_BUILD_DIR": fex_build,
                 "FEXCORE_FMT_LIBRARY": fmt_library, "FEXCORE_XXHASH_LIBRARY": xxhash_library}

    def expand(token):
        return re.sub(r"\$\{(\w+)\}", lambda m: variables[m.group(1)] if m.group(1) in variables
                      else die(f"{token}: CMake variable {m.group(1)} is not known to this script"), token)

    with open(os.path.join(MLDR, "CMakeLists.txt")) as cmake:
        text = cmake.read()
    check_target_commands(text)
    check_fex_pin()

    def target_args(command, first_arg=TARGET, skip=("PRIVATE", "SHARED")):
        return [expand(t) for t in command_args(text, command, first_arg) if t not in skip]

    sources = [os.path.join(MLDR, s) if not os.path.isabs(s) else s for s in target_args("add_library")]
    includes = target_args("target_include_directories")
    options = target_args("target_compile_options")
    link_libs = target_args("target_link_libraries")
    link_options = target_args("target_link_options")
    # mldr/CMakeLists.txt applies include_directories(include) to the whole directory.
    flags = options + [f"-I{os.path.join(MLDR, 'include')}"] + [f"-I{d}" for d in includes]

    if check_only:
        print("the darling-fexcore target, FEX's build cache and FEX_VERSION are consistent")
        return
    os.makedirs(out, exist_ok=True)
    for source in sources:
        if not os.path.isfile(source):
            die(f"source {source} does not exist")

    def compile_one(source):
        obj = os.path.join(out, os.path.basename(source) + ".o")
        subprocess.run([cxx, *flags, "-c", source, "-o", obj], check=True)
        return obj

    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        try:
            objs = list(pool.map(compile_one, sources))
        except subprocess.CalledProcessError as error:
            die(f"compiling failed: {' '.join(error.cmd[:1])} exited {error.returncode}")

    library = os.path.join(out, "libdarling-fexcore.so")
    link = [cxx, "-shared", "-o", library, *objs, *link_options, *link_libs]
    if subprocess.run(link).returncode:
        die("linking libdarling-fexcore.so failed")

    exported = subprocess.run(["nm", "-D", "--defined-only", library], check=True,
                              capture_output=True, text=True).stdout
    with open(os.path.join(MLDR, "fex", "fex_engine.cpp")) as engine:
        wanted = sorted(set(re.findall(r'dlsym\(\s*\w+,\s*"(darling_fexcore_\w+)"', engine.read())))
    if not wanted:
        die("found no dlsym(\"darling_fexcore_...\") lookups in fex_engine.cpp")
    missing = [name for name in wanted if not re.search(rf" T {name}$", exported, re.M)]
    if missing:
        die("libdarling-fexcore.so does not export what fex_engine.cpp looks up: " + ", ".join(missing))
    print(f"built {library}; exports all {len(wanted)} symbols fex_engine.cpp looks up")


main()
