"""Rasterizador mínimo para generar pixel art indexado.

Cada pieza se dibuja como una máscara (polígonos, elipses, líneas) y se
sombrea con una rampa de 3 tonos: luz arriba-izquierda, sombra abajo-derecha.
Después se añade contorno interior entre piezas y contorno exterior negro.
"""

from PIL import Image, ImageDraw

from palettes import KEY, NEGRO, OSCURO


class Layer:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [[KEY] * w for _ in range(h)]
        self.pid = [[0] * w for _ in range(h)]
        self._next = 1

    # ------------------------------------------------------------ máscaras
    def mask(self, draw_fn):
        m = Image.new("1", (self.w, self.h), 0)
        draw_fn(ImageDraw.Draw(m))
        return m.load()

    def inside(self, m, x, y):
        return 0 <= x < self.w and 0 <= y < self.h and m[x, y]

    # ------------------------------------------------------------- piezas
    def part(self, draw_fn, ramp, contour=OSCURO, shadow_w=2, light=True):
        """ramp = (sombra, base, luz). Dibuja encima de lo anterior."""
        m = self.mask(draw_fn)
        sh, base, li = ramp
        pid = self._next
        self._next += 1
        pts = [(x, y) for y in range(self.h) for x in range(self.w) if m[x, y]]
        for x, y in pts:
            c = base
            if any(not self.inside(m, x + d, y) or not self.inside(m, x, y + d)
                   for d in range(1, shadow_w + 1)):
                c = sh
            elif light and (not self.inside(m, x - 1, y) or
                            not self.inside(m, x, y - 1)):
                c = li
            self.px[y][x] = c
        if contour is not None:
            for x, y in pts:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if (0 <= nx < self.w and 0 <= ny < self.h and
                            not m[nx, ny] and self.pid[ny][nx] not in (0, pid)):
                        self.px[y][x] = contour
                        break
        for x, y in pts:
            self.pid[y][x] = pid
        return pid

    def flat(self, draw_fn, color):
        """Pieza de un solo color, sin sombreado ni contorno interior."""
        m = self.mask(draw_fn)
        pid = self._next
        self._next += 1
        for y in range(self.h):
            for x in range(self.w):
                if m[x, y]:
                    self.px[y][x] = color
                    self.pid[y][x] = pid

    def put(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = c
            if self.pid[y][x] == 0:
                self.pid[y][x] = 999

    def outline(self, color=NEGRO, diagonal=False):
        src = [row[:] for row in self.px]
        dirs = [(1, 0), (-1, 0), (0, 1), (0, -1)]
        if diagonal:
            dirs += [(1, 1), (-1, -1), (1, -1), (-1, 1)]
        for y in range(self.h):
            for x in range(self.w):
                if src[y][x] != KEY:
                    continue
                for dx, dy in dirs:
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < self.w and 0 <= ny < self.h and src[ny][nx] != KEY:
                        self.px[y][x] = color
                        break

    def blit(self, other, ox=0, oy=0):
        for y in range(other.h):
            for x in range(other.w):
                c = other.px[y][x]
                if c != KEY:
                    self.put(x + ox, y + oy, c)

    def to_image(self, palette, bg=None):
        img = Image.new("RGB", (self.w, self.h), bg or (255, 0, 255))
        p = img.load()
        for y in range(self.h):
            for x in range(self.w):
                c = self.px[y][x]
                if c != KEY:
                    p[x, y] = palette[c]
        return img


# Patrón Bayer 4x4 para degradados con dos colores.
BAYER4 = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def dither(x, y, t):
    """True si en (x, y) toca el segundo color para una mezcla t ∈ [0, 1]."""
    return BAYER4[y % 4][x % 4] < t * 16
