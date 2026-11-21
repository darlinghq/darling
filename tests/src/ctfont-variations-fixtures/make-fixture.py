# Generates DarlingVariableTest.ttf, the variable-font fixture for
# ctfont_variations.m. Requires fontTools (MIT). Usage: python3 make-fixture.py outdir
# Copyright (C) 2026 Lubos Dolezel. Released under the SIL OFL 1.1 (see OFL.txt).
import sys, os
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.designspaceLib import DesignSpaceDocument, AxisDescriptor, SourceDescriptor, InstanceDescriptor
from fontTools import varLib

out = sys.argv[1]
os.makedirs(out, exist_ok=True)

def master(width, path):
    fb = FontBuilder(1000, isTTF=True)
    fb.setupGlyphOrder([".notdef", "A"])
    fb.setupCharacterMap({0x41: "A"})
    glyphs = {}
    for g in (".notdef", "A"):
        pen = TTGlyphPen(None)
        pen.moveTo((50, 0)); pen.lineTo((50, 700)); pen.lineTo((width - 50, 700)); pen.lineTo((width - 50, 0))
        pen.closePath()
        glyphs[g] = pen.glyph()
    fb.setupGlyf(glyphs)
    fb.setupHorizontalMetrics({".notdef": (width, 50), "A": (width, 50)})
    fb.setupHorizontalHeader(ascent=800, descent=-200)
    fb.setupNameTable({"familyName": "Darling Variable Test", "styleName": "Regular"})
    fb.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=800, usWinDescent=200)
    fb.setupPost()
    fb.save(path)

ds = DesignSpaceDocument()
w = AxisDescriptor(); w.name = "weight"; w.tag = "wght"; w.minimum = 100; w.default = 400; w.maximum = 900
w.labelNames = {"en": "Weight"}
h = AxisDescriptor(); h.name = "hidden"; h.tag = "HIDE"; h.minimum = 0; h.default = 0; h.maximum = 1
h.labelNames = {"en": "Hidden Axis"}; h.hidden = True
ds.addAxis(w); ds.addAxis(h)
for wt, adv in ((100, 400), (400, 600), (900, 900)):
    p = os.path.join(out, "master%d.ttf" % wt)
    master(adv, p)
    s = SourceDescriptor(); s.path = p; s.name = "m%d" % wt
    s.location = {"weight": wt, "hidden": 0}
    if wt == 400: s.copyInfo = True
    ds.addSource(s)
ds.write(os.path.join(out, "f.designspace"))
vf, _, _ = varLib.build(ds.path if hasattr(ds, "path") else os.path.join(out, "f.designspace"))
vf.save(os.path.join(out, "DarlingVariableTest.ttf"))
# The static face: the default master, without variation tables.
static = os.path.join(out, "DarlingStaticTest.ttf")
os.replace(os.path.join(out, "master400.ttf"), static)
