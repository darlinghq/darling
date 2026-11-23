#!/usr/bin/env python3
"""Check the RPM spec packages every install component, including core_native (needs no build)."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = ROOT / 'rpm/SPECS/darling.spec'
SPECS = ROOT / 'rpm/SPECS'


def bash_array(text, name):
    body = re.search(r'^%s=\(\n(.*?)^\)' % name, text, re.S | re.M).group(1)
    # the Mach-O entries sit between %if lines
    return [line.strip() for line in body.splitlines() if not line.startswith('%')]


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

    def test_macho_part_is_one_conditional(self):
        # the Mach-O subpackages sit between "%if %{with macho}" and its "%endif", core-native does not
        self.assertRegex(self.text, r'(?s)%package core-native\n.*?\n%if %\{with macho\}\n%package core\n')
        self.assertIn('-DDARLING_BUILD_MACHO=OFF', self.text)

    def test_repo_file_has_the_noarch_directory(self):
        repo = (ROOT / 'rpm/darling.repo').read_text()
        self.assertRegex(repo, r'(?m)^baseurl=.*/RPMS/\$basearch$')
        self.assertRegex(repo, r'(?m)^baseurl=.*/RPMS/noarch$')

    # The RPM packages are meant to be built the way the Debian ones are (debian/rules, debian/control): the
    # same components in the same packages, the same slices, the same CMake switches for the Mach-O part.
    @staticmethod
    def debian_list(name):
        rules = (ROOT / 'debian/rules').read_text()
        body = re.search(r'^%s = \\\n((?:\t.*\n)+)' % name, rules, re.M).group(1)
        return body.replace('\\', ' ').split()

    def test_components_and_packages_match_debian(self):
        # The first entry is core_native, which Debian builds in its own tree (debian/rules, -arch targets).
        # Debian's cli_dev component is the darling-cli-devenv package, which has a spec of its own here.
        self.assertEqual(self.components[1:], [c for c in self.debian_list('DARLING_COMPONENTS') if c != 'cli_dev'])
        self.assertEqual(self.suffixes[1:], [c for c in self.debian_list('PACKAGE_SUFFIXES') if c != 'cli-devenv'])

    def test_slices_and_cmake_switches_match_debian(self):
        rules = (ROOT / 'debian/rules').read_text()
        deb_archs = re.search(r'^DARLING_DEB_ARCHS \?= (\S+)$', rules, re.M).group(1)
        spec_archs = re.search(r'%\{!\?darling_archs:%global darling_archs (\S+)\}', self.text).group(1)
        self.assertEqual(spec_archs, deb_archs)
        for switch in ('-DJSC_UNIFIED_BUILD=ON', '-DDARLING_SWIFT_WMO=ON', '-DDEBIAN_PACKAGING=ON'):
            self.assertIn(switch, rules)
            self.assertIn(switch, self.text)

    def test_noarch_packages_are_the_debian_architecture_all_packages(self):
        control = (ROOT / 'debian/control').read_text()
        arch_all = set()
        for stanza in control.split('\n\n'):
            package = re.search(r'^Package: (\S+)$', stanza, re.M)
            if package and re.search(r'^Architecture: all$', stanza, re.M):
                arch_all.add(package.group(1))
        # the two meta packages stay per architecture in the RPM spec, darling-cli-devenv has its own spec
        self.assertEqual(arch_all - {'darling', 'darling-extra', 'darling-cli-devenv'}, NOARCH_PACKAGES)


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


SONAME = 'libwrapped.so.9'

NOARCH_PACKAGES = {
    'darling-core', 'darling-system', 'darling-cli', 'darling-ffi', 'darling-cli-gui-common',
    'darling-iokitd', 'darling-cli-devenv-gui-common', 'darling-cli-extra', 'darling-gui',
    'darling-python2', 'darling-cli-python2-common', 'darling-pyobjc', 'darling-ruby', 'darling-perl',
    'darling-jsc-webkit-common', 'darling-jsc', 'darling-iosurface',
    'darling-cli-devenv-gui-stubs-common', 'darling-gui-stubs',
}


BUILD_SCRIPT = ROOT / 'rpm/build.bsh'


@unittest.skipUnless(shutil.which('rpmspec') and shutil.which('tar'), 'needs rpmspec')
class BuildScript(unittest.TestCase):
    """Runs rpm/build.bsh with rpmbuild and sudo replaced by scripts that log their arguments."""

    def run_script(self, *args, rpmspec_fails=False):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            (tmp / 'src/rpm').mkdir(parents=True)
            shutil.copytree(SPECS, tmp / 'src/rpm/SPECS')
            shutil.copytree(ROOT / 'tools/rpm', tmp / 'src/tools/rpm')
            (tmp / 'bin').mkdir()
            log = tmp / 'log'
            for tool in ('rpmbuild', 'sudo') + (('rpmspec',) if rpmspec_fails else ()):
                script = tmp / 'bin' / tool
                script.write_text('#!/bin/sh\n%s\n' % ('exit 1' if tool == 'rpmspec' else
                                                      'echo "%s $*" >> %s' % (tool, log)))
                script.chmod(0o755)
            env = dict(os.environ, HOME=str(tmp), DARLING_SRC=str(tmp / 'src'), DARLING_COMMIT_DATE='20260101',
                       PATH=str(tmp / 'bin') + os.pathsep + os.environ['PATH'])
            result = subprocess.run(['bash', str(BUILD_SCRIPT), *args], env=env, text=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            calls = log.read_text().splitlines() if log.exists() else []
            return result, [c.split()[-1].rsplit('/', 1)[-1] for c in calls if c.startswith('rpmbuild')], calls

    def test_macho_part_is_built_on_x86_64_only(self):
        for target, built in (('x86_64', ['darling-macho-deps.spec', 'darling.spec', 'darling-cli-devenv.spec']),
                              ('aarch64', ['darling.spec'])):
            result, calls, _ = self.run_script('--target', target)
            self.assertEqual((result.returncode, calls), (0, built), result.stderr)

    def test_rpmbuild_only_options_do_not_turn_the_macho_part_off(self):
        # rpmspec rejects --nodeps; it used to make the script skip macho-deps and darling-cli-devenv silently
        result, built, calls = self.run_script('--nodeps', '--target', 'x86_64')
        self.assertEqual((result.returncode, len(built)), (0, 3), result.stderr)
        self.assertTrue(all('--nodeps' in c for c in calls if c.startswith('rpmbuild')), calls)

    def test_with_and_without_macho_are_honoured(self):
        self.assertEqual(len(self.run_script('--without', 'macho', '--target', 'x86_64')[1]), 1)
        self.assertEqual(len(self.run_script('--with=macho', '--target', 'aarch64')[1]), 3)

    def test_unreadable_spec_is_fatal(self):
        result, built, _ = self.run_script('--target', 'x86_64', rpmspec_fails=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('rpmspec could not read', result.stderr)
        self.assertEqual(built, [])


STUB_CMAKE = r"""cmake_minimum_required(VERSION 3.13)
project(stub C)
option(DARLING_BUILD_MACHO "" ON)
set(DARLING_ARCHS "" CACHE STRING "")
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "core_native")
add_executable(darling darling.c)
install(TARGETS darling DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
	GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
if (DARLING_BUILD_MACHO)
	foreach (c core system cli ffi cli_gui_common iokitd cli_dev_gui_common cli_extra gui python
			cli_python_common pyobjc ruby perl jsc_webkit_common jsc iosurface cli_dev_gui_stubs_common
			gui_stubs cli_dev)
		file(WRITE ${CMAKE_BINARY_DIR}/${c}.txt "${c}\n")
		string(REPLACE ";" "+" slices "${DARLING_ARCHS}")
		install(FILES ${CMAKE_BINARY_DIR}/${c}.txt DESTINATION libexec/darling/usr/share/stub
			RENAME ${c}-${slices}.txt COMPONENT ${c})
	endforeach()
	install(FILES ${CMAKE_SOURCE_DIR}/wrapper.dylib DESTINATION libexec/darling/usr/lib/native COMPONENT gui)
	file(COPY ${CMAKE_SOURCE_DIR}/elfdep DESTINATION ${CMAKE_BINARY_DIR}/src/buildtools)
endif()
"""


def add_tree(tar, directory, prefix):
    for path in sorted(directory.rglob('*')):
        tar.add(path, arcname=prefix + '/' + str(path.relative_to(directory)), recursive=False)


@unittest.skipUnless(all(shutil.which(tool) for tool in ('rpmbuild', 'rpm', 'cmake', 'cc')),
                     'needs rpmbuild, rpm, cmake and a C compiler')
class SpecBuiltWithStub(unittest.TestCase):
    """Builds the spec for both targets against a stub CMake project that installs the components."""

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        tmp = Path(cls.tmp.name)
        stub = tmp / 'stub'
        stub.mkdir()
        (stub / 'CMakeLists.txt').write_text(STUB_CMAKE)
        (stub / 'darling.c').write_text('int main(void) { return 0; }\n')
        (stub / 'LICENSE').write_text('stub\n')
        # A Mach-O header, then the soname the wrappers hold in __elfname (the elfdep stand-in prints the last line)
        (stub / 'wrapper.dylib').write_bytes(bytes.fromhex('cffaedfe07000001030000000600000000000000000000000000000000000000')
                                             + b'\n' + SONAME.encode() + b'\n')
        (stub / 'elfdep').write_text('#!/bin/sh\ntail -n 1 "$1"\n')
        (stub / 'elfdep').chmod(0o755)
        # rpm's configuration with the two files of darling-macho-deps added
        configdir = Path(subprocess.run(['rpm', '--eval', '%{_rpmconfigdir}'], check=True, text=True,
                                        stdout=subprocess.PIPE).stdout.strip())
        cls.configdir = tmp / 'rpm'
        (cls.configdir / 'fileattrs').mkdir(parents=True)
        for entry in configdir.iterdir():
            if entry.name != 'fileattrs':
                (cls.configdir / entry.name).symlink_to(entry)
        for entry in (configdir / 'fileattrs').iterdir():
            (cls.configdir / 'fileattrs' / entry.name).symlink_to(entry)
        (cls.configdir / 'fileattrs/darling.attr').symlink_to(ROOT / 'tools/rpm/darling.attr')
        (cls.configdir / 'darling-deps.req').symlink_to(ROOT / 'tools/rpm/darling-deps.req')
        cls.rpms = {}
        for target in ('x86_64', 'aarch64'):
            top = tmp / ('top-' + target)
            (top / 'SOURCES').mkdir(parents=True)
            with tarfile.open(top / 'SOURCES/darling.tar.gz', 'w:gz') as tar:
                add_tree(tar, stub, 'darling')
            with tarfile.open(top / 'SOURCES/darling-macho-deps.tar.gz', 'w:gz') as tar:
                add_tree(tar, ROOT / 'tools/rpm', 'darling-macho-deps')
            specs = ['darling']
            if target == 'x86_64':
                specs = ['darling-macho-deps', 'darling', 'darling-cli-devenv']
            for spec in specs:
                result = subprocess.run(['rpmbuild', '-bb', '--nodeps', '--target', target,
                                         '--define', '_topdir ' + str(top),
                                         str(SPECS / (spec + '.spec'))],
                                        env=dict(os.environ, DARLING_COMMIT_DATE='20260101',
                                                 RPM_CONFIGDIR=str(cls.configdir)),
                                        text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                assert result.returncode == 0, '%s on %s:\n%s' % (spec, target, result.stdout[-3000:])
            cls.rpms[target] = sorted(top.glob('RPMS/*/*.rpm'))

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def query(self, rpm, fmt):
        return subprocess.run(['rpm', '-qp', '--qf', fmt, str(rpm)], check=True, text=True,
                              stdout=subprocess.PIPE).stdout

    def package_arches(self, target):
        return {self.query(rpm, '%{name}'): rpm.parent.name for rpm in self.rpms[target]}

    def test_x86_64_build_produces_the_native_and_all_noarch_packages(self):
        arches = self.package_arches('x86_64')
        self.assertEqual({n for n, a in arches.items() if a == 'x86_64'},
                         {'darling', 'darling-extra', 'darling-core-native'})
        self.assertEqual({n for n, a in arches.items() if a == 'noarch'},
                         NOARCH_PACKAGES | {'darling-cli-devenv', 'darling-macho-deps'})

    def test_aarch64_build_produces_only_the_native_packages(self):
        self.assertEqual(self.package_arches('aarch64'), {'darling': 'aarch64', 'darling-extra': 'aarch64',
                                                          'darling-core-native': 'aarch64'})

    def files(self, target, name):
        rpm, = [r for r in self.rpms[target] if self.query(r, '%{name}') == name]
        return subprocess.run(['rpm', '-qpl', str(rpm)], check=True, text=True, stdout=subprocess.PIPE).stdout.split()

    def test_slices_and_launcher_are_in_the_right_packages(self):
        self.assertEqual([Path(f).name for f in self.files('x86_64', 'darling-core')], ['core-i386+x86_64+arm64.txt'])
        self.assertEqual(self.files('aarch64', 'darling-core-native'), ['/usr/bin/darling'])

    def test_every_dependency_resolves_on_both_architectures(self):
        # the aarch64 repository is its own build plus the noarch packages of the x86-64 build
        noarch = [r for r in self.rpms['x86_64'] if r.parent.name == 'noarch']
        for target in ('x86_64', 'aarch64'):
            rpms = [r for r in self.rpms[target] if r.parent.name != 'noarch'] + noarch
            provided = set()
            for rpm in rpms:
                for line in self.query(rpm, '[%{PROVIDENAME} = %{PROVIDEVERSION}\n]').splitlines():
                    provided.add(line)
                    provided.add(line.split()[0])
            for rpm in rpms:
                for line in self.query(rpm, '[%{REQUIRENAME} %{REQUIREFLAGS:depflags} %{REQUIREVERSION}\n]').splitlines():
                    if not line.startswith('darling'):
                        continue
                    name, _, version = line.partition(' ')
                    wanted = line.replace('  ', ' ').strip() if version.strip() else name
                    self.assertIn(wanted, provided, '%s: %s requires %s' % (target, rpm.name, wanted))

    def dependencies(self, rpm, kind):
        return self.query(rpm, '[%%{%sNAME}\n]' % kind).split()

    def test_generated_dependencies_are_satisfiable_on_aarch64(self):
        # Only 32-bit packages provide a bare soname, so as a requirement it makes the package uninstallable
        # on aarch64; the 64-bit form is required and the bare one recommended
        rpm, = [r for r in self.rpms['x86_64'] if self.query(r, '%{name}') == 'darling-gui']
        self.assertIn(SONAME + '()(64bit)', self.dependencies(rpm, 'REQUIRE'))
        self.assertNotIn(SONAME, self.dependencies(rpm, 'REQUIRE'))
        self.assertIn(SONAME, self.dependencies(rpm, 'RECOMMEND'))
        for rpm in [r for r in self.rpms['x86_64'] if r.parent.name == 'noarch']:
            for name in self.dependencies(rpm, 'REQUIRE'):
                if not name.startswith(('darling', 'rpmlib(', '/', 'xdg-user-dirs')):
                    self.assertTrue(name.endswith('()(64bit)'), '%s requires %s' % (rpm.name, name))


if __name__ == '__main__':
    # The text checks always run. Without rpmspec the parsed checks cannot, which ctest reports
    # as a skip (exit status 77) once the text checks passed.
    have_rpmspec = shutil.which('rpmspec') is not None
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(SpecText)
    if have_rpmspec:
        suite.addTests(unittest.defaultTestLoader.loadTestsFromTestCase(SpecParsed))
        suite.addTests(unittest.defaultTestLoader.loadTestsFromTestCase(BuildScript))
        suite.addTests(unittest.defaultTestLoader.loadTestsFromTestCase(SpecBuiltWithStub))
    result = unittest.TextTestRunner().run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
    if not have_rpmspec:
        print('SKIP: needs rpmspec for the parsed checks')
        sys.exit(77)
