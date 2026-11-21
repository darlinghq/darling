#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Builds HelveticaNeue.ttc, the "Helvetica Neue" family of the Darling prefix,
# from the four upright/italic faces of URW++ Nimbus Sans (see nimbus-sans/).
#
# The outlines, metrics and tables are the source font's. Only the 'name'
# table is rewritten: family, style, full and PostScript names become the
# macOS ones for the faces the source provides (Regular, Bold, Italic, Bold
# Italic), the original copyright notice is kept, and the description,
# licence and vendor entries say that the font is a renamed derivative.
#
# Usage: make-helvetica-neue.py <output.ttc> <NimbusSans-Regular.ttf> <NimbusSans-Bold.ttf>
#            <NimbusSans-Italic.ttf> <NimbusSans-BoldItalic.ttf>
# The sources must be given in that order; it is the order of the faces in the output.
# Requires the fontTools package.

import sys

from fontTools.ttLib import TTFont, TTCollection

FAMILY = "Helvetica Neue"

# (style name, full name suffix, PostScript name) per source, in output order.
FACES = [
    ("Regular", "", "HelveticaNeue"),
    ("Bold", " Bold", "HelveticaNeue-Bold"),
    ("Italic", " Italic", "HelveticaNeue-Italic"),
    ("Bold Italic", " Bold Italic", "HelveticaNeue-BoldItalic"),
]

# Modification notice (name ID 10). The date is the one on which the renamed
# derivative was first made; it is fixed so that the build is reproducible.
DESCRIPTION = (
    "Renamed derivative of Nimbus Sans, URW++ base 35 fonts "
    "(https://github.com/ArtifexSoftware/urw-base35-fonts). Modified on 2026-11-21 "
    "by the Darling project: the family, style, full and PostScript names were "
    "changed so that the font can stand in for the macOS family of this name; "
    "the glyph outlines and metrics are unchanged. This is not a font of the "
    "original URW++ foundry or of Apple, and the original authors do not "
    "support it."
)
LICENSE_DESCRIPTION = (
    "GNU Affero General Public License version 3 with the font exception "
    "that permits embedding the font in a Postscript or PDF document."
)
LICENSE_URL = "https://www.gnu.org/licenses/agpl-3.0.html"
VENDOR_URL = "https://github.com/ArtifexSoftware/urw-base35-fonts"

# name ID -> replacement; IDs not listed keep the source's value (0 is the
# copyright notice of the original authors).
def names_for(style, full_suffix, postscript, original):
    full = FAMILY + full_suffix
    unique = "Darling:%s:%s" % (postscript, original[3])
    return {
        1: FAMILY,
        2: style,
        3: unique,
        4: full,
        6: postscript,
        10: DESCRIPTION,
        11: VENDOR_URL,
        13: LICENSE_DESCRIPTION,
        14: LICENSE_URL,
    }


def rename(font, style, full_suffix, postscript):
    table = font["name"]
    original = {}
    for record in table.names:
        if record.platformID == 3 and record.langID == 0x409:
            original[record.nameID] = record.toUnicode()
    if 0 not in original or 3 not in original:
        raise SystemExit("source font lacks the copyright or unique name")

    replacements = names_for(style, full_suffix, postscript, original)
    # Drop the entries that are replaced or that no longer apply (typographic
    # family and style, compatible full name), keep everything else.
    table.names = [r for r in table.names
                   if r.nameID not in replacements and r.nameID not in (16, 17, 18, 21, 22, 25)]
    for name_id, value in replacements.items():
        # Windows Unicode English, and the Macintosh Roman platform that
        # CoreText-style readers use for the family and PostScript names.
        table.setName(value, name_id, 3, 1, 0x409)
        if all(ord(c) < 128 for c in value):
            table.setName(value, name_id, 1, 0, 0)


def main(argv):
    if len(argv) != 2 + len(FACES):
        sys.stderr.write(__doc__)
        return 2
    output = argv[1]
    fonts = []
    for path, (style, suffix, postscript) in zip(argv[2:], FACES):
        # recalcTimestamp=False keeps the head table, and so the output, reproducible.
        font = TTFont(path, recalcTimestamp=False)
        rename(font, style, suffix, postscript)
        fonts.append(font)
    collection = TTCollection()
    collection.fonts = fonts
    collection.save(output)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
