#!/usr/bin/env python3
# Run with: python3 tests/swift-toolchain.py
# Host-only discovery tests; fake compilers do not build Swift SDK overlays.
# Swiftly's proxy and location-query contract:
# https://www.swift.org/swiftly/documentation/swiftly/use-toolchains/
# https://github.com/swiftlang/swiftly/blob/main/DESIGN.md

import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


CMAKE = os.environ.get('CMAKE') or shutil.which('cmake')
MODULE = Path(__file__).resolve().parents[1] / 'cmake' / 'swift_toolchain.cmake'


@unittest.skipUnless(CMAKE, 'cmake is required')
class SwiftToolchainTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='darling-swift-toolchain-')
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        self.bin = self.root / 'manager' / 'bin'
        self.bin.mkdir(parents=True)
        self.toolchain = self.root / 'selected toolchain'
        self.compiler = self.toolchain / 'usr' / 'bin' / 'swiftc'
        self.compiler.parent.mkdir(parents=True)
        for resource in ('lib/swift/shims', 'lib/clang'):
            (self.toolchain / 'usr' / resource).mkdir(parents=True)
        self.write_compiler('6.4.2')
        self.write_proxy(str(self.toolchain))
        (self.bin / 'swiftc').symlink_to('swiftly')

    def write_compiler(self, version):
        self.compiler.write_text(
            '#!/bin/sh\n'
            '[ "$1" = --version ] || exit 1\n'
            "printf '%s\\n' " + shlex.quote('Swift version ' + version) + '\n')
        self.compiler.chmod(0o755)

    def write_proxy(self, location, status=0):
        proxy = self.bin / 'swiftly'
        proxy.write_text(
            '#!/bin/sh\n'
            'if [ "$1" = use ] && [ "$2" = --print-location ] && [ "$#" = 2 ]; then\n'
            "  printf '%s\\n' " + shlex.quote(location) + '\n'
            '  exit ' + str(status) + '\n'
            'fi\n'
            '[ "$1" = --version ] || exit 1\n'
            "printf '%s\\n' 'Swift version 6.4.2'\n")
        proxy.chmod(0o755)

    def detect(self, override=None, search_path=None):
        result = self.root / 'result.txt'
        script = self.root / 'probe.cmake'
        script.write_text(
            'include([=[' + str(MODULE) + ']=])\n'
            'file(WRITE [=[' + str(result) + ']=] '
            '"${SWIFT_TOOLCHAIN_FOUND}\n${SWIFT_COMPILER}\n'
            '${SWIFT_TOOLCHAIN_USR}\n${SWIFT_TOOLCHAIN_VERSION}\n")\n')
        command = [CMAKE, '-DCMAKE_PROGRAM_PATH=' + str(search_path or self.bin)]
        if override is not None:
            command.append('-DSWIFT_TOOLCHAIN=' + str(override))
        command += ['-P', str(script)]
        env = dict(os.environ)
        env.pop('SWIFT_TOOLCHAIN', None)
        env['PATH'] = str(self.bin) + os.pathsep + env.get('PATH', '')
        run = subprocess.run(command, cwd=self.root, env=env, text=True,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        self.assertEqual(run.returncode, 0, run.stdout)
        values = result.read_text().splitlines()
        self.assertEqual(len(values), 4, run.stdout)
        return values

    def expect_selected(self, values, version='6.4.2'):
        self.assertEqual(values, ['TRUE', str(self.compiler),
                                  str(self.toolchain / 'usr'), version])

    def test_swiftly_proxy_resolves_compiler_and_resources(self):
        # The manager and selected installation are separate, nondefault paths.
        # The installation path also contains a space.
        self.expect_selected(self.detect())

    def test_swiftly_selection_is_resolved_on_reconfigure(self):
        source = self.root / 'source'
        source.mkdir()
        build = self.root / 'build'
        result = self.root / 'result.txt'
        (source / 'CMakeLists.txt').write_text(
            'cmake_minimum_required(VERSION 3.13)\n'
            'project(swift_toolchain_fixture NONE)\n'
            'include([=[' + str(MODULE) + ']=])\n'
            'get_property(discovered CACHE SWIFT_COMPILER PROPERTY VALUE)\n'
            'file(WRITE [=[' + str(result) + ']=] '
            '"${SWIFT_TOOLCHAIN_FOUND}\n${SWIFT_COMPILER}\n'
            '${SWIFT_TOOLCHAIN_USR}\n${SWIFT_TOOLCHAIN_VERSION}\n${discovered}\n")\n')
        replacement = self.root / 'replacement toolchain'
        shutil.copytree(self.toolchain, replacement)
        env = dict(os.environ)
        env.pop('SWIFT_TOOLCHAIN', None)
        for toolchain, version in ((self.toolchain, '6.4.2'), (replacement, '6.4.3')):
            with self.subTest(version=version):
                self.toolchain = toolchain
                self.compiler = toolchain / 'usr' / 'bin' / 'swiftc'
                self.write_compiler(version)
                self.write_proxy(str(toolchain))
                run = subprocess.run(
                    [CMAKE, '-S', str(source), '-B', str(build),
                     '-DCMAKE_PROGRAM_PATH=' + str(self.bin)],
                    cwd=self.root, env=env, text=True,
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                self.assertEqual(run.returncode, 0, run.stdout)
                values = result.read_text().splitlines()
                self.expect_selected(values[:4], version)
                self.assertEqual(values[4:], [str(self.bin / 'swiftc')])

    def test_direct_compiler_search_is_preserved(self):
        self.write_proxy('', status=1)
        self.expect_selected(self.detect(search_path=self.compiler.parent))

    def test_explicit_toolchain_overrides_swiftly(self):
        self.write_proxy('', status=1)
        for override in (self.toolchain, self.toolchain / 'usr'):
            with self.subTest(override=override):
                self.expect_selected(self.detect(override=override))

    def test_disabled_overlays(self):
        self.assertEqual(self.detect(override='OFF'), ['FALSE', '', '', ''])

    def test_unavailable_swiftly_selection(self):
        for location, status in (('', 0), (str(self.toolchain), 1)):
            with self.subTest(location=location, status=status):
                self.write_proxy(location, status)
                self.assertEqual(self.detect()[0], 'FALSE')

    def test_selected_compiler_version_is_checked(self):
        self.write_compiler('6.2.3')
        self.assertEqual(self.detect()[0], 'FALSE')

    def test_missing_resources_are_rejected(self):
        for resource in ('lib/swift/shims', 'lib/clang'):
            with self.subTest(resource=resource):
                path = self.toolchain / 'usr' / resource
                path.rmdir()
                self.assertEqual(self.detect()[0], 'FALSE')
                path.mkdir()

    def test_missing_selected_compiler_is_rejected(self):
        self.compiler.unlink()
        self.assertEqual(self.detect()[0], 'FALSE')


if __name__ == '__main__':
    unittest.main()
