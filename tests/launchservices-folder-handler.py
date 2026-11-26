#!/usr/bin/env python3
"""Run the query LSGetApplicationForURL uses for a directory's type (kDirectoryTypeHandlerQuery in LSInfo.m) against
launchservicesd's schema: a document type matches a type by LSItemContentTypes, by one of the type's OSType tags in
CFBundleTypeOSTypes (how Finder declares folders, 'fold', and volumes, 'disk', on macOS 26.7.3) or by one of its
filename-extension tags in CFBundleTypeExtensions; the usual handler ranks order the matches, then the order of
registration. Also launchservicesd's upgrade of a version 2 database to version 3 (upgradeDBSchemaTo3 in LSBundle.m,
or in the file named by $LSBUNDLE)."""
import os
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
# launchservicesd's upgrade of an existing database to schema version 3 (LSBundle.m, or the file in $LSBUNDLE)
bundle_source = pathlib.Path(os.environ.get("LSBUNDLE", ls / "launchservicesd/LSBundle.m")).read_text()
upgrade = re.search(r'static void upgradeDBSchemaTo3\(void\)\s*\{.*?sqlite3_exec\([^,]*,(.*?),\s*NULL, NULL', bundle_source, re.S)
upgrade_sql = "".join(re.findall(r'"((?:[^"\\\n]|\\.)*)"', upgrade.group(1))) if upgrade else None


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

    def test_equal_ranks_prefer_the_bundle_registered_first(self):
        # Finder (registered with the system) stays the default for folders when an application claims
        # public.folder with the same rank (macOS 26.7.3, tests/volumes.md).
        for doc, bundle in ((1, 9), (2, 3)):
            self.db.execute("INSERT INTO bundle(id,path,bundle_id,checksum,package_type) VALUES(?,?,?,?,?)",
                            (bundle, f"/Apps/{bundle}.app", f"test.{bundle}", 1, "APPL"))
            self.db.execute("INSERT INTO app_doc(id,bundle,role,rank) VALUES(?,?,?,?)", (doc, bundle, "Editor", "Default"))
            self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(?,'fold')", (doc,))
        self.assertEqual(self.lookup("public.folder"), ["/Apps/3.app", "/Apps/9.app"])

    def test_rank_orders_matches_and_only_applications_count(self):
        for ident, rank in ((4, "Alternate"), (5, "Default")):
            self.document(ident, rank)
            self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(?,'fold')", (ident,))
        self.document(6, "Owner", "BNDL")
        self.db.execute("INSERT INTO app_doc_ostype(doc,ostype) VALUES(6,'fold')")
        self.assertEqual(self.lookup("public.folder"), ["/Apps/5.app", "/Apps/4.app"])


class UpgradeToVersion3(unittest.TestCase):
    """A version 2 database (schema.sql without app_doc_ostype) upgraded by launchservicesd has the version 3 schema
    and asks for every bundle to be registered again."""

    def schema(self, db):
        return sorted((row[0], row[1], "".join(row[2].split())) for row in
                      db.execute("SELECT type, name, sql FROM sqlite_master WHERE sql IS NOT NULL"))

    def test_upgrade_from_version_2(self):
        self.assertIsNotNone(upgrade_sql, "LSBundle.m has no upgradeDBSchemaTo3")
        fresh = sqlite3.connect(":memory:")
        self.addCleanup(fresh.close)
        fresh.executescript((ls / "launchservicesd/schema.sql").read_text())

        old = sqlite3.connect(":memory:", isolation_level=None)
        self.addCleanup(old.close)
        old.executescript((ls / "launchservicesd/schema.sql").read_text())
        old.executescript("DROP TABLE app_doc_ostype; UPDATE globals SET value = '2' WHERE key = 'version';")
        old.execute("INSERT INTO bundle(id,path,bundle_id,checksum,package_type) VALUES(1,'/Apps/1.app','test.1',42,'APPL')")

        old.executescript(upgrade_sql)
        self.assertEqual(self.schema(old), self.schema(fresh))
        self.assertEqual(old.execute("SELECT value FROM globals WHERE key='version'").fetchone(), ("3",))
        self.assertEqual(old.execute("SELECT checksum FROM bundle").fetchall(), [(0,)])


unittest.main()
