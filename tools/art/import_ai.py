"""Importa las tiras de sprites generadas con IA (stick_rpg_sprites/).

Cada tira tiene 8 fotogramas en fila, mirando a la izquierda, sobre magenta.
Para cada una:
  1. separa los fotogramas (columnas con contenido),
  2. clasifica cada píxel en uno de los 8 índices de la paleta (o fondo),
  3. reduce a la altura del sprite del juego (voto por mayoría en cada celda),
  4. alinea los fotogramas por los pies y pone el contorno negro.

El resultado imita la interfaz de las clases de enemies.py (name, W, H,
ANCHOR) con los fotogramas ya mirando a la izquierda en `frames`.
"""

import os

from PIL import Image

from palettes import (BIOMAS, BRILLO, CLARO, ESPECTRAL, KEY, MEDIO, NEGRO,
                      ORO, OSCURO, SANGRE)
from raster import Layer

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, "..", "..", "stick_rpg_sprites"))

# nombre del juego → (archivo, altura del fotograma de reposo en px)
SOURCES = {
    "salteador": ("02_salteador_magenta.png", 70),
    "golem": ("03_golem_magenta.png", 76),
    "huargo": ("04_huargo_magenta.png", 58),
    "troll": ("05_troll_magenta.png", 86),
    "rey_ciervo": ("06_rey_ciervo_magenta.png", 106),
    "dama_espectral": ("07_dama_espectral_magenta.png", 100),
    "caballero_negro": ("08_caballero_negro_magenta.png", 100),
    "tejedora": ("09_la_tejedora_magenta.png", 84),
    "rey_liche": ("10_rey_liche_magenta.png", 106),
}
N_FRAMES = 8
IDLE = 2
PAL = BIOMAS["bosque"]
NEUTRAL = [NEGRO, OSCURO, MEDIO, CLARO, BRILLO]


# ------------------------------------------------------------ clasificación
def is_bg(r, g, b):
    """Magenta y su mezcla con el personaje en los bordes."""
    return r - g > 45 and b - g > 45 and abs(r - b) < 110


def classify(r, g, b):
    """Índice de la paleta. Los acentos solo para colores saturados, para que
    los grises verdosos no se conviertan en verde mágico."""
    mx, mn = max(r, g, b), min(r, g, b)
    sat = mx - mn
    if g - max(r, b) > 45 and g > 110:
        return ESPECTRAL
    if r - max(g, b) > 55 and r > 90 and r - b > 55 and g < r * 0.75:
        if g > b + 35 and g > 90 and r > g + 15:
            return ORO
        return SANGRE
    if r > b + 60 and g > b + 25 and r > g + 15 and r > 120 and sat > 70:
        return ORO
    # neutros: por luminancia
    lum = 0.3 * r + 0.59 * g + 0.11 * b
    best, bd = NEGRO, 1e9
    for i in NEUTRAL:
        pr, pg, pb = PAL[i]
        d = abs(lum - (0.3 * pr + 0.59 * pg + 0.11 * pb))
        if d < bd:
            best, bd = i, d
    return best


# --------------------------------------------------------------- fotogramas
def segments(mask, w, h, n):
    cols = [any(mask[y][x] for y in range(h)) for x in range(w)]
    segs, x = [], 0
    while x < w:
        if cols[x]:
            x1 = x
            while x1 < w and cols[x1]:
                x1 += 1
            segs.append([x, x1])
            x = x1
        else:
            x += 1
    # quita motas sueltas y une huecos pequeños hasta tener n
    segs = [s for s in segs if s[1] - s[0] > 2] or segs
    while len(segs) > n:
        gaps = [segs[k + 1][0] - segs[k][1] for k in range(len(segs) - 1)]
        k = gaps.index(min(gaps))
        segs[k] = [segs[k][0], segs[k + 1][1]]
        del segs[k + 1]
    assert len(segs) == n, f"{len(segs)} fotogramas en vez de {n}"
    return segs


def downscale(idx, w, h, s):
    """Reduce un mapa de índices por voto en cada celda (fondo si ≥ 50 %)."""
    nw, nh = max(1, round(w * s)), max(1, round(h * s))
    out = [[KEY] * nw for _ in range(nh)]
    for ty in range(nh):
        y0, y1 = int(ty / s), max(int(ty / s) + 1, int((ty + 1) / s))
        for tx in range(nw):
            x0, x1 = int(tx / s), max(int(tx / s) + 1, int((tx + 1) / s))
            votes = {}
            total = 0
            for y in range(y0, min(y1, h)):
                for x in range(x0, min(x1, w)):
                    c = idx[y][x]
                    votes[c] = votes.get(c, 0) + 1
                    total += 1
            bg = votes.pop(KEY, 0)
            if not votes or bg * 2 > total:
                continue
            # el negro gana empates: mantiene contornos y separaciones
            out[ty][tx] = max(votes, key=lambda c: (votes[c] + (0.6 if c == NEGRO else 0)))
    return out, nw, nh


def body_x(idx, w, h, ref_h):
    """Mediana horizontal del torso (franja del 35 al 70 % de la altura de
    reposo, medida desde el suelo); las armas y los brazos apenas la mueven."""
    xs = []
    for y in range(max(0, h - int(ref_h * 0.70)), max(1, h - int(ref_h * 0.35))):
        xs += [x for x in range(w) if idx[y][x] != KEY]
    xs.sort()
    return xs[len(xs) // 2] if xs else w // 2


def clean(px, w, h):
    """Quita píxeles opacos aislados y cierra el contorno negro exterior."""
    def opaque(x, y):
        return 0 <= x < w and 0 <= y < h and px[y][x] != KEY
    N4 = ((1, 0), (-1, 0), (0, 1), (0, -1))
    for y in range(h):
        for x in range(w):
            if px[y][x] != KEY and not any(opaque(x + dx, y + dy) for dx, dy in N4):
                px[y][x] = KEY
    add = []
    for y in range(h):
        for x in range(w):
            if px[y][x] == KEY:
                continue
            for dx, dy in N4:
                if not opaque(x + dx, y + dy) and px[y][x] not in (NEGRO, OSCURO):
                    add.append((x, y))
                    break
    for x, y in add:
        px[y][x] = NEGRO


class Imported:
    def __init__(self, name, fname, target_h):
        self.name = name
        im = Image.open(os.path.join(SRC, fname)).convert("RGB")
        w, h = im.size
        p = im.load()
        idx = [[KEY if is_bg(*p[x, y]) else classify(*p[x, y]) for x in range(w)]
               for y in range(h)]
        mask = [[c != KEY for c in row] for row in idx]
        segs = segments(mask, w, h, N_FRAMES)
        ground = max(y for y in range(h) if any(mask[y]))
        # escala: altura del fotograma de reposo
        x0, x1 = segs[IDLE]
        top = min(y for y in range(h) if any(mask[y][x0:x1]))
        s = target_h / (ground - top + 1)

        cels = []
        for x0, x1 in segs:
            top = min(y for y in range(h) if any(mask[y][x0:x1]))
            sub = [row[x0:x1] for row in idx[top:ground + 1]]
            px, fw, fh = downscale(sub, x1 - x0, ground + 1 - top, s)
            cels.append((px, fw, fh, body_x(px, fw, fh, target_h)))

        # lienzo común: pies alineados y en la fila inferior
        left = max(fx for _, _, _, fx in cels)
        right = max(fw - fx for _, fw, _, fx in cels)
        W = left + right + 2
        W += W % 2
        H = max(fh for _, _, fh, _ in cels) + 1
        ax = left + 1
        self.W, self.H, self.ANCHOR = W, H, (ax, H - 1)
        self.frames = []
        for px, fw, fh, fx in cels:
            L = Layer(W, H)
            ox, oy = ax - fx, H - fh
            for y in range(fh):
                for x in range(fw):
                    if px[y][x] != KEY:
                        L.px[oy + y][ox + x] = px[y][x]
            clean(L.px, W, H)
            self.frames.append(L)


def load(name):
    """Sprite importado para `name`, o None si no hay tira."""
    if name not in SOURCES:
        return None
    fname, th = SOURCES[name]
    if not os.path.exists(os.path.join(SRC, fname)):
        return None
    return Imported(name, fname, th)
