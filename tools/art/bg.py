"""Fondos por capas (parallax) de 240 px de ancho y repetibles en horizontal.

Capas: cielo (opaco), lejos, medio, suelo, frente y niebla. Cada encuentro
desplaza las capas una cantidad distinta para que el paisaje varíe.
"""

import math
import random

from palettes import BRILLO, CLARO, KEY, MEDIO, NEGRO, OSCURO
from raster import Layer, dither

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


# Orden de dibujo; los personajes van entre "suelo" y "frente".
ORDER = ["cielo", "lejos", "niebla", "medio", "suelo", "frente"]
