"""Lee fuentes GFX de Adafruit (.h) como las que incluye M5GFX, para
reproducir en el PC el texto tal y como se verá en el dispositivo."""

import re


class GFXFont:
    def __init__(self, path):
        src = open(path).read()
        bm = src.split("Bitmaps[]")[1].split("};")[0]
        self.bitmap = [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]{2})", bm)]
        gl = src.split("Glyphs[]")[1].split("};")[0]
        self.glyphs = [tuple(int(v) for v in g)
                       for g in re.findall(r"\{\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),"
                                           r"\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\}", gl)]
        tail = src.split("Glyphs,")[1]
        nums = re.findall(r"0x([0-9A-Fa-f]+)", tail)
        self.first, self.last = int(nums[0], 16), int(nums[1], 16)
        self.y_advance = int(re.findall(r"(\d+)\s*\}", tail)[0])

    def width(self, s):
        return sum(self.glyphs[ord(c) - self.first][3] for c in s
                   if self.first <= ord(c) <= self.last)

    def draw(self, put, s, x, baseline, color):
        for c in s:
            if not self.first <= ord(c) <= self.last:
                continue
            off, w, h, adv, xo, yo = self.glyphs[ord(c) - self.first]
            bit = 0
            for yy in range(h):
                for xx in range(w):
                    byte = self.bitmap[off + bit // 8]
                    if byte & (0x80 >> (bit % 8)):
                        put(x + xo + xx, baseline + yo + yy, color)
                    bit += 1
            x += adv
        return x
