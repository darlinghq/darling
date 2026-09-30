#!/usr/bin/env python3
"""Exercise Debian staging without compiling Darling (requires debhelper)."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class Packaging(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='darling-debian-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        shutil.copytree(ROOT / 'debian', self.root / 'debian')
        (self.root / 'debbuild').mkdir()
        (self.root / 'debian/changelog').write_text(
            'darling (0.1-1) unstable; urgency=low\n\n  * Test fixture.\n\n'
            ' -- Test <test@example.invalid>  Wed, 30 Sep 2026 00:00:00 +0000\n')
        self.env = dict(os.environ, DEB_HOST_ARCH='amd64', DEB_HOST_ARCH_OS='linux')

    def run_command(self, *args):
        return subprocess.run(args, cwd=self.root, env=self.env,
                              text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    def make(self, target):
        return self.run_command('make', '-f', 'debian/rules', target)

    def install_script(self, fail=False):
        (self.root / 'debbuild/cmake_install.cmake').write_text(
            'file(APPEND "calls" "${COMPONENT}\\n")\n' +
            ('if(COMPONENT STREQUAL "core")\nmessage(FATAL_ERROR "fixture failure")\nendif()\n'
             if fail else '') +
            'file(MAKE_DIRECTORY "$ENV{DESTDIR}/usr/share/darling-fixture")\n'
            'file(WRITE "$ENV{DESTDIR}/usr/share/darling-fixture/${COMPONENT}" "payload")\n')

    def test_component_install_stops_on_first_failure(self):
        self.install_script(fail=True)
        result = self.make('override_dh_auto_install')
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual((self.root / 'debbuild/calls').read_text(), 'core\n')

    def test_python_common_payload_reaches_binary_package(self):
        self.install_script()
        result = self.make('override_dh_auto_install')
        self.assertEqual(result.returncode, 0, result.stdout)
        result = self.run_command('dh_install', '-pdarling-cli-python2-common',
                                  '--sourcedir=debian/tmp')
        self.assertEqual(result.returncode, 0, result.stdout)
        payload = self.root / ('debian/darling-cli-python2-common/usr/share/'
                               'darling-fixture/cli_python_common')
        self.assertTrue(payload.is_file(), 'dh_install omitted the Python common payload')

    def test_shlibdeps_stops_on_first_failure(self):
        bindir = self.root / 'bin'
        bindir.mkdir()
        stub = bindir / 'dh_makeshlibs'
        stub.write_text('#!/bin/sh\nexit 0\n')
        stub.chmod(0o755)
        self.env['PATH'] = str(bindir) + os.pathsep + self.env['PATH']
        tool = self.root / 'tools/debian/make-shlibdeps'
        tool.parent.mkdir(parents=True)
        tool.write_text('#!/bin/sh\necho "$1" >> calls\n[ "$1" != core ]\n')
        tool.chmod(0o755)
        result = self.make('override_dh_makeshlibs')
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual((self.root / 'calls').read_text(), 'core\n')


if __name__ == '__main__':
    unittest.main()
