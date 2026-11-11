#!/usr/bin/env python3
#
# This file is part of Darling.
#
# Copyright (C) 2026 Lubos Dolezel
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

# Checks the arm64 code that wrapgen emits into native library wrappers:
#
# - every symbol gets a lazy trampoline that its resolver can return before
#   libSystem has set _elfcalls;
# - the shared trampoline path saves the full q0-q7 vector argument registers
#   while it resolves the symbol;
# - glTexSubImage3D and its three siblings get an adapter that moves their
#   trailing stack arguments from the Darwin arm64 layout (packed: 0, 4, 8) to
#   the AAPCS64 layout of the Linux library (8-byte slots: 0, 8, 16), and no
#   other symbol gets one;
# - the generated source compiles for arm64 and x86_64 Darwin.
#
# On an arm64 host (Linux or macOS) the generated trampoline and adapters are
# also assembled and run against callees written in this test, with negative
# controls that must fail.
#
# Usage: arm64-wrappers.py --wrapgen WRAPGEN --source-dir DARLING_SOURCE
#        arm64-wrappers.py --wrapper GENERATED.c --source-dir DARLING_SOURCE
# CC selects the compiler (default: cc).

import argparse
import json
import os
import platform
import re
import shlex
import subprocess
import sys
import tempfile

GL_UPLOADS = ["glTexSubImage3D", "glTexSubImage3DEXT",
              "glCompressedTexSubImage3D", "glCompressedTexSubImage3DARB"]

failures = 0


def check(cond, message):
    global failures
    print(("PASS " if cond else "FAIL ") + message)
    if not cond:
        failures += 1


def run(args, **kwargs):
    return subprocess.run(args, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, universal_newlines=True,
                          **kwargs)


def asm_literals(source):
    # The generated file is our own output; its asm strings only use \n, \t
    # and \" escapes, which JSON string syntax reads the same way.
    return [json.loads(m) for m in re.findall(r'__asm__\(("(?:[^"\\]|\\.)*")\);', source)]


def structural_checks(source):
    check("sub sp, sp, #208" in source, "trampoline frame holds x0-x8 and q0-q7")
    for op in ("stp", "ldp"):
        for i in range(4):
            text = "%s q%d, q%d, [sp, #%d]" % (op, i * 2, i * 2 + 1, 80 + i * 32)
            check(text in source, "trampoline %s of q%d/q%d" % (op, i * 2, i * 2 + 1))
    for sym in GL_UPLOADS + ["fixture_wide"]:
        check("___elf_lazy_%s:" % sym in source, "lazy trampoline for " + sym)
    for sym in GL_UPLOADS:
        check("___elf_abi_%s:" % sym in source, "stack-argument adapter for " + sym)
        check("\treturn __elf_abi_%s;" % sym in source, sym + " resolver returns its adapter")
    check("___elf_abi_fixture_wide" not in source, "no adapter for an unrelated function")
    check("\tif (!_elfcalls)\n\t\treturn __elf_lazy_fixture_wide;" in source,
          "early resolver of an unrelated function returns its trampoline")
    check("__elf_get_fixture_value" in source, "variable accessor still generated")


def compile_checks(cc, wrapper, elfcalls, tmp):
    for target in ("arm64-apple-macos11", "x86_64-apple-macos10.15"):
        obj = os.path.join(tmp, target + ".o")
        r = run(cc + ["-target", target, "-ffreestanding", "-I", elfcalls,
                      "-c", wrapper, "-o", obj])
        if r.returncode != 0 and "unknown target" in r.stdout.lower():
            print("SKIP compile for %s: compiler lacks the target" % target)
            continue
        check(r.returncode == 0, "generated wrapper compiles for " + target)
        if r.returncode != 0:
            print(r.stdout)


EXEC_TEST = r"""
#include <stdint.h>

typedef unsigned int V __attribute__((vector_size(16)));
struct lazy_symbol { void *address; const char *name; };

struct lazy_symbol test_symbol __asm__("test_symbol");
static int resolutions;

static V vector_target(V a, V b, V c, V d, V e, V f, V g, V h, V stack)
{
	return a + b + c + d + e + f + g + h + stack;
}

/* Called by the generated common trampoline; wipes v0-v7 the way a symbol
   lookup in the ELF library may. */
void *lazy_resolve(struct lazy_symbol *symbol) __asm__("___elf_lazy_resolve");
void *lazy_resolve(struct lazy_symbol *symbol)
{
	++resolutions;
	__asm__ volatile(
		"movi v0.16b, #0\n movi v1.16b, #0\n movi v2.16b, #0\n movi v3.16b, #0\n"
		"movi v4.16b, #0\n movi v5.16b, #0\n movi v6.16b, #0\n movi v7.16b, #0"
		::: "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7");
	symbol->address = (void *) vector_target;
	return symbol->address;
}

extern V test_entry(V, V, V, V, V, V, V, V, V) __asm__("test_entry");

/* Stand-ins for the Linux library functions: AAPCS64 gives each trailing
   argument its own 8-byte slot. uint64_t parameters read those slots under
   both the Darwin and the Linux arm64 conventions. */
static const void *expected_pointer;
static unsigned expected_format, expected_second;
static int callee_ok, callee_calls;

static void callee(uint32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f,
	int32_t g, int32_t h, uint64_t format, uint64_t second, const void *pointer)
{
	++callee_calls;
	callee_ok = a == 1 && b == 2 && c == 3 && d == 4 && e == 5 && f == 6 && g == 7 && h == 8
		&& (uint32_t) format == expected_format && (uint32_t) second == expected_second
		&& pointer == expected_pointer;
}

#define NATIVE(sym) \
	void native_##sym(uint32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, \
		int32_t g, int32_t h, uint64_t x, uint64_t y, const void *p) __asm__("___elf_lazy_" #sym); \
	void native_##sym(uint32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, \
		int32_t g, int32_t h, uint64_t x, uint64_t y, const void *p) \
	{ callee(a, b, c, d, e, f, g, h, x, y, p); } \
	extern char adapter_##sym[] __asm__("___elf_abi_" #sym);

NATIVE(glTexSubImage3D)
NATIVE(glTexSubImage3DEXT)
NATIVE(glCompressedTexSubImage3D)
NATIVE(glCompressedTexSubImage3DARB)

/* How a Darwin caller lays out (8 register args, uint32, uint32, pointer):
   the two 32-bit values share stack offset 0..7 and the pointer is at 8. */
typedef void (*darwin_call)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t,
	int32_t, int32_t, uint64_t packed, const void *pointer);

static int call_packed(void *function, uint32_t format, uint32_t second, const void *pointer)
{
	expected_format = format;
	expected_second = second;
	expected_pointer = pointer;
	callee_ok = 0;
	((darwin_call) function)(1, 2, 3, 4, 5, 6, 7, 8,
		(uint64_t) format | ((uint64_t) second << 32), pointer);
	return callee_ok;
}

int main(int argc, char **argv)
{
	int negative = argc > 1;
	V a = {1, 2, 3, 4}, b = {5, 6, 7, 8}, c = {9, 10, 11, 12}, d = {13, 14, 15, 16};
	V e = {17, 18, 19, 20}, f = {21, 22, 23, 24}, g = {25, 26, 27, 28}, h = {29, 30, 31, 32};
	V s = {33, 34, 35, 36};
	V expected = vector_target(a, b, c, d, e, f, g, h, s);
	unsigned char bytes[8];

	if (negative) {
		/* Calling the AAPCS64 function directly with Darwin packing must be
		   detected as corrupted arguments. */
		return call_packed((void *) native_glTexSubImage3D, 0x1908, 0x1401, bytes) ? 0 : 3;
	}

	for (int call = 0; call < 2; ++call) {
		V got = test_entry(a, b, c, d, e, f, g, h, s);
		for (int lane = 0; lane < 4; ++lane)
			if (got[lane] != expected[lane])
				return 1;
	}
	if (resolutions != 1)
		return 2;

	void *adapters[] = { adapter_glTexSubImage3D, adapter_glTexSubImage3DEXT,
		adapter_glCompressedTexSubImage3D, adapter_glCompressedTexSubImage3DARB };
	uint32_t seconds[] = { 0x1401, 0x1401, (uint32_t) -101, 32 };
	for (int round = 0; round < 3; ++round)
		for (int i = 0; i < 4; ++i)
			if (!call_packed(adapters[i], 0x1908, seconds[i], bytes + round + i))
				return 4;
	if (callee_calls != 12)
		return 5;
	return 0;
}
"""


def exec_checks(cc, source, tmp):
    darwin = sys.platform == "darwin"
    literals = asm_literals(source)
    common = [l for l in literals if "___elf_lazy_common:" in l]
    adapters = [l for l in literals if "___elf_abi_" in l]
    check(len(common) == 1 and len(adapters) == len(GL_UPLOADS),
          "found the generated trampoline and adapters")
    if failures:
        return

    def assembly(narrow):
        text = "\n".join(common + adapters)
        if not darwin:
            text = text.replace(".private_extern", ".global")
        if narrow:
            text = re.sub(r"\b(stp|ldp) q([0-7]), q([0-7])", r"\1 d\2, d\3", text)
        if darwin:
            text += ("\n.text\n.globl test_entry\n.p2align 2\ntest_entry:\n"
                     "\tadrp x16, test_symbol@PAGE\n\tadd x16, x16, test_symbol@PAGEOFF\n"
                     "\tb ___elf_lazy_common\n")
        else:
            text += ("\n.text\n.global test_entry\n.p2align 2\ntest_entry:\n"
                     "\tadrp x16, test_symbol\n\tadd x16, x16, :lo12:test_symbol\n"
                     "\tb ___elf_lazy_common\n"
                     ".section .note.GNU-stack,\"\",@progbits\n")
        return text

    test_c = os.path.join(tmp, "exec.c")
    with open(test_c, "w") as f:
        f.write(EXEC_TEST)

    def build(name, narrow):
        asm = os.path.join(tmp, name + ".S")
        with open(asm, "w") as f:
            f.write(assembly(narrow))
        exe = os.path.join(tmp, name)
        r = run(cc + ["-O2", test_c, asm, "-o", exe])
        check(r.returncode == 0, "build " + name)
        if r.returncode != 0:
            print(r.stdout)
            return None
        return exe

    exe = build("exec", False)
    if exe:
        r = run([exe])
        check(r.returncode == 0,
              "generated trampoline keeps vector arguments and adapters repack stack "
              "arguments (exit %d)" % r.returncode)
        r = run([exe, "negative"])
        check(r.returncode == 3, "negative control: packed arguments reach an AAPCS64 "
              "function corrupted without the adapter (exit %d)" % r.returncode)
    narrow = build("narrow", True)
    if narrow:
        r = run([narrow])
        check(r.returncode == 1, "negative control: saving only d0-d7 loses vector "
              "lanes (exit %d)" % r.returncode)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--wrapgen")
    parser.add_argument("--wrapper")
    parser.add_argument("--source-dir", required=True)
    opts = parser.parse_args()
    cc = shlex.split(os.environ.get("CC", "cc"))
    here = os.path.join(opts.source_dir, "tests", "wrapgen")
    elfcalls = os.path.join(opts.source_dir, "src", "startup", "mldr", "elfcalls")

    with tempfile.TemporaryDirectory(prefix="wrapgen-test-") as tmp:
        wrapper = opts.wrapper
        if not wrapper:
            fixture = os.path.join(tmp, "fixture.so")
            r = run(cc + ["-shared", "-fPIC", os.path.join(here, "native-fixture.c"),
                          "-o", fixture])
            if r.returncode != 0:
                print(r.stdout)
                return 1
            wrapper = os.path.join(tmp, "fixture.c")
            r = run([opts.wrapgen, fixture, wrapper, os.path.join(tmp, "fixture_vars.h")])
            if r.returncode != 0:
                print(r.stdout)
                return 1
        with open(wrapper) as f:
            source = f.read()

        structural_checks(source)
        compile_checks(cc, wrapper, elfcalls, tmp)
        if platform.machine() in ("aarch64", "arm64"):
            exec_checks(cc, source, tmp)
        else:
            print("SKIP execution checks: not an arm64 host")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
