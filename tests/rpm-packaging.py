#!/usr/bin/env python3
"""Check the RPM spec packages every install component, including core_native (needs no build)."""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = ROOT / 'rpm/SPECS/darling.spec'


def bash_array(text, name):
    body = re.search(r'^%s=\(\n(.*?)^\)' % name, text, re.S | re.M).group(1)
    return body.split()


class SpecText(unittest.TestCase):
    def setUp(self):
        self.text = SPEC.read_text()
        self.components = bash_array(self.text, 'DARLING_COMPONENTS')
        self.suffixes = bash_array(self.text, 'PACKAGE_SUFFIXES')

    def test_core_native_is_installed(self):
        # It holds the darling launcher, mldr, darlingserver and the binfmt.d file
        self.assertIn('core_native', self.components)

    def test_every_component_has_a_package(self):
        self.assertEqual(len(self.components), len(self.suffixes))
        for suffix in self.suffixes:
            self.assertRegex(self.text, r'(?m)^%%package %s$' % re.escape(suffix))
            self.assertRegex(self.text, r'(?m)^%%files %s -f build/files\.%s\.txt$'
                             % (re.escape(suffix), re.escape(suffix)))

    def test_install_loop_covers_the_whole_array(self):
        # a fixed index range that is shorter than the arrays leaves the last components out
        # (it once left out core_native); any loop over the array indices is fine
        for upper in re.findall(r'\{0\.\.(\d+)\}', self.text):
            self.assertEqual(int(upper) + 1, len(self.suffixes), 'index range {0..%s}' % upper)

    def test_core_depends_on_core_native(self):
        self.assertRegex(self.text, r'(?m)^Requires:\s+darling-core-native = %\{version\}-%\{release\}')


class SpecParsed(unittest.TestCase):
    def rpmspec(self, target, *args):
        result = subprocess.run(['rpmspec', '--target', target, *args, str(SPEC)],
                                text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.split()

    def test_core_native_subpackage(self):
        for target in ('x86_64', 'aarch64'):
            self.assertIn('darling-core-native', self.rpmspec(target, '-q', '--qf', '%{name}\n'))

    def test_32_bit_libraries_only_on_x86_64(self):
        self.assertIn('glibc-devel(x86-32)', self.rpmspec('x86_64', '-q', '--buildrequires'))
        aarch64 = self.rpmspec('aarch64', '-q', '--buildrequires')
        self.assertFalse([r for r in aarch64 if '(x86-' in r], aarch64)
        self.assertIn('glibc-devel(aarch-64)', aarch64)


if __name__ == '__main__':
    # The text checks always run. Without rpmspec the parsed checks cannot, which ctest reports
    # as a skip (exit status 77) once the text checks passed.
    have_rpmspec = shutil.which('rpmspec') is not None
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(SpecText)
    if have_rpmspec:
        suite.addTests(unittest.defaultTestLoader.loadTestsFromTestCase(SpecParsed))
    result = unittest.TextTestRunner().run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
    if not have_rpmspec:
        print('SKIP: needs rpmspec for the parsed checks')
        sys.exit(77)
