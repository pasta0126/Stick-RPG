"""Captura simulada de la pantalla de combate (240x135) para revisar la
dirección de arte en el PC. Uso: python3 mock.py <salida.png> [bioma]"""

import os
import sys

from PIL import Image

import bg
import enemies
import font
import hero
from gfxfont import GFXFont
from palettes import (BIOMAS, BRILLO, CLARO, ESPECTRAL, KEY, MEDIO, NEGRO, ORO,
                      OSCURO, SANGRE)
from raster import Layer, dither

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
GFX = os.path.join(ROOT, ".pio/libdeps/m5stickc_plus2/M5GFX/src/lgfx/Fonts/GFXFF")


class Screen(Layer):
    def text(self, s, x, y, color, shadow=True, align="left"):
        if align == "center":
            x -= font.text_width(s) // 2
        elif align == "right":
            x -= font.text_width(s)
        if shadow:
            font.draw(self.put, s, x + 1, y + 1, NEGRO)
        font.draw(self.put, s, x, y, color)

    def serif(self, f, s, cx, baseline, color):
        x = cx - f.width(s) // 2
        for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1), (1, 1)):
            f.draw(self.put, s, x + dx, baseline + dy, NEGRO)
        f.draw(self.put, s, x, baseline, color)

    def rect(self, x0, y0, x1, y1, c):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.put(x, y, c)

    def panel(self, x0, y0, x1, y1):
        """Placa de hierro: fondo negro, marco medio con brillo y remaches."""
        self.rect(x0, y0, x1, y1, NEGRO)
        for x in range(x0 + 1, x1):
            self.put(x, y0 + 1, CLARO)
            self.put(x, y1 - 1, OSCURO)
        for y in range(y0 + 1, y1):
            self.put(x0 + 1, y, MEDIO)
            self.put(x1 - 1, y, OSCURO)
        for x, y in ((x0 + 1, y0 + 1), (x1 - 1, y0 + 1), (x0 + 1, y1 - 1),
                     (x1 - 1, y1 - 1)):
            self.put(x, y, ORO)

    def bar(self, x0, y0, w, h, frac, color):
        self.rect(x0, y0, x0 + w - 1, y0 + h - 1, OSCURO)
        fill = int((w - 2) * frac)
        for x in range(x0 + 1, x0 + 1 + fill):
            for y in range(y0 + 1, y0 + h - 1):
                self.put(x, y, color)
            if (x - x0) % 6 == 0:
                for y in range(y0 + 1, y0 + h - 1):
                    self.put(x, y, NEGRO)
        for x in range(x0 + 1, x0 + 1 + fill):
            self.put(x, y0 + 1, BRILLO if color == SANGRE and x % 2 else color)

    def icon(self, rows, x, y, cmap):
        for j, row in enumerate(rows):
            for i, ch in enumerate(row):
                if ch in cmap:
                    self.put(x + i, y + j, cmap[ch])


HEART = [".##.##.", "#rr#rr#", "#rrrrr#", ".#rrr#.", "..#r#..", "...#..."]
SKULL = [".#####.", "#ccccc#", "#c#c#c#", "#ccccc#", ".#c#c#.", "..###.."]
RUNE = ["...#...", "..#o#..", ".#ooo#.", "#ooooo#", ".#ooo#.", "..#o#..", "...#..."]


def roman(n):
    vals = [(1000, "M"), (900, "CM"), (500, "D"), (400, "CD"), (100, "C"),
            (90, "XC"), (50, "L"), (40, "XL"), (10, "X"), (9, "IX"), (5, "V"),
            (4, "IV"), (1, "I")]
    out = ""
    for v, s in vals:
        while n >= v:
            out += s
            n -= v
    return out


def render(bioma="bosque", nivel=7, msg="CRITICO!", charging=0.7, enemy=None):
    pal = BIOMAS[bioma]
    S = Screen(240, 135)
    layers = bg.BUILDERS[bioma]()
    offs = {k: v * nivel for k, v in bg.PARALLAX.items()}
    for name in bg.ORDER:
        L = layers[name]
        for y in range(L.h):
            for x in range(L.w):
                c = L.px[y][(x + offs[name]) % L.w]
                if c != KEY:
                    S.put(x, y, c)
        if name == "suelo":
            # personajes entre el suelo y el primer plano
            hl = hero.build()
            for lname in hero.LAYERS:
                fr = hl[lname][0]
                S.blit(fr, 50 - hero.ANCHOR[0], 113 - hero.ANCHOR[1])
            e = enemy or enemies.Salteador
            fr = enemies.build(e)[2]
            S.blit(fr, 186 - (e.W - 1 - e.ANCHOR[0]), 113 - e.ANCHOR[1])

    # ---- HUD superior
    S.panel(1, 1, 92, 21)
    S.icon(HEART, 5, 5, {"#": NEGRO, "r": SANGRE})
    S.text("62/80", 15, 5, BRILLO)
    for i in range(3):
        on = i < 3
        S.icon(RUNE, 60 + i * 10, 4, {"#": MEDIO if on else OSCURO,
                                      "o": ESPECTRAL if on else NEGRO})
    S.bar(4, 14, 86, 5, 62 / 80, SANGRE)

    S.panel(147, 1, 238, 21)
    S.icon(SKULL, 228, 5, {"#": NEGRO, "c": CLARO})
    S.text((enemy or enemies.Salteador).name.replace("_", " ").upper(), 224, 5, BRILLO, align="right")
    S.bar(150, 14, 86, 5, 0.45, SANGRE)

    S.panel(102, 1, 137, 13)
    S.text(roman(nivel), 120, 4, ORO, align="center")

    # ---- mensaje
    if msg:
        f = GFXFont(os.path.join(GFX, "FreeSerifBold9pt7b.h"))
        S.serif(f, msg, 120, 58, ORO)

    # ---- barra de carga
    if charging is not None:
        x0, y0, w = 60, 122, 120
        S.panel(x0 - 3, y0 - 3, x0 + w + 2, y0 + 9)
        zone = int(w * 0.92)
        S.rect(x0, y0, x0 + w - 1, y0 + 6, OSCURO)
        S.rect(x0 + zone, y0, x0 + w - 1, y0 + 6, NEGRO)
        for x in range(x0 + zone, x0 + w):
            for y in range(y0, y0 + 7):
                if dither(x, y, 0.5):
                    S.put(x, y, ORO)
        S.rect(x0, y0 + 1, x0 + int(w * charging), y0 + 5, ESPECTRAL)
        S.rect(x0 + zone, y0 - 2, x0 + zone, y0 + 8, ORO)

    return S.to_image(pal)


if __name__ == "__main__":
    out = sys.argv[1]
    img = render(*(sys.argv[2:3] or ["bosque"]))
    img.resize((img.width * 4, img.height * 4), Image.NEAREST).save(out)
    img.save(out.replace(".png", "_x1.png"))
