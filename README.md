# Stick-RPG

Turn-based dark-fantasy pixel-art RPG for the **M5StickC Plus2** (240 × 135 landscape display, 8 colours per biome).

Pick a hero and hold your ground while enemies walk in, one at a time. You decide how to attack (a normal hit, or a timed special once the runes are charged) and which loot to keep after each victory. Every chapter is a new biome with 5 random enemies and a boss; beat the 10th boss and the journey starts over, harder. Survive as long as you can.

- Design: [GDD_RPG_M5StickC_Plus2.md](GDD_RPG_M5StickC_Plus2.md)
- Art style guide (palettes, sprite specs, how to deliver art): [docs/STYLE_GUIDE.md](docs/STYLE_GUIDE.md)
- Status: complete and playable on device: 6 heroes, 10 biomes with their bosses, music and effects.

In-game text is in Spanish.

## Install

The easiest way is **[M5Burner](https://burner.m5stack.com)**: search for *Stick RPG* (device StickC-Plus2) and burn it.

Or flash the merged image yourself (it goes at offset `0x0` and erases the saved record and sound setting):

```sh
make release   # builds release/StickRPG-M5StickCPlus2.bin
esptool --chip esp32 write-flash 0x0 release/StickRPG-M5StickCPlus2.bin
```

## Heroes

Choose one in the carousel (B spins right, PWR spins left, A picks).

| Hero | Weapon | Style |
|------|--------|-------|
| Caballero | Sword and shield | More defence, balanced |
| Vikinga | Axe and shield | More attack, less defence |
| Samurái | Katana | Lots of critical hits, wide perfect zone for the special |
| Hechicera | Staff and grimoire | Lots of attack, special every 2 hits |
| Pícara | Daggers | More luck, more potions |
| Druida | Crook and shield | Balanced, heals a little with each attack (capped per fight) |

## Chapters

| # | Biome | Boss |
|---|-------|------|
| I | El Bosque | Rey Ciervo |
| II | El Pantano | Bruja del Pantano |
| III | La Mina | Rey Trasgo |
| IV | Las Cumbres | Yeti |
| V | El Desierto | Guardián |
| VI | El Naufragio | Dama Espectral |
| VII | El Cementerio | La Tejedora |
| VIII | El Volcán | Dragón Ancestral |
| IX | El Castillo | Caballero Negro |
| X | La Cripta | El Primigenio (final boss) |

Normal enemies are random in every biome. The HUD shows the level as chapter-encounter (e.g. `II-4`).

## Controls

| Button | Action |
|--------|--------|
| A      | Attack (tap) · hold to charge the special when its runes are lit, release in the perfect zone · confirm |
| B      | Sound on/off · switch option on loot screens · next hero in the carousel |
| PWR    | Pause (press) · previous hero in the carousel · power off (hold 2 s) |

## Build

```sh
pio run -t upload        # build and flash the firmware (PlatformIO + M5Unified)
make release             # merged .bin for M5Burner / esptool (offset 0x0)
make test                # game-logic tests on the PC
make sim ARGS="--skill 0.7 --games 10000"   # balance simulator (--class <hero> to pick one)
make sprites             # export assets/sprites/*.aseprite to C (needs Aseprite)
make art                 # regenerate ALL art from tools/art (overwrites .aseprite hand edits)
```

## Layout

| Path | Contents |
|------|----------|
| `src/core/` | Game rules in plain C++ (combat, heroes, loot, enemies, chapters, balance) |
| `src/` | Firmware: states, rendering, UI, audio, clouds |
| `src/gfx/` | 4 bpp renderer and exported sprite headers |
| `assets/sprites/` | Aseprite sources |
| `assets/ai/` | AI-generated sprite strips, imported by `tools/art/import_ai.py` |
| `tools/art/` | Art generators and importers, palettes, pixel font, mock-ups |
| `tools/sprites/` | Aseprite assemble/export scripts |
| `tools/sim/`, `tools/test/` | Balance simulator and tests |
| `docs/` | Style guide, palette files, mock-ups |
