"""Héroe: caballero oscuro de ~64 px de alto, mirando a la derecha.

Capas (de abajo arriba): cuerpo, botas, pechera, casco, escudo, brazo.
Las piezas de equipo usan la rampa MEDIO/CLARO/BRILLO, que el juego
recolorea según la rareza.
"""

import math

from palettes import (BRILLO, CLARO, ESPECTRAL, MEDIO, NEGRO, ORO, OSCURO,
                      SANGRE)
from raster import Layer

W, H = 80, 72
ANCHOR = (30, 71)  # pies (centro, fila inferior)

GEAR = (MEDIO, CLARO, BRILLO)
MAIL = (NEGRO, OSCURO, MEDIO)
CAPA = (OSCURO, SANGRE, SANGRE)

LAYERS = ["cuerpo", "botas", "pechera", "casco", "escudo", "brazo"]

# bx, by: desplazamiento del torso. hand: mano de la espada. dir: hoja.
POSES = [
    dict(bx=0, by=0, hand=(43, 32), dir=(0.55, -1)),     # idle 0
    dict(bx=0, by=1, hand=(43, 33), dir=(0.55, -1)),     # idle 1
    dict(bx=-1, by=0, hand=(21, 15), dir=(-1, -0.45)),   # attack: preparación
    dict(bx=3, by=1, hand=(50, 35), dir=(1, -0.08)),     # attack: tajo
    dict(bx=3, by=2, hand=(47, 46), dir=(1, 0.8)),       # attack: remate
    dict(bx=0, by=0, hand=(21, 9), dir=(0.18, -1)),      # charge
    dict(bx=-3, by=1, hand=(40, 35), dir=(0.9, -1)),     # hit
]
TAGS = [("idle", 0, 2, 450), ("attack", 2, 3, 90), ("charge", 5, 1, 100),
        ("hit", 6, 1, 200)]


def legs(p):
    """Cadera, rodilla y tobillo de cada pierna (atrás, delante)."""
    bx, by = p["bx"], p["by"]
    back = ((25 + bx, 46 + by), (23, 57), (22, 66))
    front = ((32 + bx, 46 + by), (36, 57), (37, 66))
    return back, front


def draw_cuerpo(L, p):
    bx, by = p["bx"], p["by"]
    o = lambda x, y: (x + bx, y + by)
    # capa
    L.part(lambda d: d.polygon([o(22, 25), o(30, 25), o(27, 44), (19, 64),
                                (7, 68), (4, 64), o(12, 44)], fill=1), CAPA)
    # piernas (malla)
    for hip, knee, ank in legs(p):
        L.part(lambda d, a=hip, b=knee, c=ank: (d.line([a, b], fill=1, width=7),
                                               d.line([b, c], fill=1, width=6)),
               MAIL)
    # torso
    L.part(lambda d: d.polygon([o(21, 25), o(37, 25), o(39, 45), o(20, 46)],
                               fill=1), MAIL)
    # faldón del tabardo
    L.part(lambda d: d.polygon([o(20, 42), o(39, 42), o(42, 54), o(31, 52),
                                o(18, 54)], fill=1), (NEGRO, OSCURO, OSCURO))


def draw_botas(L, p):
    for hip, knee, ank in legs(p):
        (kx, ky), (ax, ay) = knee, ank
        # greba ahusada (más ancha en la rodilla)
        L.part(lambda d, kx=kx, ky=ky, ax=ax, ay=ay: d.polygon(
            [(kx - 3, ky), (kx + 3, ky), (ax + 2, ay), (ax - 2, ay)], fill=1),
            GEAR, shadow_w=1)
        L.part(lambda d, kx=kx, ky=ky: d.polygon(
            [(kx - 3, ky - 3), (kx + 2, ky - 3), (kx + 4, ky + 1),
             (kx - 2, ky + 2)], fill=1), GEAR, shadow_w=1)
        ax, ay = ank
        L.part(lambda d, ax=ax, ay=ay: d.polygon(
            [(ax - 4, ay - 1), (ax + 3, ay - 1), (ax + 8, ay + 2),
             (ax + 8, ay + 4), (ax - 4, ay + 4)], fill=1), GEAR)


def draw_pechera(L, p):
    bx, by = p["bx"], p["by"]
    o = lambda x, y: (x + bx, y + by)
    L.part(lambda d: d.polygon([o(21, 25), o(37, 25), o(40, 35), o(37, 44),
                                o(22, 44), o(19, 34)], fill=1), GEAR)
    # cinturón con hebilla
    L.part(lambda d: d.rectangle([o(20, 43), o(39, 46)], fill=1),
           (NEGRO, OSCURO, OSCURO))
    L.flat(lambda d: d.rectangle([o(29, 43), o(31, 46)], fill=1), ORO)
    # hombrera trasera
    L.part(lambda d: d.ellipse([o(16, 22), o(28, 32)], fill=1), GEAR)


def draw_casco(L, p):
    bx, by = p["bx"], p["by"]
    o = lambda x, y: (x + bx, y + by)
    # penacho
    L.part(lambda d: d.polygon([o(27, 8), o(31, 4), o(26, 2), o(17, 5),
                                o(10, 12), o(13, 14), o(20, 10)], fill=1), CAPA)
    # yelmo
    L.part(lambda d: d.polygon([o(23, 9), o(33, 7), o(38, 11), o(39, 22),
                                o(35, 25), o(24, 25), o(21, 15)], fill=1), GEAR)
    # visera y respiraderos
    L.flat(lambda d: d.line([o(30, 15), o(38, 15)], fill=1), NEGRO)
    L.flat(lambda d: d.line([o(31, 16), o(38, 16)], fill=1), NEGRO)
    for i in range(3):
        L.flat(lambda d, i=i: d.point(o(34 + i * 2, 20)), NEGRO)
    # cresta
    L.flat(lambda d: d.line([o(26, 8), o(35, 9)], fill=1), BRILLO)


def draw_escudo(L, p):
    bx, by = p["bx"], p["by"]
    o = lambda x, y: (x + bx, y + by)
    L.part(lambda d: d.polygon([o(33, 28), o(47, 27), o(48, 40), o(41, 55),
                                o(34, 42)], fill=1), GEAR)
    # emblema: espada en cruz dorada
    L.flat(lambda d: d.line([o(41, 30), o(41, 49)], fill=1, width=2), ORO)
    L.flat(lambda d: d.line([o(36, 36), o(46, 36)], fill=1, width=2), ORO)


def draw_brazo(L, p):
    bx, by = p["bx"], p["by"]
    hx, hy = p["hand"]
    hy += by
    dx, dy = p["dir"]
    n = math.hypot(dx, dy)
    dx, dy = dx / n, dy / n
    px, py = -dy, dx
    sx, sy = 27 + bx, 28 + by          # hombro
    ex, ey = (sx + hx) / 2 - px * 3, (sy + hy) / 2 - py * 3  # codo

    def P(t, s=0.0):
        return (hx + dx * t + px * s, hy + dy * t + py * s)

    # hoja
    L.part(lambda d: d.polygon([P(3, -1.6), P(27, -1.6), P(31, 0), P(27, 1.6),
                                P(3, 1.6)], fill=1), GEAR, shadow_w=1)
    L.flat(lambda d: d.line([P(4, 0), P(26, 0)], fill=1), BRILLO)
    # guarda, empuñadura y pomo
    L.flat(lambda d: d.line([P(2, -5), P(2, 5)], fill=1, width=2), ORO)
    L.flat(lambda d: d.line([P(-1, 0), P(-4, 0)], fill=1, width=2), OSCURO)
    L.flat(lambda d: d.ellipse([P(-6, 0)[0] - 1.5, P(-6, 0)[1] - 1.5,
                                P(-6, 0)[0] + 1.5, P(-6, 0)[1] + 1.5],
                               fill=1), ORO)
    # brazo: malla + guantelete
    L.part(lambda d: d.line([(sx, sy), (ex, ey)], fill=1, width=5), MAIL)
    L.part(lambda d: d.line([(ex, ey), (hx, hy)], fill=1, width=5), GEAR)
    L.part(lambda d: d.ellipse([hx - 3, hy - 3, hx + 3, hy + 3], fill=1), GEAR)


DRAW = dict(cuerpo=draw_cuerpo, botas=draw_botas, pechera=draw_pechera,
            casco=draw_casco, escudo=draw_escudo, brazo=draw_brazo)


def build():
    """Devuelve {capa: [Layer por frame]}."""
    out = {}
    for name in LAYERS:
        frames = []
        for p in POSES:
            L = Layer(W, H)
            DRAW[name](L, p)
            L.outline()
            frames.append(L)
        out[name] = frames
    return out
