#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""Check that the Clang modules and API notes in Darling's SDK load.

Host test: runs the host clang with -fmodules -fsyntax-only against the SDK in
this checkout; it needs no Swift and no Darling runtime. The SDK's headers are
symlinks into src/external, so the submodules must be checked out (the test is
skipped otherwise).

Usage: tests/sdk-clang-modules.py   (CLANG=/path/to/clang to choose the compiler)
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SDK = Path(os.environ.get('SDK', ROOT / 'Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk'))
CLANG = os.environ.get('CLANG', 'clang')
AE_INCLUDE = ROOT / 'src/frameworks/CoreServices/include'

# Modules that only need the C headers.
C_MODULES = ['Darwin', 'Darwin.MacTypes', 'Darwin.os.lock', 'Dispatch', 'os.log',
             'os.activity', 'os.signpost', 'XPC', 'CommonCrypto']
# Modules that need Foundation (and so the AE headers, see below).
FOUNDATION_MODULES = ['CoreFoundation', 'Foundation', 'CoreGraphics', 'QuartzCore',
                      'CoreText', 'AppKit', 'OpenGL']
OBJC_MODULES = ['ObjectiveC']


def sdk_usable():
    return shutil.which(CLANG) is not None and (SDK / 'usr/include/stdio.h').exists()


@unittest.skipUnless(sdk_usable(), 'clang or the SDK headers (submodules) are not available')
class SdkModules(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='darling-modules-')
        self.addCleanup(self.tmp.cleanup)
        self.cache = Path(self.tmp.name) / 'cache'

    def clang(self, source, *extra, language='objective-c'):
        src = Path(self.tmp.name) / 'probe.m'
        src.write_text(source)
        cmd = [CLANG, '-target', 'x86_64-apple-macos10.13', '-isysroot', str(SDK),
               '-fmodules', '-fmodules-cache-path=' + str(self.cache),
               '-fsyntax-only', '-Wno-everything', *extra, '-x', language, str(src)]
        return subprocess.run(cmd, capture_output=True, text=True)

    def import_module(self, name):
        extra = []
        # Foundation's NSAppleEventDescriptor.h includes <AE/AE.h>, which the
        # SDK does not expose by itself yet; add the framework sources' include
        # directory so the Foundation-dependent modules can be tested.
        if name in FOUNDATION_MODULES and not (SDK / 'usr/include/AE/AE.h').exists():
            extra = ['-I', str(AE_INCLUDE)]
        res = self.clang('@import %s;\n' % name, *extra)
        self.assertEqual(res.returncode, 0, 'import of %s failed:\n%s' % (name, res.stderr))

    def test_import_c_modules(self):
        for name in C_MODULES:
            with self.subTest(module=name):
                self.import_module(name)

    def test_import_objc_modules(self):
        for name in OBJC_MODULES:
            with self.subTest(module=name):
                self.import_module(name)

    def test_import_foundation_modules(self):
        for name in FOUNDATION_MODULES:
            with self.subTest(module=name):
                self.import_module(name)

    def test_unknown_module_is_rejected(self):
        # Guards the tests above: a module that does not exist must fail.
        self.assertNotEqual(self.clang('@import DarlingNoSuchModule;\n').returncode, 0)

    def test_objectivec_apinotes_apply(self):
        # ObjectiveC.apinotes marks -respondsToSelector:'s argument nonnull.
        code = ('#import <objc/NSObject.h>\n'
                'void f(NSObject *o, SEL _Nullable s) { [o respondsToSelector:s]; }\n')
        on = self.clang(code, '-fapinotes-modules', '-Wnullable-to-nonnull-conversion')
        self.assertEqual(on.returncode, 0, on.stderr)
        self.assertIn('nullable pointer', on.stderr)
        off = self.clang(code, '-fno-apinotes-modules', '-Wnullable-to-nonnull-conversion')
        self.assertNotIn('nullable pointer', off.stderr)

    def test_dispatch_and_os_apinotes_apply(self):
        # The Dispatch and os API notes give the types and functions the Swift names
        # that the Swift overlays are written against; -D__swift__ makes the headers
        # declare the classes as they are when Swift imports them.
        code = '@import Dispatch;\n@import os.log;\n'
        expected = {'OS_dispatch_queue': 'DispatchQueue', 'OS_os_log': 'OSLog',
                    'os_log_create': 'OSLog.init(__subsystem:category:)'}
        for decl, swift_name in expected.items():
            with self.subTest(decl=decl):
                dump = self.clang(code, '-D__swift__=60000', '-fapinotes-modules', '-Xclang', '-ast-dump',
                                  '-Xclang', '-ast-dump-filter', '-Xclang', decl)
                self.assertEqual(dump.returncode, 0, dump.stderr)
                self.assertIn('SwiftNameAttr', dump.stdout)
                self.assertIn('"%s"' % swift_name, dump.stdout)

    def test_darwin_umbrella_is_current(self):
        # Every header the generated Darwin umbrella includes must exist; a
        # stale umbrella (headers moved or removed) breaks `import Darwin`.
        umbrella = (SDK / 'usr/include/_DarlingDarwin.h').read_text().splitlines()
        for line in umbrella:
            if line.startswith('#include <'):
                header = line[len('#include <'):-1]
                self.assertTrue((SDK / 'usr/include' / header).exists(), header)

    def test_sdksettings_matches_plist(self):
        import json
        import plistlib
        settings = json.loads((SDK / 'SDKSettings.json').read_text())
        plist = plistlib.loads((SDK / 'SDKSettings.plist').read_bytes())
        for key in ('CanonicalName', 'DisplayName', 'MinimalDisplayName', 'Version',
                    'MaximumDeploymentTarget'):
            self.assertEqual(settings[key], plist[key], key)


if __name__ == '__main__':
    unittest.main()
