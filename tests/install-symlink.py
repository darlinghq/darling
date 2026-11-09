#!/usr/bin/env python3
"""Exercise the real install helper with an isolated CMake project; no Darling build."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

MODULE = Path(__file__).resolve().parents[1] / 'cmake/InstallSymlink.cmake'


class InstallSymlink(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='darling-symlink-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.source = self.root / 'source'
        self.source.mkdir()
        self.build = self.root / 'build'
        self.prefix = self.root / 'prefix with spaces'
        self.stage = self.root / 'stage with spaces'
        (self.source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.13)
project(symlink_fixture NONE)
include("%s")
InstallSymlink("../target with spaces" "lib/default link")
InstallSymlink("../component target" "${CMAKE_INSTALL_PREFIX}/lib/component link" COMPONENT sample)
InstallSymlink("../excluded target" "lib/excluded link" COMPONENT extra EXCLUDE_FROM_ALL)
''' % MODULE)
        result = self.run_command('cmake', '-S', str(self.source), '-B', str(self.build),
                                  '-DCMAKE_INSTALL_PREFIX=' + str(self.prefix))
        self.assertEqual(result.returncode, 0, result.stdout)

    def run_command(self, *args, env=None):
        return subprocess.run(args, cwd=self.root, env=env, text=True,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    def install(self, component=None, staged=True):
        env = dict(os.environ)
        env.pop('DESTDIR', None)
        if staged:
            env['DESTDIR'] = str(self.stage)
        args = ['cmake']
        if component:
            args.append('-DCOMPONENT=' + component)
        args += ['-P', str(self.build / 'cmake_install.cmake')]
        return self.run_command(*args, env=env)

    def destination(self):
        return self.stage / self.prefix.relative_to('/')

    def test_staged_install_and_reinstall(self):
        for _ in range(2):
            result = self.install()
            self.assertEqual(result.returncode, 0, result.stdout)
            dest = self.destination() / 'lib'
            self.assertEqual(os.readlink(dest / 'default link'), '../target with spaces')
            self.assertEqual(os.readlink(dest / 'component link'), '../component target')
            self.assertFalse(os.path.lexists(dest / 'excluded link'))
            self.assertFalse(self.prefix.exists(), 'staging wrote into the host prefix')

    def test_component_selection(self):
        for component, name, target in [('sample', 'component link', '../component target'),
                                        ('extra', 'excluded link', '../excluded target')]:
            result = self.install(component)
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertEqual(os.readlink(self.destination() / 'lib' / name), target)
        self.assertFalse(os.path.lexists(self.destination() / 'lib/default link'))

    def test_unstaged_install(self):
        result = self.install(staged=False)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(os.readlink(self.prefix / 'lib/default link'), '../target with spaces')

    def test_parent_creation_failure_is_fatal(self):
        dest = self.destination()
        dest.mkdir(parents=True)
        (dest / 'lib').write_text('keep this file')
        result = self.install()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual((dest / 'lib').read_text(), 'keep this file')

    def test_symlink_creation_failure_is_fatal(self):
        dest = self.destination() / 'lib/default link'
        dest.mkdir(parents=True)
        (dest / 'keep').write_text('keep this directory')
        result = self.install()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual((dest / 'keep').read_text(), 'keep this directory')


if __name__ == '__main__':
    unittest.main()
