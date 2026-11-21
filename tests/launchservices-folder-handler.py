#!/usr/bin/env python3
"""Run the query LSGetApplicationForURL uses for a directory's type (kDirectoryTypeHandlerQuery in LSInfo.m) against
launchservicesd's schema: a document type matches a type by LSItemContentTypes, by one of the type's OSType tags in
CFBundleTypeOSTypes (how Finder declares folders, 'fold', and volumes, 'disk', on macOS 26.7.3) or by one of its
filename-extension tags in CFBundleTypeExtensions; the usual handler ranks order the matches."""
import pathlib
import re
import sqlite3
import sys
import unittest

root = pathlib.Path(__file__).resolve().parents[1]
ls = root / "src/frameworks/CoreServices/src/LaunchServices"
source = pathlib.Path(sys.argv.pop(1)) if len(sys.argv) == 2 else ls / "LSInfo.m"
text = source.read_text()
match = re.search(r'kDirectoryTypeHandlerQuery = (.*?);', text, re.S)
query = "".join(re.findall(r'@"([^"\n]*)"', match.group(1))) if match else None


class DirectoryTypeQuery(unittest.TestCase):
    def setUp(self):
        self.db = sqlite3.connect(":memory:")
        self.addCleanup(self.db.close)
        self.db.executescript((ls / "launchservicesd/schema.sql").read_text())
        self.db.execute("INSERT INTO uti(id,type_identifier) VALUES(1,'public.folder')")
        self.db.execute("INSERT INTO uti_tag(uti,tag,value) VALUES(1,'com.apple.ostype','fold')")
        self.db.execute("INSERT INTO uti(id,type_identifier) VALUES(2,'public.volume')")
        self.db.execute("INSERT INTO uti_tag(uti,tag,value) VALUES(2,'com.apple.ostype','disk')")
        self.db.execute("INSERT INTO uti(id,type_identifier) VALUES(3,'com.apple.rtfd')")
        self.db.execute("INSERT INTO uti_tag(uti,tag,value) VALUES(3,'public.filename-extension','rtfd')")

    def document(self, ident, rank="Default", package="APPL"):
        self.db.execute("INSERT INTO bundle(id,path,bundle_id,checksum,package_type) VALUES(?,?,?,?,?)",
                        (ident, f"/Apps/{ident}.app", f"test.{ident}", 1, package))
        self.db.execute("INSERT INTO app_doc(id,bundle,role,rank) VALUES(?,?,?,?)", (ident, ident, "Editor", rank))

    def lookup(self, uti):
        self.assertIsNotNone(query, "LSInfo.m has no kDirectoryTypeHandlerQuery")
        return [row[0] for row in self.db.execute(query.replace("%@", ""), (uti, uti, uti))]

    def test_schema_version_is_3(self):
        self.assertEqual(self.db.execute("SELECT value FROM globals WHERE key='version'").fetchone(), ("3",))

    def test_ostype_table_exists(self):
        self.db.execute("SELECT id, doc, ostype FROM app_doc_ostype")

    def test_volume_by_ostype(self):
        self.document(1)
        self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(1,'disk')")
        self.assertEqual(self.lookup("public.volume"), ["/Apps/1.app"])
        self.assertEqual(self.lookup("public.folder"), [])

    def test_folder_by_content_type(self):
        self.document(2)
        self.db.execute("INSERT INTO app_doc_uti(doc,uti) VALUES(2,'public.folder')")
        self.assertEqual(self.lookup("public.folder"), ["/Apps/2.app"])

    def test_package_by_extension_tag(self):
        self.document(3)
        self.db.execute("INSERT INTO app_doc_extension(doc,extension) VALUES(3,'rtfd')")
        self.assertEqual(self.lookup("com.apple.rtfd"), ["/Apps/3.app"])

    def test_rank_orders_matches_and_only_applications_count(self):
        for ident, rank in ((4, "Alternate"), (5, "Default")):
            self.document(ident, rank)
            self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(?,'fold')", (ident,))
        self.document(6, "Owner", "BNDL")
        self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(6,'fold')")
        self.assertEqual(self.lookup("public.folder"), ["/Apps/5.app", "/Apps/4.app"])


unittest.main()
