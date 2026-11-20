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
    # Lote 2 (docs/PROMPTS_PERSONAJES.md): se ignoran mientras falte la tira.
    "jabali": ("11_jabali_magenta.png", 56),
    "arquero": ("12_arquero_magenta.png", 70),
    "hombre_seta": ("13_hombre_seta_magenta.png", 62),
    "trasgo": ("16_trasgo_magenta.png", 54),
    "oso": ("17_oso_magenta.png", 84),
    "serpiente": ("19_serpiente_magenta.png", 60),
    "cazador": ("20_cazador_magenta.png", 74),
    "esqueleto_guerrero": ("21_esqueleto_magenta.png", 70),
    "necrofago": ("23_necrofago_magenta.png", 62),
    "fantasma": ("24_fantasma_magenta.png", 74),
    "murcielago": ("25_murcielago_magenta.png", 60),
    "banshee": ("27_banshee_magenta.png", 78),
    "gargola": ("28_gargola_magenta.png", 76),
    "sabueso": ("29_sabueso_magenta.png", 58),
    "cuervo": ("31_cuervo_magenta.png", 56),
    "guardia_real": ("32_guardia_real_magenta.png", 76),
    "armadura_animada": ("33_armadura_animada_magenta.png", 80),
    "alquimista": ("34_alquimista_magenta.png", 70),
    "bufon": ("35_bufon_magenta.png", 64),
    "verdugo": ("36_verdugo_magenta.png", 86),
    "mago_torre": ("37_mago_torre_magenta.png", 80),
    "dragon_joven": ("38_dragon_joven_magenta.png", 72),
    "mimico": ("39_mimico_magenta.png", 56),
    "espia": ("40_espia_magenta.png", 70),
    "ballestero": ("41_ballestero_magenta.png", 72),
    "mastin": ("42_mastin_magenta.png", 58),
    "bruja_pantano": ("43_bruja_pantano_magenta.png", 98),
    "gran_jabali": ("44_gran_jabali_magenta.png", 90),
    "senor_trasgos": ("45_senor_trasgos_magenta.png", 96),
    "nigromante": ("46_nigromante_magenta.png", 104),
    "viuda_negra": ("47_viuda_negra_magenta.png", 100),
    "guardian_cripta": ("48_guardian_cripta_magenta.png", 106),
    "reina_hechicera": ("49_reina_hechicera_magenta.png", 102),
    "dragon_ancestral": ("50_dragon_ancestral_magenta.png", 100),
    "rey_loco": ("51_rey_loco_magenta.png", 104),
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


def figure_labels(mask, w, h, n):
    """Asigna cada píxel opaco a uno de los n fotogramas por piezas
    conectadas, no por columnas: un arma que invade la columna del vecino
    sigue siendo de su figura. Devuelve (etiquetas, [bbox por fotograma])."""
    from collections import deque
    lab = [[-1] * w for _ in range(h)]
    comps = []
    for y in range(h):
        for x in range(w):
            if mask[y][x] and lab[y][x] == -1:
                cid = len(comps)
                q = deque([(x, y)])
                lab[y][x] = cid
                n_px, x0, y0, x1, y1 = 0, x, y, x, y
                while q:
                    cx, cy = q.popleft()
                    n_px += 1
                    x0, x1 = min(x0, cx), max(x1, cx)
                    y0, y1 = min(y0, cy), max(y1, cy)
                    for dx in (-1, 0, 1):
                        for dy in (-1, 0, 1):
                            nx, ny = cx + dx, cy + dy
                            if 0 <= nx < w and 0 <= ny < h and mask[ny][nx] \
                                    and lab[ny][nx] == -1:
                                lab[ny][nx] = cid
                                q.append((nx, ny))
                comps.append((n_px, x0, y0, x1 + 1, y1 + 1))
    # las n piezas más grandes son las figuras, ordenadas de izquierda a derecha
    big = sorted(sorted(range(len(comps)), key=lambda c: -comps[c][0])[:n],
                 key=lambda c: comps[c][1])
    frame_of = {c: i for i, c in enumerate(big)}
    for c, (_, x0, y0, x1, y1) in enumerate(comps):
        if c in frame_of:
            continue
        # resto: al recuadro de figura más cercano (motas muy pequeñas fuera)
        if comps[c][0] < 30:
            frame_of[c] = -1
            continue
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2

        def dist(i):
            _, a0, b0, a1, b1 = comps[big[i]]
            return max(a0 - cx, 0, cx - a1) + max(b0 - cy, 0, cy - b1)
        frame_of[c] = min(range(n), key=dist)
    boxes = [[w, h, 0, 0] for _ in range(n)]
    for y in range(h):
        row = lab[y]
        for x in range(w):
            if row[x] >= 0:
                f = frame_of[row[x]]
                row[x] = f
                if f >= 0:
                    b = boxes[f]
                    b[0], b[1] = min(b[0], x), min(b[1], y)
                    b[2], b[3] = max(b[2], x + 1), max(b[3], y + 1)
    return lab, [tuple(b) for b in boxes]


def _mask(im):
    w, h = im.size
    p = im.load()
    return [[not is_bg(*p[x, y]) for x in range(w)] for y in range(h)], w, h


def _bbox(mask, x0, x1, h):
    ys = [y for y in range(h) if any(mask[y][x0:x1])]
    xs = [x for x in range(x0, x1) if any(mask[y][x] for y in ys)]
    return min(xs), min(ys), max(xs) + 1, max(ys) + 1


def fit_map(im, mp, n=7):
    """Encaja el mapa de piezas sobre la tira fotograma a fotograma: el
    generador no siempre lo calca, pero sí repite la pose; se escala el
    recuadro de cada figura del mapa al de la tira."""
    ma, w, h = _mask(im)
    mb, mw, mh = _mask(mp)
    _, boxa = figure_labels(ma, w, h, n)
    _, boxb = figure_labels(mb, mw, mh, n)
    out = Image.new("RGB", (w, h), (255, 0, 255))
    for A, B in zip(boxa, boxb):
        piece = mp.crop(B).resize((A[2] - A[0], A[3] - A[1]), Image.NEAREST)
        out.paste(piece, (A[0], A[1]))
    return out


class ImportedHero:
    def __init__(self, file=HERO_FILE, mapfile=HERO_MAP):
        im = Image.open(os.path.join(SRC, file)).convert("RGB")
        mp = fit_map(im, Image.open(os.path.join(SRC, mapfile)).convert("RGB"))
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
        flab, boxes = figure_labels(mask, w, h, 7)
        ground = max(b[3] for b in boxes) - 1
        s = HERO_H / (ground - boxes[0][1] + 1)

        cels = []
        for f, (x0, top, x1, _) in enumerate(boxes):
            sub = [[v if flab[y][x] == f else KEY for x, v in
                    zip(range(x0, x1), comb[y][x0:x1])]
                   for y in range(top, ground + 1)]
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


def load_hero(name="heroe"):
    """Héroe desde assets/ai/<name>.png + <name>_mapa.png, o None."""
    files = (f"{name}.png", f"{name}_mapa.png")
    if not all(os.path.exists(os.path.join(SRC, f)) for f in files):
        return None
    return ImportedHero(*files)


# ------------------------------------------------------------------ título
TITLE_LOGO, TITLE_BG = "titulo_logo.png", "titulo_fondo.png"
BAYER4 = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def load_logo(W=208, H=72):
    """Logo sobre magenta → Layer W×H (encajado y centrado, fondo KEY)."""
    path = os.path.join(SRC, TITLE_LOGO)
    if not os.path.exists(path):
        return None
    im = Image.open(path).convert("RGB")
    w, h = im.size
    p = im.load()
    idx = [[KEY if is_bg(*p[x, y]) else classify(*p[x, y]) for x in range(w)]
           for y in range(h)]
    ys = [y for y in range(h) if any(c != KEY for c in idx[y])]
    xs = [x for x in range(w) if any(idx[y][x] != KEY for y in ys)]
    x0, x1, y0, y1 = min(xs), max(xs) + 1, min(ys), max(ys) + 1
    sub = [row[x0:x1] for row in idx[y0:y1]]
    s = min((W - 2) / (x1 - x0), (H - 2) / (y1 - y0))
    px, fw, fh = downscale(sub, x1 - x0, y1 - y0, s)
    L = Layer(W, H)
    ox, oy = (W - fw) // 2, (H - fh) // 2
    for y in range(fh):
        for x in range(fw):
            if px[y][x] != KEY:
                L.px[oy + y][ox + x] = px[y][x]
    clean(L.px, W, H)
    return L


def load_title_bg(W=240, H=135):
    """Ilustración del título → Layer W×H opaca. Los neutros se traman con
    Bayer 4×4 entre los dos tonos vecinos; los acentos solo si son claros."""
    path = os.path.join(SRC, TITLE_BG)
    if not os.path.exists(path):
        return None
    im = Image.open(path).convert("RGB").resize((W, H), Image.BOX)
    sharp = Image.open(path).convert("RGB").resize((W, H), Image.NEAREST)
    p, q = im.load(), sharp.load()
    lums = [0.3 * r + 0.59 * g + 0.11 * b for r, g, b in (PAL[i] for i in NEUTRAL)]
    L = Layer(W, H)
    for y in range(H):
        for x in range(W):
            # acentos: del muestreo nítido (no se diluyen las ascuas)
            c = classify(*q[x, y])
            if c in (SANGRE, ORO, ESPECTRAL):
                L.px[y][x] = c
                continue
            r, g, b = p[x, y]
            lum = (0.3 * r + 0.59 * g + 0.11 * b) * 0.9  # algo más oscuro
            k = 0
            while k < len(lums) - 2 and lum > lums[k + 1]:
                k += 1
            lo, hi = lums[k], lums[k + 1]
            t = min(1.0, max(0.0, (lum - lo) / (hi - lo)))
            # trama solo en la franja de transición; fuera, tono plano
            t = min(1.0, max(0.0, (t - 0.3) / 0.4))
            L.px[y][x] = NEUTRAL[k + 1] if t * 16 > BAYER4[y % 4][x % 4] + 0.5 else NEUTRAL[k]
    return L


# ------------------------------------------------------------------- fondos
# Una imagen vertical por bioma con 5 franjas apiladas: cielo, lejos, medio,
# suelo y frente. Las franjas empiezan donde arranca una fila toda magenta
# (o en una línea oscura de separación).
BG_BANDS = ["cielo", "lejos", "medio", "suelo", "frente"]
# Colocación en pantalla (240×135): fila donde cae el borde inferior del
# contenido ("bottom") o el superior ("top").
BG_PLACE = {
    "lejos": ("bottom", 104),   # el pie lo tapa el suelo
    "medio": ("bottom", 113),   # raíces y pies de columna sobre el suelo
    "suelo": ("top", 106),
    "frente": ("top", 101),     # casi todo queda bajo el HUD (y ≥ 112)
}


def classify_bg(r, g, b):
    """Como classify, pero la hierba verde no cuenta como brillo mágico:
    solo los verdes muy luminosos (fuegos fatuos, luciérnagas)."""
    c = classify(r, g, b)
    if c == ESPECTRAL and g < 185:
        lum = 0.3 * r + 0.59 * g + 0.11 * b
        c = min(NEUTRAL, key=lambda i: abs(lum - (0.3 * PAL[i][0] + 0.59 * PAL[i][1] + 0.11 * PAL[i][2])))
    return c


def _band_rows(p, w, h):
    """Inicio de cada franja: filas donde empieza una racha toda magenta."""
    full = [sum(is_bg(*p[x, y]) for x in range(0, w, 4)) / (w / 4) > 0.97
            for y in range(h)]
    starts = [0]
    for y in range(1, h):
        if full[y] and not full[y - 1] and y - starts[-1] > h // 20:
            starts.append(y)
    assert len(starts) == 5, f"{len(starts)} franjas en vez de 5"
    return starts + [h]


def _quant_dither(px_avg, x, y):
    """Neutro tramado (Bayer 4×4) entre los dos tonos vecinos."""
    r, g, b = px_avg
    lums = [0.3 * pr + 0.59 * pg + 0.11 * pb for pr, pg, pb in (PAL[i] for i in NEUTRAL)]
    lum = 0.3 * r + 0.59 * g + 0.11 * b
    k = 0
    while k < len(lums) - 2 and lum > lums[k + 1]:
        k += 1
    t = min(1.0, max(0.0, (lum - lums[k]) / (lums[k + 1] - lums[k])))
    t = min(1.0, max(0.0, (t - 0.3) / 0.4))
    return NEUTRAL[k + 1] if t * 16 > BAYER4[y % 4][x % 4] + 0.5 else NEUTRAL[k]


def load_biome_bg(biome, W=240, H=135):
    """{capa: Layer} para cielo, lejos, medio, suelo y frente, o None."""
    path = os.path.join(SRC, f"bg_{biome}.png")
    if not os.path.exists(path):
        return None
    im = Image.open(path).convert("RGB")
    w, h = im.size
    p = im.load()
    rows = _band_rows(p, w, h)
    s = W / w
    out = {}
    for i, name in enumerate(BG_BANDS):
        y0, y1 = rows[i], rows[i + 1]
        band = im.crop((0, y0, w, y1))
        L = Layer(W, H)
        if name == "cielo":
            avg = band.resize((W, H), Image.BOX).load()
            sharp = band.resize((W, H), Image.NEAREST).load()
            for y in range(H):
                for x in range(W):
                    c = classify_bg(*sharp[x, y])
                    L.px[y][x] = c if c in (SANGRE, ORO, ESPECTRAL) else _quant_dither(avg[x, y], x, y)
            out[name] = L
            continue
        bp = band.load()
        bw, bh = band.size
        idx = [[KEY if is_bg(*bp[x, y]) else classify_bg(*bp[x, y]) for x in range(bw)]
               for y in range(bh)]
        px, fw, fh = downscale(idx, bw, bh, s)
        ys = [y for y in range(fh) if any(c != KEY for c in px[y])]
        top, bot = min(ys), max(ys)
        mode, row = BG_PLACE[name]
        oy = row - bot if mode == "bottom" else row - top
        for y in range(top, bot + 1):
            sy = oy + y
            if 0 <= sy < H:
                for x in range(min(fw, W)):
                    if px[y][x] != KEY:
                        L.px[sy][x] = px[y][x]
        if name == "suelo":  # opaco hasta abajo
            for x in range(W):
                last = NEGRO
                for y in range(H):
                    if L.px[y][x] != KEY:
                        last = L.px[y][x]
                    elif y > row + 4:
                        L.px[y][x] = last if last in (NEGRO, OSCURO) else OSCURO
        out[name] = L
    return out
