#!/usr/bin/env python3
"""Run LSGetApplicationForInfo's extracted extension SQL against its schema."""
import pathlib
import re
import sqlite3
import sys
import unittest

root = pathlib.Path(__file__).resolve().parents[1]
ls = root / "src/frameworks/CoreServices/src/LaunchServices"
source = pathlib.Path(sys.argv.pop(1)) if len(sys.argv) == 2 else ls / "LSInfo.m"
function = source.read_text().split("LSGetApplicationForInfo(", 1)[1].split(
    "CFURLRef LSCopyDefaultApplicationURLForContentType", 1)[0]
def strings(text): return "".join(re.findall(r'@"([^"\n]*)"', text))
base = strings(re.search(r'NSString\* query = (@"[^"\n]*");', function).group(1))
extension = strings(re.search(r'query = \[query stringByAppendingFormat:(.*?),\s*escaped, escaped\];', function, re.S).group(1))
ordering = strings(re.search(r'query = \[query stringByAppendingString:(.*?)\];', function, re.S).group(1))
query = base + extension + ordering

class ExtensionQuery(unittest.TestCase):
    def setUp(self):
        self.db = sqlite3.connect(":memory:")
        self.addCleanup(self.db.close)
        self.db.executescript((ls / "launchservicesd/schema.sql").read_text())
    def document(self, ident, cls, rank="Default", package="APPL"):
        self.db.execute("INSERT INTO bundle(id,path,bundle_id,checksum,package_type) VALUES(?,?,?,?,?)", (ident, f"/Apps/{ident}.app", f"test.{ident}", 1, package))
        self.db.execute("INSERT INTO app_doc(id,bundle,class,role,rank) VALUES(?,?,?,?,?)", (ident, ident, cls, "Editor", rank))
    def extension(self, ident, value):
        self.db.execute("INSERT INTO app_doc_extension(doc,extension) VALUES(?,?)", (ident, value))
    def lookup(self, value):
        sql = query.replace("%@", value.replace("'", "''"))
        return [row[0] for row in self.db.execute(sql)]
    def test_class_name_is_not_a_foreign_key(self):
        self.document(17, "FixtureDocument"); self.extension(17, "fixture")
        self.assertEqual(self.lookup("fixture"), ["/Apps/17.app"])
    def test_class_is_optional(self):
        self.document(18, None); self.extension(18, "none")
        self.assertEqual(self.lookup("none"), ["/Apps/18.app"])
    def test_numeric_class_cannot_select_another_document(self):
        self.document(19, "Other"); self.extension(19, "fixture")
        self.document(20, "19", "Owner")
        self.assertEqual(self.lookup("fixture"), ["/Apps/19.app"])
    def test_rank_and_application_filter_remain(self):
        for ident, rank in ((21, "Alternate"), (22, "Owner"), (23, "Default")):
            self.document(ident, "Document", rank); self.extension(ident, "fixture")
        self.document(24, "Document", "Owner", "BNDL"); self.extension(24, "fixture")
        self.assertEqual(self.lookup("fixture"), ["/Apps/22.app", "/Apps/23.app", "/Apps/21.app"])
    def test_uti_route_remains(self):
        self.document(25, "Document")
        self.db.execute("INSERT INTO uti(id,type_identifier,bundle) VALUES(1,'test.fixture',25)")
        self.db.execute("INSERT INTO uti_tag(uti,tag,value) VALUES(1,'public.filename-extension','uti')")
        self.db.execute("INSERT INTO app_doc_uti(doc,uti) VALUES(25,'test.fixture')")
        self.assertEqual(self.lookup("uti"), ["/Apps/25.app"])

unittest.main()
