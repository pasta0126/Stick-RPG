"""Enemigos. Se dibujan mirando a la derecha y se voltean al exportar
(miran al héroe, a la izquierda). Una sola capa por enemigo.

Animaciones comunes: walk (entrada), idle, attack, hit, prepare.
"""

import math

from palettes import (BRILLO, CLARO, ESPECTRAL, MEDIO, NEGRO, ORO, OSCURO,
                      SANGRE)
from raster import Layer

ABRIGO = (OSCURO, MEDIO, CLARO)
CUERO = (NEGRO, OSCURO, MEDIO)
ACERO = (MEDIO, CLARO, BRILLO)
TELA_ROJA = (OSCURO, SANGRE, SANGRE)

TAGS = [("walk", 0, 2, 160), ("idle", 2, 2, 450), ("attack", 4, 2, 110),
        ("hit", 6, 1, 200), ("prepare", 7, 1, 200)]


def flip(L):
    out = Layer(L.w, L.h)
    for y in range(L.h):
        out.px[y] = list(reversed(L.px[y]))
    return out


# ------------------------------------------------------------- Salteador
class Salteador:
    name = "salteador"
    W, H = 64, 72
    ANCHOR = (32, 71)
    POSES = [
        dict(bx=0, by=0, legs=(-4, 3), hand=(44, 41), dag=(0.6, -1)),   # walk 0
        dict(bx=0, by=-1, legs=(3, -3), hand=(44, 40), dag=(0.6, -1)),  # walk 1
        dict(bx=0, by=0, legs=(-2, 2), hand=(44, 41), dag=(0.6, -1)),   # idle 0
        dict(bx=0, by=1, legs=(-2, 2), hand=(44, 42), dag=(0.6, -1)),   # idle 1
        dict(bx=2, by=1, legs=(-4, 4), hand=(50, 38), dag=(1, -0.1)),   # attack 0
        dict(bx=4, by=2, legs=(-5, 5), hand=(54, 40), dag=(1, 0.15)),   # attack 1
        dict(bx=-3, by=1, legs=(-1, 1), hand=(40, 44), dag=(0.3, -1)),  # hit
        dict(bx=-2, by=3, legs=(-5, 4), hand=(22, 26), dag=(-0.4, -1)), # prepare
    ]

    @classmethod
    def draw(cls, p):
        L = Layer(cls.W, cls.H)
        bx, by = p["bx"], p["by"]
        o = lambda x, y: (x + bx, y + by)
        lb, lf = p["legs"]
        # piernas y botas
        for hip, foot in (((27, 54), (27 + lb, 66)), ((34, 54), (34 + lf, 66))):
            L.part(lambda d, h=hip, f=foot: d.line([o(*h), f], fill=1, width=6),
                   CUERO)
            fx, fy = foot
            L.part(lambda d, fx=fx, fy=fy: d.polygon(
                [(fx - 3, fy - 3), (fx + 3, fy - 3), (fx + 6, fy + 2),
                 (fx + 6, fy + 4), (fx - 3, fy + 4)], fill=1), CUERO)
        # cola de la bufanda
        L.part(lambda d: d.polygon([o(24, 29), o(14, 33), o(8, 40), o(12, 41),
                                    o(18, 36), o(25, 33)], fill=1), TELA_ROJA)
        # abrigo largo
        L.part(lambda d: d.polygon([o(23, 27), o(38, 27), o(42, 44), o(44, 58),
                                    o(30, 56), o(16, 59), o(19, 42)], fill=1),
               ABRIGO)
        # cinturón
        L.part(lambda d: d.polygon([o(19, 42), o(41, 42), o(41, 45), o(19, 45)],
                                   fill=1), (NEGRO, NEGRO, OSCURO))
        L.flat(lambda d: d.rectangle([o(30, 42), o(32, 45)], fill=1), ORO)
        # capucha
        L.part(lambda d: d.polygon([o(25, 9), o(34, 7), o(41, 13), o(42, 25),
                                    o(37, 29), o(24, 29), o(20, 18)], fill=1),
               ABRIGO)
        # rostro en sombra y ojos
        L.flat(lambda d: d.polygon([o(32, 14), o(40, 15), o(41, 25), o(33, 26),
                                    o(31, 20)], fill=1), NEGRO)
        L.flat(lambda d: d.line([o(35, 19), o(36, 19)], fill=1), ESPECTRAL)
        L.flat(lambda d: d.point(o(39, 19)), ESPECTRAL)
        # bufanda
        L.part(lambda d: d.polygon([o(23, 27), o(40, 26), o(41, 31), o(24, 32)],
                                   fill=1), TELA_ROJA)
        # brazo y daga
        hx, hy = p["hand"]
        hy += by
        dx, dy = p["dag"]
        n = math.hypot(dx, dy)
        dx, dy = dx / n, dy / n
        px, py = -dy, dx
        P = lambda t, s=0.0: (hx + dx * t + px * s, hy + dy * t + py * s)
        L.part(lambda d: d.polygon([P(2, -1.5), P(12, -2), P(17, 1), P(12, 1.5),
                                    P(2, 1.5)], fill=1), ACERO, shadow_w=1)
        L.flat(lambda d: d.line([P(1, -3), P(1, 3)], fill=1, width=2), ORO)
        sx, sy = 35 + bx, 31 + by
        L.part(lambda d: d.line([(sx, sy), (hx, hy)], fill=1, width=5), ABRIGO)
        L.part(lambda d: d.ellipse([hx - 3, hy - 3, hx + 3, hy + 3], fill=1),
               CUERO)
        L.outline()
        return L


ENEMIES = [Salteador]


def build(cls):
    return [flip(cls.draw(p)) for p in cls.POSES]
