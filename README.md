# Stick-RPG

Turn-based dark-fantasy RPG for the **M5StickC Plus2** (240 × 135 landscape display, 8 colours per biome).

A lone knight holds his ground while enemies walk in, one at a time. You decide how to attack (normal or a timed special every 3 hits) and which loot to keep after each victory. Survive as long as you can: a boss every 10 encounters, three biomes (forest, graveyard, castle) and an endless difficulty curve.

- Design: [GDD_RPG_M5StickC_Plus2.md](GDD_RPG_M5StickC_Plus2.md)
- Art style guide (palettes, sprite specs, how to deliver art): [docs/STYLE_GUIDE.md](docs/STYLE_GUIDE.md)
- Status: playable on device, full art, music and effects.

In-game text is in Spanish.

## Controls

| Button | Action |
|--------|--------|
| A      | Attack (tap) · hold to charge the special when its 3 runes are lit · confirm |
| B      | Stats screen · switch option on reward screens · sound toggle on title |
| PWR    | Pause (press) · power off (hold 2 s) |

## Build

```sh
pio run -t upload        # build and flash the firmware (PlatformIO + M5Unified)
make test                # game-logic tests on the PC
make sim ARGS="--skill 0.7 --games 10000"   # balance simulator
make sprites             # export assets/sprites/*.aseprite to C (needs Aseprite)
make art                 # regenerate ALL art from tools/art (overwrites .aseprite hand edits)
```

## Layout

| Path | Contents |
|------|----------|
| `src/core/` | Game rules in plain C++ (combat, loot, enemies, balance) |
| `src/` | Firmware: states, rendering, UI, audio, clouds |
| `src/gfx/` | 4 bpp renderer and exported sprite headers |
| `assets/sprites/` | Aseprite sources |
| `tools/art/` | Art generators, palettes, pixel font, mock-ups |
| `tools/sprites/` | Aseprite assemble/export scripts |
| `tools/sim/`, `tools/test/` | Balance simulator and tests |
| `docs/` | Style guide, palette files, mock-ups |
