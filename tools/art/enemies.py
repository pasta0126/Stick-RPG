"""Enemigos y jefes. Se dibujan mirando a la derecha y se voltean al exportar
(miran al héroe, a la izquierda). Una sola capa por enemigo.

Poses comunes (8 frames):
  walk0 walk1 | idle0 idle1 (respiración) | atk0 atk1 | hit | prep
"""

import math

from palettes import (BRILLO, CLARO, ESPECTRAL, MEDIO, NEGRO, ORO, OSCURO,
                      SANGRE)
from raster import (Layer, dot, ellipse, flat_line, flat_poly, limb, poly)

POSES = ["walk0", "walk1", "idle0", "idle1", "atk0", "atk1", "hit", "prep"]
TAGS = [("walk", 0, 2, 180), ("idle", 2, 2, 650), ("attack", 4, 2, 120),
        ("hit", 6, 1, 200), ("prepare", 7, 1, 200)]

PIEDRA = (OSCURO, MEDIO, CLARO)
PIEL = (OSCURO, MEDIO, CLARO)
CUERO = (NEGRO, OSCURO, MEDIO)
OSCURA = (NEGRO, OSCURO, OSCURO)
ACERO = (MEDIO, CLARO, BRILLO)
HUESO = (MEDIO, CLARO, BRILLO)
ROJA = (OSCURO, SANGRE, SANGRE)
NEGRA = (NEGRO, OSCURO, MEDIO)


def flip(L):
    out = Layer(L.w, L.h)
    for y in range(L.h):
        out.px[y] = list(reversed(L.px[y]))
    return out


def unit(dx, dy):
    n = math.hypot(dx, dy)
    return dx / n, dy / n


def blade(L, hx, hy, d, length, width, ramp=ACERO, tip=4, guard=ORO,
          guard_len=5, grip=OSCURO):
    dx, dy = unit(*d)
    px, py = -dy, dx
    P = lambda t, s=0.0: (hx + dx * t + px * s, hy + dy * t + py * s)
    hw = width / 2
    poly(L, [P(3, -hw), P(length - tip, -hw), P(length, 0),
             P(length - tip, hw), P(3, hw)], ramp, shadow_w=1)
    flat_line(L, [P(4, 0), P(length - tip, 0)], BRILLO)
    if guard is not None:
        flat_line(L, [P(2, -guard_len), P(2, guard_len)], guard, width=2)
    if grip is not None:
        flat_line(L, [P(-1, 0), P(-5, 0)], grip, width=2)


def breathe(pose):
    return 1 if pose in ("idle1", "walk1") else 0


# =============================================================== normales
class Salteador:
    name = "salteador"
    W, H = 64, 72
    ANCHOR = (32, 71)

    @classmethod
    def draw(cls, pose):
        P = dict(
            walk0=dict(bx=0, by=0, legs=(-4, 3), hand=(44, 41), dag=(0.6, -1)),
            walk1=dict(bx=0, by=-1, legs=(3, -3), hand=(44, 40), dag=(0.6, -1)),
            idle0=dict(bx=0, by=0, legs=(-2, 2), hand=(44, 41), dag=(0.6, -1)),
            idle1=dict(bx=0, by=1, legs=(-2, 2), hand=(44, 42), dag=(0.6, -1)),
            atk0=dict(bx=-2, by=1, legs=(-4, 3), hand=(26, 30), dag=(-0.6, -1)),
            atk1=dict(bx=4, by=2, legs=(-5, 5), hand=(54, 40), dag=(1, 0.15)),
            hit=dict(bx=-3, by=1, legs=(-1, 1), hand=(40, 44), dag=(0.3, -1)),
            prep=dict(bx=-2, by=3, legs=(-5, 4), hand=(22, 26), dag=(-0.4, -1)),
        )[pose]
        L = Layer(cls.W, cls.H)
        bx, by = P["bx"], P["by"]
        o = lambda x, y: (x + bx, y + by)
        lb, lf = P["legs"]
        for hip, foot in (((27, 54), (27 + lb, 66)), ((34, 54), (34 + lf, 66))):
            limb(L, [o(*hip), foot], 6, CUERO)
            fx, fy = foot
            poly(L, [(fx - 3, fy - 3), (fx + 3, fy - 3), (fx + 6, fy + 2),
                     (fx + 6, fy + 4), (fx - 3, fy + 4)], CUERO)
        poly(L, [o(24, 29), o(14, 33), o(8, 40), o(12, 41), o(18, 36),
                 o(25, 33)], ROJA)
        poly(L, [o(23, 27), o(38, 27), o(42, 44), o(44, 58), o(30, 56),
                 o(16, 59), o(19, 42)], PIEL)
        poly(L, [o(19, 42), o(41, 42), o(41, 45), o(19, 45)], (NEGRO, NEGRO, OSCURO))
        flat_poly(L, [o(30, 42), o(32, 42), o(32, 45), o(30, 45)], ORO)
        poly(L, [o(25, 9), o(34, 7), o(41, 13), o(42, 25), o(37, 29), o(24, 29),
                 o(20, 18)], PIEL)
        flat_poly(L, [o(32, 14), o(40, 15), o(41, 25), o(33, 26), o(31, 20)], NEGRO)
        flat_line(L, [o(35, 19), o(36, 19)], ESPECTRAL)
        dot(L, *o(39, 19), ESPECTRAL)
        poly(L, [o(23, 27), o(40, 26), o(41, 31), o(24, 32)], ROJA)
        hx, hy = P["hand"]
        hy += by
        blade(L, hx, hy, P["dag"], 17, 3.5, guard_len=3, grip=None)
        limb(L, [o(35, 31), (hx, hy)], 5, PIEL)
        ellipse(L, hx, hy, 3, 3, CUERO)
        L.outline()
        return L


class Golem:
    name = "golem"
    W, H = 76, 78
    ANCHOR = (38, 77)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        bx = {"hit": -3, "atk1": 3, "prep": -1}.get(pose, 0)
        lift = {"walk0": (2, 0), "walk1": (0, 2)}.get(pose, (0, 0))
        fist = {"atk0": (54, 6), "atk1": (72, 54), "prep": (44, 2),
                "hit": (56, 56)}.get(pose, (60, 60 + by))
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        # piernas
        for x0, lf in ((24, lift[0]), (42, lift[1])):
            poly(L, [o(x0, 58), o(x0 + 11, 58), (x0 + 12 + bx // 2, 76 - lf),
                     (x0 - 1 + bx // 2, 76 - lf)], PIEDRA)
        # brazo trasero
        limb(L, [o(22, 30), o(16, 46), o(14, 56)], 11, PIEDRA)
        ellipse(L, *o(14, 60), 7, 6, PIEDRA)
        # torso
        poly(L, [o(14, 34), o(26, 20), o(50, 18), o(62, 30), o(60, 52),
                 o(48, 62), o(24, 62), o(12, 50)], PIEDRA)
        # grietas
        flat_line(L, [o(22, 30), o(28, 38), o(26, 46)], NEGRO)
        flat_line(L, [o(52, 44), o(46, 50), o(48, 56)], NEGRO)
        # runa del pecho (brilla más al prepararse)
        rune = BRILLO if pose == "prep" else ESPECTRAL
        flat_line(L, [o(38, 32), o(38, 48)], rune)
        flat_line(L, [o(34, 36), o(38, 40), o(42, 36)], rune)
        flat_line(L, [o(35, 46), o(41, 46)], rune)
        # cabeza
        poly(L, [o(36, 10), o(48, 6), o(56, 12), o(56, 22), o(42, 24),
                 o(35, 18)], PIEDRA)
        flat_line(L, [o(44, 14), o(48, 14)], ESPECTRAL, width=2)
        dot(L, *o(53, 14), ESPECTRAL)
        # brazo delantero
        sx, sy = o(52, 28)
        ex, ey = (sx + fist[0]) / 2 + 4, (sy + fist[1]) / 2 + 2
        limb(L, [(sx, sy), (ex, ey), fist], 11, PIEDRA)
        ellipse(L, fist[0], fist[1], 8, 7, PIEDRA)
        L.outline()
        return L


class Huargo:
    name = "huargo"
    W, H = 92, 64
    ANCHOR = (46, 63)
    FUR = (OSCURO, MEDIO, CLARO)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        bx = {"hit": -4, "atk0": -3, "atk1": 6, "prep": -2}.get(pose, 0)
        crouch = 4 if pose in ("prep", "atk0") else 0
        step = {"walk0": 1, "walk1": -1}.get(pose, 0)
        jaw = pose in ("atk1", "prep")
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by + crouch)
        F = cls.FUR
        # patas traseras (lado lejano)
        limb(L, [o(30, 40), (26 - step * 4 + bx, 52), (28 - step * 5 + bx, 61)], 5, CUERO)
        limb(L, [o(62, 42), (63 + step * 4 + bx, 52), (61 + step * 5 + bx, 61)], 5, CUERO)
        # cola
        limb(L, [o(22, 30), o(10, 24), o(4, 32)], 7, F)
        # cuerpo y pecho
        ellipse(L, *o(40, 34), 22, 12, F)
        ellipse(L, *o(62, 34), 12, 14, F)
        # crin dentada
        flat_poly(L, [o(30, 22), o(34, 16), o(38, 22), o(42, 15), o(46, 21),
                      o(50, 14), o(54, 20), o(58, 13), o(62, 21)], MEDIO)
        # patas cercanas
        limb(L, [o(34, 42), (32 + step * 4 + bx, 52), (34 + step * 5 + bx, 61)], 6, F)
        limb(L, [o(66, 44), (68 - step * 4 + bx, 53), (66 - step * 5 + bx, 61)], 6, F)
        # cabeza
        poly(L, [o(66, 20), o(76, 21), o(88, 28), o(87, 32), o(76, 34),
                 o(68, 34)], F)
        poly(L, [o(68, 20), o(70, 9), o(75, 20)], F)
        if jaw:
            poly(L, [o(72, 34), o(86, 34), o(84, 42), o(74, 38)], F)
            flat_line(L, [o(76, 34), o(86, 34)], BRILLO)
            flat_poly(L, [o(74, 34), o(86, 33), o(84, 37), o(75, 36)], SANGRE)
        else:
            flat_line(L, [o(76, 33), o(86, 31)], NEGRO)
        dot(L, *o(87, 28), NEGRO)
        flat_line(L, [o(75, 25), o(77, 25)], SANGRE, width=2)
        L.outline()
        return L


class Troll:
    name = "troll"
    W, H = 92, 88
    ANCHOR = (42, 87)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        bx = {"hit": -4, "atk1": 4}.get(pose, 0)
        step = {"walk0": 3, "walk1": -3}.get(pose, 0)
        club = {
            "atk0": ((38, 12), (-0.8, -0.7)),
            "atk1": ((68, 56), (1, 0.45)),
            "prep": ((46, 8), (0.1, -1)),
            "hit": ((58, 46), (-0.2, -1)),
        }.get(pose, ((58, 46 + by), (-0.25, -1)))
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        # piernas
        limb(L, [o(34, 60), (30 - step + bx, 72), (30 - step + bx, 84)], 10, PIEL)
        limb(L, [o(48, 60), (52 + step + bx, 72), (52 + step + bx, 84)], 10, PIEL)
        # brazo trasero
        limb(L, [o(26, 36), o(16, 54), o(18, 68)], 9, PIEL)
        ellipse(L, *o(18, 71), 5, 5, PIEL)
        # cuerpo jorobado
        ellipse(L, *o(42, 52), 19, 16, PIEL)
        ellipse(L, *o(36, 34), 19, 13, PIEL)
        poly(L, [o(24, 60), o(60, 60), o(56, 72), o(44, 68), o(30, 72)], CUERO)
        # cabeza baja y adelantada
        ellipse(L, *o(58, 32), 9, 9, PIEL)
        poly(L, [o(54, 36), o(68, 36), o(66, 44), o(55, 42)], PIEL)
        flat_poly(L, [o(60, 37), o(62, 31), o(63, 37)], BRILLO)
        flat_poly(L, [o(65, 37), o(66, 33), o(67, 37)], BRILLO)
        flat_line(L, [o(61, 27), o(64, 28)], NEGRO, width=2)
        dot(L, *o(63, 28), SANGRE)
        if pose == "prep":
            flat_poly(L, [o(57, 38), o(66, 38), o(64, 43), o(58, 42)], NEGRO)
        # garrote con pinchos
        (hx, hy), d = club
        hy += by
        dx, dy = unit(*d)
        px, py = -dy, dx
        Q = lambda t, s=0.0: (hx + dx * t + px * s, hy + dy * t + py * s)
        poly(L, [Q(-4, -2.5), Q(30, -6), Q(36, 0), Q(30, 6), Q(-4, 2.5)], CUERO)
        for t in (16, 24, 31):
            for s in (-1, 1):
                flat_line(L, [Q(t, s * 5), Q(t + 1, s * 9)], BRILLO)
        # brazo delantero
        limb(L, [o(48, 36), ((48 + bx + hx) / 2 + 4, (36 + by + hy) / 2 + 4),
                 (hx, hy)], 9, PIEL)
        ellipse(L, hx, hy, 5, 5, PIEL)
        L.outline()
        return L


# ================================================================= jefes
class ReyCiervo:
    name = "rey_ciervo"
    W, H = 100, 108
    ANCHOR = (46, 107)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        bx = {"hit": -4, "atk1": 4}.get(pose, 0)
        sway = {"walk0": 3, "walk1": -3}.get(pose, 0)
        staff = {"atk0": (58, 26), "atk1": (80, 56), "prep": (64, 20),
                 "hit": (66, 58)}.get(pose, (70, 58 + by))
        sdir = {"atk0": (-0.3, -1), "atk1": (1, 0.3)}.get(pose, (0.05, 1))
        glow = BRILLO if pose == "prep" else ESPECTRAL
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        # túnica de musgo con bajo raído
        poly(L, [o(34, 42), o(58, 42), o(64, 70), (70 + sway, 98),
                 (64 + sway, 104), (58 + sway, 98), (52, 106), (46, 99),
                 (38 - sway, 106), (32 - sway, 98), (24 - sway, 104), o(28, 70)],
             (OSCURO, MEDIO, CLARO))
        flat_line(L, [o(46, 46), o(44, 70), (46, 98)], NEGRO)
        # hombreras de corteza
        ellipse(L, *o(36, 44), 9, 6, CUERO)
        ellipse(L, *o(56, 44), 9, 6, CUERO)
        # cornamenta
        for pts in ([o(46, 22), o(40, 12), o(34, 2)], [o(40, 12), o(30, 10)],
                    [o(37, 6), o(42, 0)], [o(52, 20), o(58, 8), o(68, 2)],
                    [o(58, 8), o(66, 10)], [o(63, 5), o(64, -2)]):
            limb(L, pts, 3, HUESO, shadow_w=1)
        # cráneo de ciervo
        poly(L, [o(44, 20), o(56, 17), o(66, 23), o(74, 30), o(70, 34),
                 o(58, 34), o(46, 32), o(42, 26)], HUESO)
        flat_poly(L, [o(54, 23), o(59, 22), o(60, 27), o(55, 27)], NEGRO)
        dot(L, *o(57, 24), glow)
        dot(L, *o(58, 25), glow)
        flat_line(L, [o(66, 30), o(72, 31)], NEGRO)
        # corona de raíces
        flat_line(L, [o(44, 20), o(48, 14), o(52, 19), o(55, 13), o(57, 18)], OSCURO)
        # báculo
        hx, hy = staff
        dx, dy = unit(*sdir)
        if sdir[1] > 0:  # apoyado: de la mano al suelo y hacia arriba
            top = (hx - dx * 22, hy - dy * 22)
            bot = (hx + dx * 46, min(106, hy + dy * 46))
        else:
            top = (hx + dx * 30, hy + dy * 30)
            bot = (hx - dx * 26, hy - dy * 26)
        limb(L, [bot, (hx, hy), top], 4, CUERO, shadow_w=1)
        ellipse(L, top[0], top[1], 5, 5, (MEDIO, glow, BRILLO), contour=None)
        # brazo
        limb(L, [o(56, 46), ((56 + bx + hx) / 2 + 3, (46 + by + hy) / 2),
                 (hx, hy)], 6, CUERO)
        ellipse(L, hx, hy, 4, 4, HUESO)
        L.outline()
        return L


class DamaEspectral:
    name = "dama_espectral"
    W, H = 92, 104
    ANCHOR = (44, 103)
    GHOST = (MEDIO, CLARO, BRILLO)

    @classmethod
    def draw(cls, pose):
        # flota: sube y baja más que respirar
        by = {"walk0": -2, "walk1": 1, "idle0": -1, "idle1": 2, "atk0": -3,
              "atk1": 0, "hit": 1, "prep": -6}[pose]
        bx = {"hit": -5, "atk1": 6, "atk0": -2}.get(pose, 0)
        reach = {"atk0": ((56, 20), (60, 14)), "atk1": ((72, 40), (84, 38)),
                 "prep": ((54, 16), (50, 4)), "hit": ((52, 44), (50, 56))
                 }.get(pose, ((62, 44), (68, 52)))
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        G = cls.GHOST
        # vestido que acaba en jirones
        poly(L, [o(36, 30), o(52, 30), o(60, 58), o(68, 82), o(60, 78),
                 o(54, 92), o(48, 80), o(42, 94), o(36, 80), o(28, 88),
                 o(24, 74), o(28, 56)], G)
        flat_line(L, [o(44, 40), o(42, 70), o(44, 86)], MEDIO)
        flat_line(L, [o(50, 44), o(54, 70)], MEDIO)
        # cabello / velo al viento
        poly(L, [o(36, 14), o(28, 22), o(18, 34), o(22, 36), o(16, 46),
                 o(30, 38), o(36, 30)], G)
        # velo y rostro hueco
        poly(L, [o(37, 8), o(50, 6), o(57, 14), o(58, 32), o(48, 28),
                 o(36, 32), o(34, 18)], G)
        flat_poly(L, [o(46, 14), o(55, 15), o(55, 26), o(47, 26)], NEGRO)
        dot(L, *o(49, 18), ESPECTRAL)
        dot(L, *o(53, 18), ESPECTRAL)
        if pose in ("atk1", "prep"):
            flat_poly(L, [o(49, 22), o(53, 22), o(52, 26), o(50, 26)], ESPECTRAL)
        # brazo con cadena
        (ex, ey), (hx, hy) = reach
        limb(L, [o(50, 32), (ex + bx, ey + by), (hx + bx, hy + by)], 4, G,
             shadow_w=1)
        cx, cy = hx + bx, hy + by
        for i in range(6):
            dot(L, cx - 1 + (i % 2), cy + 3 + i * 3, MEDIO)
            dot(L, cx + (i % 2), cy + 4 + i * 3, CLARO)
        # halo espectral y contorno
        L.outline(ESPECTRAL)
        L.outline(NEGRO)
        return L


class CaballeroNegro:
    name = "caballero_negro"
    W, H = 104, 104
    ANCHOR = (44, 103)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        bx = {"hit": -4, "atk1": 5, "atk0": -2}.get(pose, 0)
        step = {"walk0": 3, "walk1": -3}.get(pose, 0)
        sword = {"atk0": ((30, 18), (-0.6, -1)), "atk1": ((66, 54), (1, 0.25)),
                 "prep": ((44, 12), (0.05, -1)), "hit": ((58, 56), (0.5, -1))
                 }.get(pose, ((60, 58 + by), (0.45, -1)))
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        A = (NEGRO, OSCURO, CLARO)  # negro con filo de luz
        # capa raída
        poly(L, [o(30, 34), o(44, 34), o(40, 60), (34, 98), (28, 92), (22, 100),
                 (18, 90), (10, 96), o(16, 60)], OSCURA)
        # piernas
        for hip, knee, ank in (((38, 66), (34 - step, 82), (32 - step, 96)),
                               ((50, 66), (56 + step, 82), (58 + step, 96))):
            limb(L, [o(*hip), knee, ank], 9, A)
            ax, ay = ank
            poly(L, [(ax - 5, ay - 2), (ax + 4, ay - 2), (ax + 10, ay + 3),
                     (ax + 10, ay + 6), (ax - 5, ay + 6)], A)
            ellipse(L, knee[0], knee[1], 5, 4, A)
        # torso y faldón
        poly(L, [o(30, 36), o(56, 36), o(60, 52), o(56, 66), o(32, 66),
                 o(28, 50)], A)
        poly(L, [o(30, 62), o(58, 62), o(62, 76), o(44, 72), o(28, 76)], OSCURA)
        flat_line(L, [o(43, 40), o(43, 60)], SANGRE)
        flat_line(L, [o(36, 46), o(50, 46)], SANGRE)
        # hombreras con pinchos
        ellipse(L, *o(30, 38), 9, 7, A)
        flat_poly(L, [o(24, 34), o(26, 26), o(30, 33)], CLARO)
        # yelmo con cuernos
        for pts in ([o(36, 16), o(28, 12), o(24, 2)], [o(50, 14), o(58, 8), o(62, 0)]):
            limb(L, pts, 4, HUESO, shadow_w=1)
        poly(L, [o(34, 14), o(48, 11), o(54, 16), o(55, 32), o(50, 36),
                 o(36, 36), o(32, 24)], A)
        flat_line(L, [o(42, 22), o(54, 22)], SANGRE, width=2)
        # mandoble a dos manos
        (hx, hy), d = sword
        hy += by
        blade(L, hx, hy, d, 52, 6, guard_len=8)
        limb(L, [o(52, 40), ((52 + bx + hx) / 2 + 3, (40 + by + hy) / 2 + 3),
                 (hx, hy)], 7, A)
        ellipse(L, hx, hy, 4, 4, A)
        L.outline()
        return L


class Tejedora:
    name = "tejedora"
    W, H = 124, 92
    ANCHOR = (62, 91)
    QUITINA = (NEGRO, OSCURO, CLARO)

    @classmethod
    def draw(cls, pose):
        by = breathe(pose)
        rear = {"prep": -10, "atk0": -6, "atk1": 2, "hit": 0}.get(pose, 0)
        bx = {"hit": -5, "atk1": 6}.get(pose, 0)
        step = {"walk0": 1, "walk1": -1}.get(pose, 0)
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        Q = cls.QUITINA
        thx, thy = 74 + bx, 54 + by + rear // 2

        def leg(root, knee_dx, foot_dx, far):
            rx, ry = root
            kx, ky = rx + knee_dx, ry - 22
            fx, fy = rx + foot_dx, 90
            limb(L, [(rx, ry), (kx, ky), (fx, fy)], 3 if far else 4,
                 CUERO if far else Q, shadow_w=1)

        # patas lejanas
        for i, (kd, fd) in enumerate(((-18, -34), (-6, -14), (8, 14), (20, 36))):
            s = step * (1 if i % 2 else -1) * 4
            leg((thx - 4 + i * 3, thy - 2), kd + s, fd + s, True)
        # abdomen con marca de calavera
        ellipse(L, *o(38, 42 + rear // 2), 28, 22, Q)
        cx, cy = o(36, 40 + rear // 2)
        flat_poly(L, [(cx - 6, cy - 6), (cx + 6, cy - 6), (cx + 7, cy + 2),
                      (cx + 3, cy + 6), (cx - 3, cy + 6), (cx - 7, cy + 2)], SANGRE)
        flat_line(L, [(cx - 3, cy - 1), (cx - 2, cy - 1)], NEGRO, width=2)
        flat_line(L, [(cx + 2, cy - 1), (cx + 3, cy - 1)], NEGRO, width=2)
        # tórax
        ellipse(L, thx, thy, 15, 11, Q)
        # máscara pálida
        mx, my = thx + 16, thy - 8 + rear // 2
        ellipse(L, mx, my, 10, 12, HUESO)
        flat_poly(L, [(mx - 5, my - 5), (mx - 1, my - 4), (mx - 2, my), (mx - 6, my - 1)], NEGRO)
        flat_poly(L, [(mx + 2, my - 4), (mx + 6, my - 5), (mx + 6, my - 1), (mx + 2, my)], NEGRO)
        dot(L, mx - 3, my - 3, SANGRE)
        dot(L, mx + 4, my - 3, SANGRE)
        if pose in ("atk1", "prep"):
            flat_poly(L, [(mx - 3, my + 4), (mx + 3, my + 4), (mx + 2, my + 9), (mx - 2, my + 9)], NEGRO)
        else:
            flat_line(L, [(mx - 3, my + 6), (mx + 3, my + 6)], NEGRO)
        # patas cercanas
        for i, (kd, fd) in enumerate(((-22, -40), (-8, -18), (10, 18), (24, 42))):
            s = step * (-1 if i % 2 else 1) * 4
            if i == 3 and pose in ("atk0", "atk1", "prep"):
                # pata delantera alzada para golpear
                tip = {"atk0": (thx + 30, thy - 40), "atk1": (thx + 46, thy + 6),
                       "prep": (thx + 26, thy - 46)}[pose]
                limb(L, [(thx + 6, thy), (thx + 24, thy - 24), tip], 4, Q, shadow_w=1)
                continue
            leg((thx - 6 + i * 4, thy + 2), kd + s, fd + s, False)
        L.outline()
        return L


class ReyLiche:
    name = "rey_liche"
    W, H = 100, 108
    ANCHOR = (44, 107)

    @classmethod
    def draw(cls, pose):
        by = {"walk0": -1, "walk1": 1, "idle0": 0, "idle1": 2, "atk0": -2,
              "atk1": 1, "hit": 1, "prep": -3}[pose]
        bx = {"hit": -4, "atk1": 4}.get(pose, 0)
        staff = {"atk0": (60, 22), "atk1": (76, 44), "prep": (62, 16)
                 }.get(pose, (66, 50))
        flame = BRILLO if pose == "prep" else ESPECTRAL
        L = Layer(cls.W, cls.H)
        o = lambda x, y: (x + bx, y + by)
        # túnica con ribete dorado y bajo en jirones
        poly(L, [o(30, 38), o(56, 38), o(62, 66), o(68, 98), o(60, 94),
                 o(54, 104), o(46, 96), o(38, 104), o(32, 94), o(22, 100),
                 o(24, 66)], ROJA)
        flat_line(L, [o(30, 40), o(24, 66), o(22, 98)], ORO)
        flat_line(L, [o(56, 40), o(62, 66)], ORO)
        # costillas a la vista
        flat_poly(L, [o(37, 42), o(50, 42), o(48, 60), o(39, 60)], NEGRO)
        for i in range(4):
            flat_line(L, [o(38, 45 + i * 4), o(49, 45 + i * 4)], CLARO)
        flat_line(L, [o(43, 42), o(43, 60)], BRILLO)
        # cuello de capa
        poly(L, [o(26, 30), o(36, 38), o(30, 42), o(22, 36)], ROJA)
        poly(L, [o(60, 30), o(52, 38), o(58, 42), o(64, 36)], ROJA)
        # cráneo
        ellipse(L, *o(44, 26), 10, 10, HUESO)
        poly(L, [o(40, 30), o(52, 30), o(50, 38), o(42, 38)], HUESO)
        flat_poly(L, [o(43, 23), o(47, 23), o(47, 27), o(43, 27)], NEGRO)
        flat_poly(L, [o(49, 23), o(53, 23), o(53, 27), o(49, 27)], NEGRO)
        dot(L, *o(45, 25), flame)
        dot(L, *o(51, 25), flame)
        for i in range(4):
            dot(L, *o(43 + i * 2, 35), NEGRO)
        # corona
        flat_poly(L, [o(34, 18), o(36, 8), o(39, 15), o(43, 5), o(46, 14),
                      o(50, 6), o(52, 15), o(55, 9), o(55, 19), o(34, 19)], ORO)
        dot(L, *o(43, 15), SANGRE)
        # báculo con llama espectral
        hx, hy = staff
        hy += by
        top = (hx + 2, hy - 30)
        limb(L, [(hx - 2, min(104, hy + 50)), (hx, hy), top], 3, CUERO, shadow_w=1)
        ellipse(L, top[0], top[1] - 2, 6, 7, (MEDIO, flame, BRILLO), contour=None)
        flat_poly(L, [(top[0] - 3, top[1] - 6), (top[0], top[1] - 16),
                      (top[0] + 3, top[1] - 6)], flame)
        # brazo huesudo
        limb(L, [o(56, 40), ((56 + bx + hx) / 2 + 2, (40 + by + hy) / 2 + 4),
                 (hx, hy)], 4, HUESO, shadow_w=1)
        # mano libre con fuego al prepararse
        if pose == "prep":
            limb(L, [o(30, 40), o(22, 30), o(20, 18)], 4, HUESO, shadow_w=1)
            ellipse(L, *o(20, 12), 5, 6, (MEDIO, ESPECTRAL, BRILLO), contour=None)
        L.outline()
        return L


# Lote 2: solo existen como tiras de IA (assets/ai/, ver import_ai.SOURCES).
LOTE2 = [type(n, (), {"name": n}) for n in [
    "jabali",
    "arquero",
    "hombre_seta",
    "espiritu_roble",
    "trasgo",
    "oso",
    "druida",
    "serpiente",
    "esqueleto_guerrero",
    "zombi",
    "fantasma",
    "sepulturero",
    "banshee",
    "gargola",
    "momia",
    "cuervo",
    "guardia_real",
    "armadura_animada",
    "verdugo",
    "mago_torre",
    "dragon_joven",
    "demonio",
]]

NORMALES = [Salteador, Golem, Huargo, Troll] + LOTE2
JEFES = [ReyCiervo, DamaEspectral, CaballeroNegro, Tejedora, ReyLiche]
ENEMIES = NORMALES + JEFES  # mismo orden que EnemyType en C++


def build(cls):
    return [flip(cls.draw(p)) for p in POSES]
