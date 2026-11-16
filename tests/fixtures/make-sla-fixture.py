#!/usr/bin/env python3
"""Writes the resource file (XML) that `hdiutil udifrez -xml` embeds into the sla-*.dmg fixtures: a licence
agreement in English (region code 0, resources 5000), German (region code 3, resources 5001) and Japanese
(region code 14, resources 5002, Shift JIS).

Usage: make-sla-fixture.py <output.xml> [<LPic first field> [good|short-lpic|overcount]]

The layout follows public descriptions of the UDIF licence resources ('LPic' language table, one 'STR#' with the
language name, button titles and prompt per language, and the agreement text as 'TEXT' in the language's Mac
encoding). The first LPic field (the default language) defaults to 5000. The "short-lpic" variant cuts the LPic
table off inside its first entry; "overcount" claims five entries in a table that has three.
"""
import plistlib
import struct
import sys

LANGUAGES = [
    # region code, resource ID, encoding, two-byte, strings (language, agree, disagree, print, save, prompt), text
    (0, 5000, "mac_roman", 0,
     ["English", "Agree", "Disagree", "Print", "Save...",
      "If you agree with the terms of this license, press \"Agree\" to install the software."],
     "SLA TEST AGREEMENT (English)\rThis is the English test agreement.\r"),
    (3, 5001, "mac_roman", 0,
     ["Deutsch", "Akzeptieren", "Ablehnen", "Drucken", "Sichern ...",
      "Klicken Sie auf „Akzeptieren“, wenn Sie einverstanden sind."],
     "SLA TEST AGREEMENT (Deutsch)\rDies ist die deutsche Testvereinbarung: Größe, Übung.\r"),
    (14, 5002, "shift_jis", 1,
     ["日本語", "同意します", "同意しません",
      "プリント", "保存...",
      "同意する場合は「同意します」をクリック"
      "してください。"],
     "SLA TEST AGREEMENT (日本語)\rこれはテスト契約です。\r"),
]


def resource(res_id, name, data):
    return {"Attributes": "0x0000", "Data": data, "ID": str(res_id), "Name": name}


def pascal(s, encoding):
    b = s.encode(encoding)
    return bytes([len(b)]) + b


def main():
    default = int(sys.argv[2]) if len(sys.argv) > 2 else 5000
    variant = sys.argv[3] if len(sys.argv) > 3 else "good"
    lpic = struct.pack(">HH", default, 5 if variant == "overcount" else len(LANGUAGES))
    strs, texts = [], []
    for region, res_id, encoding, two_byte, strings, text in LANGUAGES:
        lpic += struct.pack(">HHH", region, res_id - 5000, two_byte)
        strs.append(resource(res_id, "buttons",
                             struct.pack(">H", len(strings)) + b"".join(pascal(s, encoding) for s in strings)))
        texts.append(resource(res_id, "SLA", text.encode(encoding)))
    if variant == "short-lpic":
        lpic = lpic[:7]
    resources = {"LPic": [resource(5000, "", lpic)], "STR#": strs, "TEXT": texts}
    with open(sys.argv[1], "wb") as f:
        plistlib.dump(resources, f)


main()
