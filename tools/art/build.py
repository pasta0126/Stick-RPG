"""Genera todo el arte del juego.

  python3 tools/art/build.py            → regenera assets/sprites/*.aseprite
                                          (SOBRESCRIBE los retoques a mano),
                                          y exporta src/gfx/*.h
  python3 tools/art/build.py --only-c   → solo cabeceras de fuente/paletas/tablas

Después, `make sprites` exporta los .aseprite a C (también tras editarlos a mano).
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)

import bg  # noqa: E402
import enemies  # noqa: E402
import font  # noqa: E402
import hero  # noqa: E402
import title  # noqa: E402
from palettes import BIOMAS, KEY, full16  # noqa: E402

ASEPRITE = os.environ.get("ASEPRITE",
                          "/Applications/Aseprite.app/Contents/MacOS/aseprite")
BUILD = os.path.join(ROOT, "build", "art")
ASSETS = os.path.join(ROOT, "assets", "sprites")
GFX = os.path.join(ROOT, "src", "gfx")
BIOME_ORDER = ["bosque", "cementerio", "castillo"]


# ------------------------------------------------------------- .aseprite
def write_cels(path, w, h, layers, tags):
    """layers: [(nombre, [Layer por frame])]."""
    frames = len(layers[0][1])
    out = [f"SIZE {w} {h} {frames} {KEY}"]
    out += [f"PAL {r} {g} {b}" for r, g, b in full16("bosque")]
    for name, cels in layers:
        out.append(f"LAYER {name}")
        for fi, L in enumerate(cels):
            out.append(f"CEL {fi}")
            out += ["".join("%x" % c for c in row) for row in L.px]
    out += [f"TAG {n} {f} {c} {ms}" for n, f, c, ms in tags]
    with open(path, "w") as fh:
        fh.write("\n".join(out) + "\n")


def assemble(name, w, h, layers, tags):
    os.makedirs(BUILD, exist_ok=True)
    cels = os.path.join(BUILD, name + ".txt")
    write_cels(cels, w, h, layers, tags)
    out = os.path.join(ASSETS, name + ".aseprite")
    subprocess.run([ASEPRITE, "-b", "--script-param", f"src={cels}",
                    "--script-param", f"out={out}", "--script",
                    os.path.join(ROOT, "tools/sprites/assemble.lua")],
                   check=True, stdout=subprocess.DEVNULL)
    print("  ", os.path.relpath(out, ROOT))


def build_sprites():
    os.makedirs(ASSETS, exist_ok=True)
    print("Héroe…")
    hl = hero.build()
    assemble("heroe", hero.W, hero.H, [(n, hl[n]) for n in hero.LAYERS],
             hero.TAGS)
    print("Enemigos…")
    for cls in enemies.ENEMIES:
        assemble(cls.name, cls.W, cls.H, [("cuerpo", enemies.build(cls))],
                 enemies.TAGS)
    print("Título…")
    assemble("titulo", title.W, title.H, [("logo", [title.build()])], [])
    print("Fondos…")
    for b in BIOME_ORDER:
        layers = bg.BUILDERS[b]()
        assemble("bg_" + b, bg.W, bg.H, [(n, [layers[n]]) for n in bg.ORDER], [])


# ------------------------------------------------------------- cabeceras
def write(path, text):
    with open(path, "w") as fh:
        fh.write(text)
    print("  ", os.path.relpath(path, ROOT))


def build_font():
    chars = [chr(c) for c in range(32, 96)]
    rows = []
    for ch in chars:
        g = font.GLYPHS.get(ch, font.GLYPHS["?"])
        w = len(g[0])
        bits = []
        for r in g:
            v = 0
            for i, c in enumerate(r):
                if c == "#":
                    v |= 0x80 >> i
            bits.append("0x%02X" % v)
        rows.append(f"    {{{w}, {{{', '.join(bits)}}}}},  // '{ch}'")
    write(os.path.join(GFX, "font5x7_gfx.h"), f"""#pragma once
// Generado por tools/art/build.py desde tools/art/font.py. No editar.
// Fuente pixel 5x7, ASCII 32..95 (las minúsculas se dibujan en mayúsculas).

#include <cstdint>

namespace gfx {{

struct Glyph5 {{
  uint8_t w;
  uint8_t rows[7];  // bit 7 = columna izquierda
}};

static const Glyph5 kFont5[64] = {{
{chr(10).join(rows)}
}};

}}  // namespace gfx
""")


def build_tables():
    pals = []
    for b in BIOME_ORDER:
        cols = ", ".join("{0x%02X, 0x%02X, 0x%02X}" % c for c in BIOMAS[b])
        pals.append(f"    {{{cols}}},  // {b}")
    includes = "\n".join(f'#include "{n}_gfx.h"' for n in
                         ["heroe", "titulo"] + [c.name for c in enemies.ENEMIES] +
                         ["bg_" + b for b in BIOME_ORDER])
    art = []
    for c in enemies.ENEMIES:
        ax = c.W - 1 - c.ANCHOR[0]  # tras voltear
        n = c.name
        art.append(f"    {{&{n}::sheet, {ax}, {c.ANCHOR[1]}, {n}::WALK, {n}::IDLE, "
                   f"{n}::ATTACK, {n}::HIT, {n}::PREPARE}},  // {n}")
    par = ", ".join(str(bg.PARALLAX[n]) for n in bg.ORDER)
    write(os.path.join(GFX, "assets.h"), f"""#pragma once
// Generado por tools/art/build.py. No editar.

#include "sprite.h"
{includes}

namespace gfx {{

constexpr int NUM_BIOMAS = {len(BIOME_ORDER)};
// Paletas de 8 colores por bioma (índices: ver src/palette.h).
static const uint8_t kBiomePalette[NUM_BIOMAS][8][3] = {{
{chr(10).join(pals)}
}};

static const Sheet* const kBiomeBg[NUM_BIOMAS] = {{
    {", ".join("&bg_" + b + "::sheet" for b in BIOME_ORDER)}}};
// Desplazamiento por encuentro de cada capa del fondo (parallax).
static const int16_t kParallax[{len(bg.ORDER)}] = {{{par}}};
constexpr int BG_LAYER_PERSONAJES = {bg.ORDER.index("frente")};  // se dibujan antes de esta capa

constexpr int HERO_AX = {hero.ANCHOR[0]}, HERO_AY = {hero.ANCHOR[1]};

struct EnemyArt {{
  const Sheet* sheet;
  uint8_t ax, ay;  // pies
  Anim walk, idle, attack, hit, prepare;
}};

// Mismo orden que EnemyType (src/core/game_types.h).
static const EnemyArt kEnemyArt[] = {{
{chr(10).join(art)}
}};

}}  // namespace gfx
""")


if __name__ == "__main__":
    if "--only-c" not in sys.argv:
        build_sprites()
    print("Cabeceras…")
    os.makedirs(GFX, exist_ok=True)
    build_font()
    build_tables()
