"""Fondos por capas (parallax) de 240 px de ancho y repetibles en horizontal.

Capas: cielo (opaco), lejos, medio, suelo, frente y niebla. Cada encuentro
desplaza las capas una cantidad distinta para que el paisaje varíe.
"""

import math
import random

from palettes import (BRILLO, CLARO, ESPECTRAL, KEY, MEDIO, NEGRO, ORO,
                      OSCURO, SANGRE)
from raster import Layer, dither, ellipse, flat_line, flat_poly, poly

W, H = 240, 135
SUELO_Y = 112  # línea donde apoyan los pies


def wrap_put(L, x, y, c):
    L.put(x % L.w, y, c)


def tree_dead(L, x, base, height, color, rng, rim=None):
    """Árbol muerto retorcido: tronco ahusado y ramas recursivas."""
    def branch(x0, y0, ang, length, width, depth):
        if depth == 0 or length < 3:
            return
        x1 = x0 + math.cos(ang) * length
        y1 = y0 - math.sin(ang) * length
        steps = int(length * 1.5) + 1
        for i in range(steps + 1):
            t = i / steps
            cx = x0 + (x1 - x0) * t
            cy = y0 + (y1 - y0) * t
            wd = max(1, width * (1 - t * 0.5))
            for dx in range(-int(wd // 2), int(math.ceil(wd / 2))):
                wrap_put(L, int(cx + dx), int(cy), color)
                if rim is not None and dx == int(math.ceil(wd / 2)) - 1 and width >= 3:
                    wrap_put(L, int(cx + dx), int(cy), rim)
        n = rng.choice([2, 2, 3])
        for k in range(n):
            na = ang + rng.uniform(-0.9, 0.9)
            branch(x1, y1, na, length * rng.uniform(0.55, 0.75),
                   max(1, width * 0.6), depth - 1)

    # tronco con raíces
    for r in range(3):
        dx = rng.randint(-6, 6)
        for i in range(6):
            wrap_put(L, x + dx * i // 6, base - i // 2, color)
    branch(x, base, math.pi / 2 + rng.uniform(-0.15, 0.15), height * 0.45,
           max(3, height // 9), 5)


def pine(L, x, base, height, color):
    width = height // 3
    for y in range(height):
        t = y / height
        # escalones de copa
        seg = (y % (height // 4 + 1)) / (height // 4 + 1)
        half = int(width * t * (0.6 + 0.4 * seg))
        for dx in range(-half, half + 1):
            wrap_put(L, x + dx, base - height + y, color)
    for y in range(4):
        wrap_put(L, x, base + y - 2, color)


def build_bosque(seed=7):
    rng = random.Random(seed)
    out = {}

    # ---- cielo: degradado + luna + bruma
    sky = Layer(W, H)
    for y in range(H):
        for x in range(W):
            if y < 34:
                c = NEGRO
            elif y < 44:
                c = OSCURO if dither(x, y, (y - 34) / 10) else NEGRO
            elif y < 84:
                c = OSCURO
            elif y < 94:
                c = MEDIO if dither(x, y, (y - 84) / 10 * 0.5) else OSCURO
            else:
                c = MEDIO if dither(x, y, 0.5 + (y - 94) / 8) else OSCURO
            sky.px[y][x] = c
    mx, my, r = 72, 33, 11
    for y in range(my - r - 4, my + r + 5):
        for x in range(mx - r - 4, mx + r + 5):
            d = math.hypot(x - mx, y - my)
            if d <= r:
                c = BRILLO
                if math.hypot(x - mx - 4, y - my + 2) < 3 or \
                   math.hypot(x - mx + 4, y - my - 4) < 2:
                    c = CLARO
                if d > r - 1.2 and x > mx:
                    c = CLARO
                sky.px[y][x] = c
            elif d <= r + 3 and dither(x, y, 0.3 * (1 - (d - r) / 3)):
                sky.px[y][x] = MEDIO
    # estrellas
    for _ in range(26):
        x, y = rng.randrange(W), rng.randrange(14, 60)
        if math.hypot(x - mx, y - my) > r + 8:
            sky.px[y][x] = CLARO if rng.random() < 0.4 else MEDIO
    out["cielo"] = sky

    # ---- lejos: línea de pinos
    far = Layer(W, H)
    x = 0
    while x < W:
        pine(far, x, 104 + rng.randint(-2, 3), rng.randint(22, 40), OSCURO)
        x += rng.randint(10, 18)
    out["lejos"] = far

    # ---- medio: árboles muertos retorcidos
    mid = Layer(W, H)
    for tx in (14, 70, 128, 196):
        tree_dead(mid, tx + rng.randint(-8, 8), SUELO_Y + 2,
                  rng.randint(80, 104), NEGRO, rng, rim=MEDIO)
    out["medio"] = mid

    # ---- suelo: tierra, piedras y raíces
    gr = Layer(W, H)
    for y in range(SUELO_Y - 2, H):
        for xx in range(W):
            t = (y - SUELO_Y) / (H - SUELO_Y)
            c = OSCURO if (y < SUELO_Y + 1 or dither(xx, y, 0.25 - t * 0.25)) else NEGRO
            gr.px[y][xx] = c
    for _ in range(40):  # matas en el borde superior
        xx = rng.randrange(W)
        h = rng.randint(2, 5)
        for k in range(h):
            wrap_put(gr, xx + rng.randint(-1, 1), SUELO_Y - 2 - k, NEGRO)
    for _ in range(14):  # piedras
        xx, yy = rng.randrange(W), rng.randint(SUELO_Y + 4, H - 4)
        for dx in range(-2, 3):
            wrap_put(gr, xx + dx, yy, MEDIO)
            wrap_put(gr, xx + dx, yy + 1, OSCURO)
        wrap_put(gr, xx - 1, yy - 1, MEDIO)
        wrap_put(gr, xx, yy - 1, MEDIO)
    out["suelo"] = gr

    # ---- frente: helechos negros en los extremos
    fg = Layer(W, H)
    for fx in (6, 120, 222):
        for k in range(7):
            ang = math.pi * (0.25 + k * 0.08)
            ln = rng.randint(10, 20)
            for i in range(ln):
                px = fx + int(math.cos(ang) * i * (1 if k % 2 else -1))
                py = H - int(math.sin(ang) * i) + (i * i) // 40
                wrap_put(fg, px, py, NEGRO)
                wrap_put(fg, px + 1, py, NEGRO)
    out["frente"] = fg

    # ---- niebla: banda tramada que se desplaza sola
    fog = Layer(W, H)
    for y in range(88, SUELO_Y + 4):
        for xx in range(W):
            band = math.sin(xx / W * math.pi * 4 + y * 0.3) * 0.5 + 0.5
            t = band * 0.14 * (1 - abs(y - 100) / 12)
            if t > 0 and dither(xx, y, t):
                fog.px[y][xx] = CLARO
    out["niebla"] = fog
    return out




# ------------------------------------------------------------- comunes
def night_sky(rng, mx, my, r, stars=26):
    sky = Layer(W, H)
    for y in range(H):
        for x in range(W):
            if y < 34:
                c = NEGRO
            elif y < 44:
                c = OSCURO if dither(x, y, (y - 34) / 10) else NEGRO
            elif y < 84:
                c = OSCURO
            elif y < 94:
                c = MEDIO if dither(x, y, (y - 84) / 10 * 0.5) else OSCURO
            else:
                c = MEDIO if dither(x, y, 0.5 + (y - 94) / 8) else OSCURO
            sky.px[y][x] = c
    for y in range(my - r - 4, my + r + 5):
        for x in range(mx - r - 4, mx + r + 5):
            d = math.hypot(x - mx, y - my)
            if d <= r:
                c = BRILLO
                if math.hypot(x - mx - 4, y - my + 2) < 3 or \
                   math.hypot(x - mx + 4, y - my - 4) < 2:
                    c = CLARO
                if d > r - 1.2 and x > mx:
                    c = CLARO
                sky.px[y][x] = c
            elif d <= r + 3 and dither(x, y, 0.3 * (1 - (d - r) / 3)):
                sky.px[y][x] = MEDIO
    for _ in range(stars):
        x, y = rng.randrange(W), rng.randrange(16, 60)
        if math.hypot(x - mx, y - my) > r + 8 and sky.px[y][x] in (NEGRO, OSCURO):
            sky.px[y][x] = CLARO if rng.random() < 0.4 else MEDIO
    return sky


def fog_layer(y0=88, y1=SUELO_Y + 4, color=CLARO, density=0.14):
    fog = Layer(W, H)
    mid = (y0 + y1) / 2
    half = (y1 - y0) / 2
    for y in range(y0, y1):
        for xx in range(W):
            band = math.sin(xx / W * math.pi * 4 + y * 0.3) * 0.5 + 0.5
            t = band * density * (1 - abs(y - mid) / half)
            if t > 0 and dither(xx, y, t):
                fog.px[y][xx] = color
    return fog


def dirt_ground(rng, stones=14):
    gr = Layer(W, H)
    for y in range(SUELO_Y - 2, H):
        for xx in range(W):
            t = (y - SUELO_Y) / (H - SUELO_Y)
            c = OSCURO if (y < SUELO_Y + 1 or dither(xx, y, 0.25 - t * 0.25)) else NEGRO
            gr.px[y][xx] = c
    for _ in range(40):
        xx = rng.randrange(W)
        for k in range(rng.randint(2, 5)):
            wrap_put(gr, xx + rng.randint(-1, 1), SUELO_Y - 2 - k, NEGRO)
    for _ in range(stones):
        xx, yy = rng.randrange(W), rng.randint(SUELO_Y + 4, H - 4)
        for dx in range(-2, 3):
            wrap_put(gr, xx + dx, yy, MEDIO)
            wrap_put(gr, xx + dx, yy + 1, OSCURO)
        wrap_put(gr, xx - 1, yy - 1, MEDIO)
        wrap_put(gr, xx, yy - 1, MEDIO)
    return gr


def grass_front(rng, xs):
    fg = Layer(W, H)
    for fx in xs:
        for k in range(7):
            ang = math.pi * (0.25 + k * 0.08)
            ln = rng.randint(10, 20)
            for i in range(ln):
                px = fx + int(math.cos(ang) * i * (1 if k % 2 else -1))
                py = H - int(math.sin(ang) * i) + (i * i) // 40
                wrap_put(fg, px, py, NEGRO)
                wrap_put(fg, px + 1, py, NEGRO)
    return fg


def tombstone(L, x, base, w, h, cross=False):
    if cross:
        poly(L, [(x - 1, base - h), (x + 2, base - h), (x + 2, base),
                 (x - 1, base)], (OSCURO, MEDIO, CLARO), shadow_w=1)
        poly(L, [(x - 5, base - h + 4), (x + 6, base - h + 4),
                 (x + 6, base - h + 7), (x - 5, base - h + 7)],
             (OSCURO, MEDIO, CLARO), shadow_w=1)
    else:
        L.part(lambda d: (d.rectangle([x - w // 2, base - h + w // 2, x + w // 2, base], fill=1),
                          d.ellipse([x - w // 2, base - h, x + w // 2, base - h + w], fill=1)),
               (OSCURO, MEDIO, CLARO), shadow_w=1)
        flat_line(L, [(x - 2, base - h + 6), (x + 2, base - h + 6)], OSCURO)
        flat_line(L, [(x, base - h + 4), (x, base - h + 9)], OSCURO)


# ------------------------------------------------------------ cementerio
def build_cementerio(seed=11):
    rng = random.Random(seed)
    out = {"cielo": night_sky(rng, 112, 30, 13)}

    far = Layer(W, H)
    # colinas
    for x in range(W):
        top = int(98 - 6 * math.sin(x / W * math.pi * 2) - 3 * math.sin(x / 17))
        for y in range(top, 108):
            far.px[y][x] = OSCURO
    # iglesia en ruinas
    flat_poly(far, [(150, 98), (150, 72), (162, 62), (174, 72), (174, 98)], OSCURO)
    flat_poly(far, [(174, 98), (174, 80), (204, 80), (204, 98)], OSCURO)
    flat_poly(far, [(158, 72), (162, 46), (166, 72)], OSCURO)
    flat_line(far, [(162, 40), (162, 48)], OSCURO)
    flat_line(far, [(159, 43), (165, 43)], OSCURO)
    flat_poly(far, [(160, 80), (164, 80), (164, 88), (160, 88)], ESPECTRAL)
    for x0 in range(180, 202, 7):
        flat_poly(far, [(x0, 84), (x0 + 3, 84), (x0 + 3, 90), (x0, 90)], NEGRO)
    out["lejos"] = far
    out["niebla"] = fog_layer(86, 110, CLARO, 0.18)

    mid = Layer(W, H)
    tree_dead(mid, 30, SUELO_Y + 2, 92, NEGRO, rng, rim=MEDIO)
    # verja de hierro
    for x in range(96, 236, 5):
        flat_line(mid, [(x, SUELO_Y), (x, SUELO_Y - 22)], NEGRO)
        flat_poly(mid, [(x - 1, SUELO_Y - 22), (x, SUELO_Y - 26), (x + 1, SUELO_Y - 22)], NEGRO)
    flat_line(mid, [(96, SUELO_Y - 18), (236, SUELO_Y - 18)], NEGRO)
    flat_line(mid, [(96, SUELO_Y - 6), (236, SUELO_Y - 6)], NEGRO)
    # tumbas
    for x, w, h, cross in ((64, 10, 16, False), (84, 0, 20, True),
                           (118, 12, 14, False), (210, 0, 18, True)):
        tombstone(mid, x, SUELO_Y + 1, w, h, cross)
    mid.outline()
    out["medio"] = mid
    out["suelo"] = dirt_ground(rng, 10)
    fg = grass_front(rng, (8, 230))
    tombstone(fg, 222, H + 2, 16, 22)
    out["frente"] = fg
    return out


# -------------------------------------------------------------- castillo
def build_castillo(seed=5):
    rng = random.Random(seed)
    wall = Layer(W, H)
    # muro de sillares
    for y in range(H):
        row = y // 9
        for x in range(W):
            off = 0 if row % 2 else 12
            mortar = y % 9 == 0 or (x + off) % 24 == 0
            c = NEGRO if mortar else OSCURO
            if not mortar and (x + y * 3) % 37 == 0:
                c = MEDIO
            wall.px[y][x] = c
    # ventanales góticos con luz espectral
    for cx in (60, 180):
        x0, x1, ytop, ybot = cx - 14, cx + 14, 22, 84
        for y in range(ytop - 14, ybot):
            for x in range(x0 - 3, x1 + 4):
                arch = (y >= ytop) or math.hypot(x - cx, (y - ytop) * 1.0) <= 17
                inner = (y >= ytop and x0 <= x <= x1) or \
                        math.hypot(x - cx, y - ytop) <= 14
                if arch and not inner:
                    wall.px[y][x] = MEDIO
                elif inner:
                    t = (y - (ytop - 14)) / (ybot - ytop + 14)
                    wall.px[y][x] = ESPECTRAL if dither(x, y, 0.5 - t * 0.45) else NEGRO
        for x in range(x0, x1 + 1):  # parteluz y travesaño
            pass
        for y in range(ytop - 12, ybot):
            wall.px[y][cx] = NEGRO
        for x in range(x0, x1 + 1):
            wall.px[50][x] = NEGRO
        for x in range(x0 - 4, x1 + 5):  # alféizar
            wall.px[ybot][x] = CLARO
            wall.px[ybot + 1][x] = MEDIO
    out = {"cielo": wall}

    far = Layer(W, H)  # arcadas lejanas
    for cx in range(0, W, 40):
        flat_poly(far, [(cx - 3, 112), (cx - 3, 70), (cx + 3, 70), (cx + 3, 112)], NEGRO)
    out["lejos"] = far
    out["niebla"] = fog_layer(92, 112, MEDIO, 0.2)

    mid = Layer(W, H)  # columnas con estandartes y antorchas
    for cx in (20, 120, 220):
        poly(mid, [(cx - 9, 112), (cx - 9, 16), (cx + 9, 16), (cx + 9, 112)],
             (NEGRO, OSCURO, MEDIO), shadow_w=3)
        poly(mid, [(cx - 12, 16), (cx + 12, 16), (cx + 12, 22), (cx - 12, 22)],
             (OSCURO, MEDIO, CLARO), shadow_w=1)
        poly(mid, [(cx - 12, 104), (cx + 12, 104), (cx + 12, 112), (cx - 12, 112)],
             (OSCURO, MEDIO, CLARO), shadow_w=1)
    for cx in (70, 170):  # estandartes
        poly(mid, [(cx - 8, 0), (cx + 8, 0), (cx + 8, 46), (cx, 40), (cx - 8, 46)],
             (OSCURO, SANGRE, SANGRE))
        flat_poly(mid, [(cx - 3, 14), (cx + 3, 14), (cx + 3, 22), (cx, 26), (cx - 3, 22)], ORO)
    for cx in (20, 220):  # antorchas
        flat_poly(mid, [(cx - 2, 60), (cx + 2, 60), (cx + 1, 68), (cx - 1, 68)], NEGRO)
        ellipse(mid, cx, 55, 3, 5, (SANGRE, ORO, BRILLO), contour=None, shadow_w=1)
    mid.outline()
    out["medio"] = mid

    gr = Layer(W, H)  # losas
    for y in range(SUELO_Y - 1, H):
        band = (y - SUELO_Y) // 7
        for x in range(W):
            off = (band % 2) * 15
            joint = (y - SUELO_Y) % 7 == 0 or (x + off) % 30 == 0
            gr.px[y][x] = NEGRO if joint else (MEDIO if y == SUELO_Y - 1 else OSCURO)
    out["suelo"] = gr

    fg = Layer(W, H)  # escombros y cadenas
    for x0 in (4, 226):
        for i in range(6):
            fx, fy = x0 + rng.randint(-4, 8), H - rng.randint(1, 8)
            poly(fg, [(fx, fy), (fx + 5, fy - 2), (fx + 7, fy + 3), (fx + 1, fy + 5)],
                 (NEGRO, OSCURO, MEDIO), shadow_w=1)
    for y in range(0, 34, 3):
        flat_line(fg, [(6, y), (6, y + 2)], NEGRO)
        flat_line(fg, [(5, y + 1), (7, y + 1)], NEGRO)
    fg.outline()
    out["frente"] = fg
    return out


BUILDERS = {"bosque": build_bosque, "cementerio": build_cementerio,
            "castillo": build_castillo}

# Orden de dibujo; los personajes van entre "suelo" y "frente".
ORDER = ["cielo", "lejos", "niebla", "medio", "suelo", "frente"]
# Desplazamiento por encuentro (px) de cada capa: parallax.
PARALLAX = {"cielo": 0, "lejos": 17, "niebla": 29, "medio": 53, "suelo": 89,
            "frente": 131}
