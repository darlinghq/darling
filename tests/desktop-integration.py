#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""The files that let a Linux desktop open disk images with Darling: the desktop entry, the shared-mime-info package
and the icons that src/DiskImageMounter installs (component gui). Host-only; needs a configured build tree.

Usage: desktop-integration.py <build dir> <install prefix>

Installs DiskImageMounter's directory of the build into a scratch DESTDIR under the build tree, then checks the files
against the Desktop Entry and Shared MIME-info specifications (freedesktop.org) with the host's own tools:
desktop-file-validate, update-mime-database, update-desktop-database and gio. A missing tool skips its check.
"""
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

BUILD = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else None
PREFIX = sys.argv[2] if len(sys.argv) > 2 else None
ID = "org.darlinghq.Darling.DiskImageMounter"
TYPE = "application/x-apple-diskimage"
SIZES = (16, 32, 128, 256, 512)
FREEDESKTOP = Path("/usr/share/mime/packages/freedesktop.org.xml")
HAVE_MIME = shutil.which("update-mime-database") is not None


def desktop_exec_args(value):
    """Splits an Exec value as the Desktop Entry specification quotes it"""
    args, cur, quoted, escaped, started = [], "", False, False, False
    for ch in value:
        if escaped:
            cur += ch
            escaped = False
        elif quoted and ch == "\\":
            escaped = True
        elif ch == '"':
            quoted = not quoted
            started = True
        elif ch == " " and not quoted:
            if started or cur:
                args.append(cur)
            cur, started = "", False
        else:
            cur += ch
    if started or cur:
        args.append(cur)
    return args


class DesktopIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix="desktop-integration-", dir=BUILD)
        cls.dest = Path(cls.tmp.name)
        result = subprocess.run(["cmake", "-DCOMPONENT=gui", "-P", str(BUILD / "src/DiskImageMounter/cmake_install.cmake")],
                                env=dict(os.environ, DESTDIR=str(cls.dest)), cwd=BUILD,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        if result.returncode != 0:
            raise RuntimeError("install failed:\n" + result.stdout)
        cls.share = Path(str(cls.dest) + PREFIX) / "share"

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def entry(self):
        return self.share / "applications" / (ID + ".desktop")

    def entry_keys(self):
        keys = {}
        for line in self.entry().read_text().splitlines():
            if "=" in line and not line.startswith("#"):
                key, value = line.split("=", 1)
                keys[key.strip()] = value.strip()
        return keys

    def scratch_mime(self, with_freedesktop):
        root = Path(tempfile.mkdtemp(dir=self.dest))
        packages = root / "mime" / "packages"
        packages.mkdir(parents=True)
        shutil.copy(self.share / "mime" / "packages" / (ID + ".xml"), packages)
        if with_freedesktop:
            shutil.copy(FREEDESKTOP, packages)
        subprocess.run(["update-mime-database", str(root / "mime")], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return root / "mime"

    def test_files(self):
        self.assertTrue(self.entry().is_file(), self.entry())
        self.assertTrue((self.share / "mime" / "packages" / (ID + ".xml")).is_file())
        for n in SIZES:
            png = self.share / "icons" / "hicolor" / f"{n}x{n}" / "apps" / (ID + ".png")
            data = png.read_bytes()
            self.assertEqual(data[:8], b"\x89PNG\r\n\x1a\n", png)
            self.assertEqual(struct.unpack(">II", data[16:24]), (n, n), png)

    def test_desktop_entry(self):
        if shutil.which("desktop-file-validate"):
            result = subprocess.run(["desktop-file-validate", str(self.entry())],
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertNotIn("error", result.stdout)
        keys = self.entry_keys()
        self.assertEqual(keys.get("Type"), "Application")
        self.assertEqual(keys.get("MimeType"), TYPE + ";")
        self.assertEqual(keys.get("NoDisplay"), "true")
        self.assertEqual(keys.get("Terminal"), "false")
        self.assertEqual(keys.get("Icon"), ID)
        self.assertEqual(desktop_exec_args(keys.get("Exec", "")), [PREFIX + "/bin/darling", "open", "%F"])

    @unittest.skipUnless(HAVE_MIME, "no update-mime-database (shared-mime-info)")
    def test_mime_alone(self):
        globs = (self.scratch_mime(False) / "globs2").read_text()
        self.assertIn(f":{TYPE}:*.dmg", globs)

    @unittest.skipUnless(HAVE_MIME and FREEDESKTOP.is_file(), "no shared-mime-info on this host")
    def test_mime_with_freedesktop(self):
        mime = self.scratch_mime(True)
        lines = [line for line in (mime / "globs2").read_text().splitlines() if line.endswith(":*.dmg")]
        # update-mime-database keeps one line per package that defines the pattern; identical lines (same weight,
        # same type) can't disagree, so what matters is that every one names this type
        self.assertTrue(lines, "no *.dmg pattern")
        self.assertEqual(lines, [l for l in lines if l.split(":")[1] == TYPE], lines)
        self.assertIn('xml:lang="de"', (mime / "application" / "x-apple-diskimage.xml").read_text())

    @unittest.skipUnless(HAVE_MIME and shutil.which("gio") and shutil.which("update-desktop-database"),
                         "no gio, update-desktop-database or update-mime-database")
    def test_handler(self):
        mime = self.scratch_mime(False)
        share = mime.parent
        shutil.copytree(self.share / "applications", share / "applications")
        subprocess.run(["update-desktop-database", str(share / "applications")], check=True)
        env = dict(os.environ, XDG_DATA_HOME=str(share), XDG_DATA_DIRS=str(share), XDG_CONFIG_HOME=str(share / "config"),
                   XDG_CONFIG_DIRS=str(share / "config"))
        result = subprocess.run(["gio", "mime", TYPE], env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        self.assertIn(ID + ".desktop", result.stdout)


if __name__ == "__main__":
    if BUILD is None or PREFIX is None:
        sys.exit(__doc__)
    unittest.main(argv=sys.argv[:1])
