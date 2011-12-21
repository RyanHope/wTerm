#!/usr/bin/python
import fontforge
import sys

print('Dumping glyphs for file: %s' % sys.argv[1])
f = fontforge.open(sys.argv[1])

glyphs = f.glyphs()
print glyphs

chars = ['a','b','c'];

lowercase = f.select(("ranges",None),"a","z")
uppercase = f.select(("ranges",None),"a","z")

for g in lowercase:
    print("Exporting glyph: %s" % g.unicode)
    g.export(g.unicode + '.bmp', 12, 1)
