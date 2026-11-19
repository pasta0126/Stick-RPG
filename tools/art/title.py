"""Logotipo de la pantalla de título: «STICK» dorado en relieve, espada como
subrayado y «R P G» espaciado entre rombos."""

import os

from gfxfont import GFXFont
from palettes import BRILLO, CLARO, KEY, MEDIO, NEGRO, ORO, OSCURO, SANGRE
from raster import Layer, dither

HERE = os.path.dirname(os.path.abspath(__file__))
GFX = os.path.join(HERE, "..", "..",
                   ".pio/libdeps/m5stickc_plus2/M5GFX/src/lgfx/Fonts/GFXFF")

W, H = 208, 72


def text_mask(font, s, spacing):
    """Máscara de un texto con la fuente GFX (lista de (x, y))."""
    pts = []
    x = 0
    for ch in s:
        f = font.glyphs[ord(ch) - font.first]
        local = []
        font.draw(lambda px, py, c: local.append((px, py)), ch, x, 0, 1)
        pts += local
        x += f[3] + spacing
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    minx, miny = min(xs), min(ys)
    pts = [(px - minx, py - miny) for px, py in pts]
    return set(pts), max(xs) - minx + 1, max(ys) - miny + 1


def bevel(L, mask, ox, oy, h, base, light, shadow, deep):
    """Relleno con brillo arriba, sombra abajo y degradado tramado."""
    for (x, y) in mask:
        c = base
        if (x, y - 1) not in mask or (x, y - 2) not in mask and y < h * 0.4:
            c = light
        elif (x, y + 1) not in mask:
            c = deep
        elif (x, y + 2) not in mask or (y > h * 0.62 and dither(x, y, 0.5)):
            c = shadow
        L.put(x + ox, y + oy, c)


def build():
    L = Layer(W, H)
    big = GFXFont(os.path.join(GFX, "FreeSerifBold24pt7b.h"))
    small = GFXFont(os.path.join(GFX, "FreeSerifBold12pt7b.h"))

    # ---- STICK
    m, mw, mh = text_mask(big, "STICK", 3)
    ox, oy = (W - mw) // 2, 4
    # sombra proyectada
    for (x, y) in m:
        L.put(x + ox + 2, y + oy + 3, NEGRO)
    bevel(L, m, ox, oy, mh, ORO, BRILLO, SANGRE, NEGRO)
    stick_bottom = oy + mh

    # ---- espada como subrayado
    sy = stick_bottom + 6
    x0, x1 = 14, W - 12
    for x in range(x0 + 22, x1 - 6):  # hoja
        L.put(x, sy - 1, BRILLO)
        L.put(x, sy, CLARO)
        L.put(x, sy + 1, MEDIO)
    for k in range(6):  # punta
        L.put(x1 - 6 + k, sy - 1 + k // 3, CLARO if k < 5 else BRILLO)
        if k < 4:
            L.put(x1 - 6 + k, sy + 1 - k // 3, MEDIO)
    L.put(x1, sy, BRILLO)
    for y in range(sy - 5, sy + 6):  # guarda
        L.put(x0 + 20, y, ORO)
        L.put(x0 + 21, y, ORO if abs(y - sy) < 5 else SANGRE)
    L.put(x0 + 20, sy - 5, BRILLO)
    for x in range(x0 + 7, x0 + 20):  # empuñadura con vendas
        L.put(x, sy - 1, OSCURO)
        L.put(x, sy, MEDIO if x % 3 else OSCURO)
        L.put(x, sy + 1, OSCURO)
    for dy in range(-2, 3):  # pomo
        for dx in range(-2, 3):
            if dx * dx + dy * dy <= 5:
                L.put(x0 + 4 + dx, sy + dy, BRILLO if dx + dy < -1 else ORO)

    # ---- R P G entre rombos
    m2, w2, h2 = text_mask(small, "RPG", 10)
    ox2, oy2 = (W - w2) // 2, sy + 7
    bevel(L, m2, ox2, oy2, h2, CLARO, BRILLO, MEDIO, OSCURO)
    for cx in (ox2 - 16, ox2 + w2 + 15):
        cy = oy2 + h2 // 2
        for k in range(4):
            for j in range(-k, k + 1):
                L.put(cx + j, cy - 3 + k, ORO)
                L.put(cx + j, cy + 3 - k, ORO)
        L.put(cx, cy - 3, BRILLO)
        for x in range(cx - 12 if cx < W // 2 else cx + 5,
                       cx - 4 if cx < W // 2 else cx + 13):
            if x % 2 == 0:
                L.put(x, cy, MEDIO)

    L.outline(NEGRO)
    return L
