"""Importa las tiras de sprites generadas con IA (assets/ai/).

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
SRC = os.path.abspath(os.path.join(HERE, "..", "..", "assets", "ai"))

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
    # dos fotogramas que se tocan: corta el más ancho por su columna más vacía
    while len(segs) < n:
        k = max(range(len(segs)), key=lambda i: segs[i][1] - segs[i][0])
        a, b = segs[k]
        m = (b - a) // 5
        cut = min(range(a + m, b - m), key=lambda x: sum(mask[y][x] for y in range(h)))
        segs[k:k + 1] = [[a, cut], [cut, b]]
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


# -------------------------------------------------------------------- héroe
# Tira de 7 fotogramas mirando a la derecha + mapa de piezas con la misma
# silueta, un color plano por capa.
HERO_FILE, HERO_MAP, HERO_H = "heroe.png", "heroe_mapa.png", 66
HERO_LAYERS = ["cuerpo", "botas", "pechera", "casco", "escudo", "brazo"]
MAP_COLORS = {
    (128, 128, 128): "cuerpo", (0, 255, 255): "botas", (255, 255, 0): "pechera",
    (0, 0, 255): "casco", (255, 128, 0): "escudo", (0, 255, 0): "brazo",
}


def map_label(r, g, b):
    if is_bg(r, g, b):
        return None
    best, bd = None, 120
    for (cr, cg, cb), n in MAP_COLORS.items():
        d = abs(r - cr) + abs(g - cg) + abs(b - cb)
        if d < bd:
            best, bd = HERO_LAYERS.index(n), d
    return best


def fill_labels(lab, mask, w, h):
    """Píxeles del personaje sin etiqueta (contornos que el mapa no tiene):
    toman la etiqueta más cercana (BFS)."""
    from collections import deque
    q = deque((x, y) for y in range(h) for x in range(w) if lab[y][x] is not None)
    while q:
        x, y = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < h and mask[ny][nx] and lab[ny][nx] is None:
                lab[ny][nx] = lab[y][x]
                q.append((nx, ny))


def behind_shield(layers, W, H):
    """Rellena en `cuerpo` la parte del torso que tapa el escudo, para que no
    quede un agujero cuando no hay escudo: prolonga hacia la derecha el color
    del cuerpo un tercio del ancho del escudo en cada fila."""
    cu, es = layers["cuerpo"], layers["escudo"]
    others = [layers[n] for n in HERO_LAYERS if n != "escudo"]
    for y in range(H):
        xs = [x for x in range(W) if es.px[y][x] != KEY]
        if not xs:
            continue
        x0, x1 = min(xs), max(xs)
        # color del cuerpo justo a la izquierda (saltando el contorno)
        src = None
        for x in range(x0 - 1, max(-1, x0 - 6), -1):
            v = next((L.px[y][x] for L in others if L.px[y][x] != KEY), KEY)
            if v == KEY:
                break
            if v != NEGRO:
                src = v
                break
        if src is None:
            continue
        end = x0 + max(1, (x1 - x0 + 1) // 3)
        for x in range(x0, end):
            if es.px[y][x] != KEY and all(L.px[y][x] == KEY for L in others):
                cu.px[y][x] = src
        if end < W and es.px[y][end] != KEY and all(L.px[y][end] == KEY for L in others):
            cu.px[y][end] = NEGRO


class ImportedHero:
    def __init__(self):
        im = Image.open(os.path.join(SRC, HERO_FILE)).convert("RGB")
        mp = Image.open(os.path.join(SRC, HERO_MAP)).convert("RGB")
        if mp.size != im.size:
            mp = mp.resize(im.size, Image.NEAREST)
        w, h = im.size
        p, q = im.load(), mp.load()
        idx = [[KEY if is_bg(*p[x, y]) else classify(*p[x, y]) for x in range(w)]
               for y in range(h)]
        mask = [[c != KEY for c in row] for row in idx]
        lab = [[map_label(*q[x, y]) if mask[y][x] else None for x in range(w)]
               for y in range(h)]
        fill_labels(lab, mask, w, h)
        n = len(HERO_LAYERS)
        # índice combinado: capa * 16 + color, para reducir ambos a la vez
        comb = [[KEY if idx[y][x] == KEY else (lab[y][x] or 0) * 16 + idx[y][x]
                 for x in range(w)] for y in range(h)]
        segs = segments(mask, w, h, 7)
        ground = max(y for y in range(h) if any(mask[y]))
        x0, x1 = segs[0]
        top = min(y for y in range(h) if any(mask[y][x0:x1]))
        s = HERO_H / (ground - top + 1)

        for x0, x1 in segs:
            drop_specks(comb, mask, x0, x1, h)
        cels = []
        for x0, x1 in segs:
            top = min(y for y in range(h) if any(mask[y][x0:x1]))
            sub = [row[x0:x1] for row in comb[top:ground + 1]]
            px, fw, fh = downscale_comb(sub, x1 - x0, ground + 1 - top, s)
            # pies: mediana de las botas en la franja inferior
            xs = sorted(x for y in range(fh - max(3, fh // 10), fh)
                        for x in range(fw) if px[y][x] != KEY
                        and px[y][x] // 16 == HERO_LAYERS.index("botas"))
            fx = xs[len(xs) // 2] if xs else fw // 2
            cels.append((px, fw, fh, fx))

        left = max(fx for _, _, _, fx in cels)
        right = max(fw - fx for _, fw, _, fx in cels)
        W = left + right + 2
        W += W % 2
        H = max(fh for _, _, fh, _ in cels) + 1
        ax = left + 1
        self.W, self.H, self.ANCHOR = W, H, (ax, H - 1)
        self.layers = {name: [] for name in HERO_LAYERS}
        for px, fw, fh, fx in cels:
            comp = [[KEY] * W for _ in range(H)]
            lay = [[0] * W for _ in range(H)]
            ox, oy = ax - fx, H - fh
            for y in range(fh):
                for x in range(fw):
                    v = px[y][x]
                    if v != KEY:
                        comp[oy + y][ox + x] = v % 16
                        lay[oy + y][ox + x] = v // 16
            before = [row[:] for row in comp]
            clean(comp, W, H)
            # los píxeles nuevos del contorno toman la capa de un vecino
            for y in range(H):
                for x in range(W):
                    if comp[y][x] != KEY and before[y][x] == KEY:
                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                            nx, ny = x + dx, y + dy
                            if 0 <= nx < W and 0 <= ny < H and before[ny][nx] != KEY:
                                lay[y][x] = lay[ny][nx]
                                break
            frame = {}
            for li, name in enumerate(HERO_LAYERS):
                L = Layer(W, H)
                for y in range(H):
                    for x in range(W):
                        if comp[y][x] != KEY and lay[y][x] == li:
                            L.px[y][x] = comp[y][x]
                frame[name] = L
            behind_shield(frame, W, H)
            for name in HERO_LAYERS:
                self.layers[name].append(frame[name])


def drop_specks(comb, mask, x0, x1, h):
    """Borra los trozos sueltos (p. ej. la punta de la espada del fotograma
    vecino) dejando solo la pieza conectada más grande."""
    from collections import deque
    seen = set()
    comps = []
    for y in range(h):
        for x in range(x0, x1):
            if mask[y][x] and (x, y) not in seen:
                comp, q = [], deque([(x, y)])
                seen.add((x, y))
                while q:
                    cx, cy = q.popleft()
                    comp.append((cx, cy))
                    for dx in (-1, 0, 1):
                        for dy in (-1, 0, 1):
                            nx, ny = cx + dx, cy + dy
                            if x0 <= nx < x1 and 0 <= ny < h and mask[ny][nx] \
                                    and (nx, ny) not in seen:
                                seen.add((nx, ny))
                                q.append((nx, ny))
                comps.append(comp)
    comps.sort(key=len)
    for comp in comps[:-1]:
        for x, y in comp:
            mask[y][x] = False
            comb[y][x] = KEY


def downscale_comb(idx, w, h, s):
    """Como downscale, pero con valores capa*16+color: vota la capa y luego el
    color dentro de esa capa."""
    nw, nh = max(1, round(w * s)), max(1, round(h * s))
    out = [[KEY] * nw for _ in range(nh)]
    for ty in range(nh):
        y0, y1 = int(ty / s), max(int(ty / s) + 1, int((ty + 1) / s))
        for tx in range(nw):
            x0, x1 = int(tx / s), max(int(tx / s) + 1, int((tx + 1) / s))
            votes, total, bg = {}, 0, 0
            for y in range(y0, min(y1, h)):
                for x in range(x0, min(x1, w)):
                    v = idx[y][x]
                    total += 1
                    if v == KEY:
                        bg += 1
                    else:
                        votes[v] = votes.get(v, 0) + 1
            if not votes or bg * 2 > total:
                continue
            layer_votes = {}
            for v, c in votes.items():
                layer_votes[v // 16] = layer_votes.get(v // 16, 0) + c
            li = max(layer_votes, key=layer_votes.get)
            cand = {v: c for v, c in votes.items() if v // 16 == li}
            out[ty][tx] = max(cand, key=lambda v: cand[v] + (0.6 if v % 16 == NEGRO else 0))
    return out, nw, nh


def load_hero():
    if not all(os.path.exists(os.path.join(SRC, f)) for f in (HERO_FILE, HERO_MAP)):
        return None
    return ImportedHero()
