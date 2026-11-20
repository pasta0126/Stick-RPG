"""Genera todo el arte del juego.

  python3 tools/art/build.py            → regenera assets/sprites/*.aseprite
                                          (SOBRESCRIBE los retoques a mano),
                                          y exporta src/gfx/*.h
  python3 tools/art/build.py --only-c   → solo cabeceras de fuente/paletas/tablas
  python3 tools/art/build.py --enemies  → solo enemigos (importa assets/ai/)
  python3 tools/art/build.py --bg       → solo fondos (assets/ai/bg_<bioma>.png, 5 franjas)
  python3 tools/art/build.py --title    → solo título (assets/ai/titulo_logo.png + titulo_fondo.png)
  python3 tools/art/build.py --hero     → solo héroe (assets/ai/heroe.png + heroe_mapa.png)

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
import import_ai  # noqa: E402
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
    build_hero()
    build_enemies()
    build_title()
    build_backgrounds()


def build_backgrounds():
    """Fondos: capas importadas de assets/ai/bg_<bioma>.png si existen (la
    niebla sigue siendo procedural); si no, las de bg.py."""
    print("Fondos…")
    for b in BIOME_ORDER:
        layers = bg.BUILDERS[b]()
        imp = import_ai.load_biome_bg(b, bg.W, bg.H)
        if imp:
            layers.update(imp)
        assemble("bg_" + b, bg.W, bg.H, [(n, [layers[n]]) for n in bg.ORDER], [])


# Héroes jugables, en el orden de HeroClass (src/core/game_types.h).
HEROES = ["heroe", "vikinga"]
# Forma de no muerto (talismán): cualquier héroe que muere con él la toma.
UNDEAD = "esqueleto"
_hero = {}


def HERO(name="heroe"):
    """(W, H, ANCHOR, {capa: frames}) importado de assets/ai/<name>.png si
    existe; si no, el caballero (importado o de hero.py)."""
    if name not in _hero:
        imp = import_ai.load_hero(name)
        if imp:
            _hero[name] = (imp.W, imp.H, imp.ANCHOR, imp.layers)
        elif name != "heroe":
            _hero[name] = HERO("heroe")
        else:
            _hero[name] = (hero.W, hero.H, hero.ANCHOR, hero.build())
    return _hero[name]


def build_hero():
    print("Héroes…")
    for name in HEROES + [UNDEAD]:
        w, h, _, hl = HERO(name)
        assemble(name, w, h, [(n, hl[n]) for n in hero.LAYERS], hero.TAGS)


def build_title():
    """Logo (importado de assets/ai si existe) y fondo del título."""
    print("Título…")
    logo = import_ai.load_logo(title.W, title.H) or title.build()
    assemble("titulo", title.W, title.H, [("logo", [logo])], [])
    fondo = import_ai.load_title_bg(bg.W, bg.H)
    if fondo:
        assemble("titulo_fondo", bg.W, bg.H, [("fondo", [fondo])], [])


def enemy_art():
    """Enemigos importados de las tiras de IA si existen; si no, los de
    enemies.py. Mismo orden que EnemyType."""
    out = []
    for cls in enemies.ENEMIES:
        imp = import_ai.load(cls.name)
        out.append((imp, imp.frames) if imp else (cls, enemies.build(cls)))
    return out


def build_enemies():
    print("Enemigos…")
    for cls, frames in ENEMY_ART():
        assemble(cls.name, cls.W, cls.H, [("cuerpo", frames)], enemies.TAGS)


_cache = []


def ENEMY_ART():
    if not _cache:
        _cache.extend(enemy_art())
    return _cache


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
                         HEROES + [UNDEAD, "titulo", "titulo_fondo"] + [c.name for c in enemies.ENEMIES] +
                         ["bg_" + b for b in BIOME_ORDER])
    art = []
    for c, frames in ENEMY_ART():
        if isinstance(c, import_ai.Imported):
            ax = c.ANCHOR[0]  # ya mira a la izquierda
        else:
            ax = c.W - 1 - c.ANCHOR[0]  # tras voltear
        # píxeles visibles a la izquierda de los pies al caminar (frames 0-1)
        left = min(x for L in frames[:2] for row in L.px
                   for x, v in enumerate(row) if v != KEY)
        n = c.name
        art.append(f"    {{&{n}::sheet, {ax}, {c.ANCHOR[1]}, {ax - left}, {n}::WALK, {n}::IDLE, "
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

struct HeroArt {{
  const Sheet* sheet;  // capas: las de heroe::Layer
  uint8_t ax, ay;      // pies
}};

// Mismo orden que HeroClass (src/core/game_types.h).
static const HeroArt kHeroArt[] = {{
{chr(10).join(f"    {{&{n}::sheet, {HERO(n)[2][0]}, {HERO(n)[2][1]}}},  // {n}" for n in HEROES)}
}};

// Héroe maldito (talismán). Si aún no hay arte propio es una copia del
// caballero y el juego lo tiñe.
static const HeroArt kUndeadArt = {{&{UNDEAD}::sheet, {HERO(UNDEAD)[2][0]}, {HERO(UNDEAD)[2][1]}}};
constexpr bool kUndeadOwnArt = {"true" if import_ai.load_hero(UNDEAD) else "false"};

struct EnemyArt {{
  const Sheet* sheet;
  uint8_t ax, ay;  // pies
  uint8_t reach;   // píxeles visibles a la izquierda de los pies al caminar
  Anim walk, idle, attack, hit, prepare;
}};

// Mismo orden que EnemyType (src/core/game_types.h).
static const EnemyArt kEnemyArt[] = {{
{chr(10).join(art)}
}};

}}  // namespace gfx
""")


if __name__ == "__main__":
    if {"--enemies", "--hero", "--title", "--bg"} & set(sys.argv):
        if "--bg" in sys.argv:
            build_backgrounds()
        if "--title" in sys.argv:
            build_title()
        if "--hero" in sys.argv:
            build_hero()
        if "--enemies" in sys.argv:
            build_enemies()
    elif "--only-c" not in sys.argv:
        build_sprites()
    print("Cabeceras…")
    os.makedirs(GFX, exist_ok=True)
    build_font()
    build_tables()
