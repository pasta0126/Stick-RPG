# STICK RPG

**Game Design Document (GDD) — turn-based combat RPG with automatic encounters**

M5StickC Plus2 · Landscape 240 × 135 px · 8 colours per biome · Dark medieval fantasy

> **CORE IDEA**
>
> A lone knight holds his ground while enemies come to him, one at a time. The player decides how to attack and, after every victory, whether to take the reward or keep the current gear. The goal is to survive as many enemies as possible.

| **DOCUMENT**                       | **STATUS**                                                      |
|------------------------------------|-----------------------------------------------------------------|
| Version 1.4 · 19 November 2026     | Playable on device; full art, biomes, bosses, music and effects |
| Scope                              | Agreed decisions and parameters still to be balanced            |

Working title: “Stick RPG” (same as the repository). The final title is not decided yet.

**Language:** documentation is in English. In-game text is in Spanish (e.g. *Salteador*, *Botín*, *Has caído*); English glosses are given in this document where useful.

# 1. Overview

Landscape game with **dark medieval fantasy** pixel art and 8 colours per biome (sinister forest, graveyard, castle). The knight stays at his post on the left; enemies walk in from the right in front of a multi-layer parallax background. Interaction is focused on button A during combat and on reward decisions with A and B.

## 1.1. Main loop

1. **Title**: press A to start.
2. **Biome intro**: the current biome’s scenery appears with the knight on guard on the left, and a chapter card with the biome name.
3. **Encounter**: an enemy walks in from the right. Each encounter shifts the background layers so the scenery changes; changing biome triggers a palette fade and a new chapter card.
4. **Combat**: turn-based. The hero always acts first.
5. **Victory**: the enemy dissolves, the hero recovers a percentage of max HP, and exactly one reward drops; the player decides what to do with it.
6. **Continue**: the next enemy walks in.
7. **Defeat**: “Has caído” (*You have fallen*) screen with run stats, record and restart.

> **RUN STRUCTURE**
>
> An endless loop of increasingly harder encounters. There are no platforms, jumps or manual dodging. **Hero HP carries over between fights**: it is only restored by the victory heal and by potions.

# 2. Device and constraints

| **Aspect**     | **Rule**                                                                   |
|----------------|----------------------------------------------------------------------------|
| Target         | M5StickC Plus2 (ESP32-PICO-V3-02, ST7789V2 display, buzzer, battery)       |
| Orientation    | Landscape (240 × 135 pixels)                                               |
| Colour         | 8 simultaneous colours; the palette changes with the biome (§8.2)          |
| Look           | Pixel art, highly legible sprites and typography                           |
| Movement       | No player movement; enemies walk in automatically                          |
| Input          | Button A, button B and power button (PWR); no external controls            |
| Sound          | Built-in buzzer: effects and short jingles                                 |
| Persistence    | Record and settings stored in non-volatile memory (NVS)                    |
| Structure      | One enemy per encounter, turn-based combat, one reward per victory         |

# 3. States and controls

| **State**          | **Button A**                                                              | **Button B**                       |
|--------------------|---------------------------------------------------------------------------|------------------------------------|
| Title              | Start                                                                     | Sound on / off                     |
| Hero select        | Confirm the highlighted hero                                              | Switch hero (Caballero / Vikinga)  |
| Biome intro        | Skip (after a short delay)                                                | —                                  |
| Enemy walking in   | —                                                                         | —                                  |
| Hero turn          | Tap: normal attack. Hold: charge the special (when ready, §4.3).          | Sound on / off                     |
| Enemy turn         | —                                                                         | —                                  |
| Gear reward        | Confirm the highlighted panel                                             | Switch panel (keep / equip)        |
| Potion reward      | Confirm the highlighted panel                                             | Switch panel (restore / max HP)    |
| Defeat             | Back to title                                                             | —                                  |

An extra turn is a normal hero turn and is controlled the same way. While A is held, B does nothing.

**Hero select:** after the title the player picks the hero. *Caballero*: base stats. *Vikinga* (axe and round shield): ATK 9, DEF 1, crit damage +80 %, miss 12 % (`VIKINGA_*` in `balance.h`); the simulator puts both at ~35 enemies on average. The record is shared.

**B toggles sound at any moment** (title, combat, intros, pause, defeat) and shows a short “SONIDO SI / NO” notice; the only exception is the reward screens, where B switches panel.

**PWR button** (any state): short press = pause / resume. Hold 2 s = power off. While paused, the screen dims after a few seconds of inactivity.

# 4. Combat rules

## 4.1. Encounter sequence

- The hero always opens the fight.
- The player attacks with A (normal or special). The hit may land or miss.
- On a hit, damage is computed; a critical may trigger and increase it.
- After the hero’s action, the extra-turn check happens.
- On an extra turn the enemy’s turn is skipped and the hero attacks again; otherwise the enemy acts.
- The enemy turn resolves automatically with the same rules: **enemies can also miss and crit**. If both are alive, control returns to the hero.
- The fight ends as soon as either HP reaches zero.

## 4.2. Damage, defence, misses and criticals

Proportional formula (stable in an endless run, because the result depends on the ratio between attack and defence, not their difference):

```
base_damage = ATK² / (ATK + DEF)
damage      = round(base_damage × charge_multiplier × crit_multiplier)
damage      = max(damage, 1)          // only on hits
```

- With ATK = DEF a hit deals half the ATK; very high DEF reduces damage gradually without ever nullifying it.
- Misses deal 0 damage.
- Crit multiplier = 1 + CRIT DMG (e.g. +50 % → ×1.5).
- Resolution order: hit? → base damage → × charge → × crit → round → minimum 1.

## 4.3. Normal attack and special (charged) attack

**The charged attack is special, not constant:** it can only be used **from the 3rd hit onward and every 3 hits** (`ESPECIAL_CADA` = 3).

- Every normal attack lights one rune (3 runes in the HUD); misses also count.
- Runes are **not reset between fights**.
- With 3 runes lit, holding A charges the special; releasing it fires it and spends the runes.
- Without enough runes, A attacks **on press**, without waiting for the threshold.
- With the special ready, a short tap is still a normal attack and does not spend the runes.
- **Runes are only spent when the special is actually fired**, so it can be saved for the right moment. Safeguards: 350 ms hold threshold; releasing A within the first 150 ms of charging counts as a normal attack.

Press handling uses a **single time threshold** (`T_UMBRAL_MS`, 350 ms):

| **Action**                          | **Behaviour**                                                                   |
|-------------------------------------|---------------------------------------------------------------------------------|
| Press and release before threshold  | Normal attack (×1). Fires on release.                                           |
| Hold past the threshold             | The charge bar appears across the top of the screen and starts filling.         |
| Full bar while still holding        | The bar **wraps back to zero and fills again** (continuous cycle).              |
| Release A                           | Special attack; its multiplier depends on the bar fill at release time.         |
| Cost                                | Any attack, normal or special, takes a single hero action.                      |

Special multiplier from the fill `f` (0–100 %):

```
charge_multiplier = CARGA_MIN + (CARGA_MAX − CARGA_MIN) × f
if f ≥ ZONA_PERFECTA: charge_multiplier = CARGA_PERFECTA
```

Current values: `CARGA_MIN` = ×0.6, `CARGA_MAX` = ×2.4, `ZONA_PERFECTA` = 92 %, `CARGA_PERFECTA` = ×3.2.

Charging is a timing gamble: released at the right moment the hit is far stronger than a normal one; released late (after the bar wrapped) it can be weaker than a normal attack. The perfect zone is marked in gold on the bar and has its own flash and sound.

## 4.4. Extra turn

The extra-turn chance (EXTRA %) comes from the hero’s stats and gear. When it triggers, the enemy loses its turn and the hero attacks again. It is automatic.

- Extra turns can chain, but **each consecutive extra turn halves the chance of the next one** within the same sequence.
- The base chance is capped (§5).
- A missed attack can also trigger an extra turn (simple rule: the check always happens). Can be revisited during balancing.

## 4.5. Telegraphed heavy blow

On its turn an enemy may **prepare** instead of attacking. That turn deals no damage, but a blinking red “!” appears above it and the text “SE PREPARA!” (*It is preparing!*). Its next action is a **heavy blow** (`GOLPE_FUERTE`, ×2.5), which can also miss or crit.

- The warning survives the hero’s extra turns: the heavy blow lands on the enemy’s next real turn.
- It creates a decision without extra buttons: gamble on a perfect special to kill it first, or take the hit.
- Each enemy type prepares with a different frequency (§7.1).

# 5. Stats and equipment

The knight has base stats plus bonuses from equipped pieces. Swapping an item removes its modifiers and applies the new ones; displayed stats update instantly.

| **Stat**                 | **In-game label**  | **Role**                                                         | **Cap**   |
|--------------------------|--------------------|------------------------------------------------------------------|-----------|
| HP (current / max)       | Vida               | Survival. Items never change it (§6.4).                          | —         |
| Attack (ATK)             | Ataque             | Offensive power.                                                 | —         |
| Defence (DEF)            | Defensa            | Reduces incoming damage (proportional formula).                  | —         |
| Crit chance (CRIT %)     | Crítico            | Chance for a landed hit to be critical.                          | 50 %      |
| Crit damage (CRIT DMG)   | Poder crítico      | Extra damage on a critical.                                      | +150 %    |
| Extra turn (EXTRA %)     | Turno extra        | Chance to skip the enemy turn and attack again.                  | 35 %      |
| Luck (LUCK)              | Suerte             | Improves the expected quality of loot.                           | —         |
| Lifesteal (VAMP %)       | Vampirismo         | Heals a % of damage dealt (min 1 if VAMP > 0 and the hit lands). Shown as a green “+N” over the hero. | 25 % |
| Accuracy / miss          | —                  | Base miss chance for hero and enemies.                           | TBD       |

Caps are starting values. Stats above their cap have no extra effect; the stats screen shows them in gold with “max”.

## 5.1. Slots

| **Slot** (in-game)   | **Initial state**                                  |
|----------------------|----------------------------------------------------|
| Helmet (Casco)       | Basic piece equipped                               |
| Chestplate (Pechera) | Basic piece equipped                               |
| Boots (Botas)        | Basic piece equipped                               |
| Sword (Espada)       | Basic weapon equipped                              |
| Shield (Escudo)      | Empty at start; available for future drops         |

**Each item has at most 3 stats:** 1 primary and up to 2 secondaries, no repeats. Weapons mostly give offensive stats; armour and shield give defence plus secondary bonuses (crit, luck, extra turn or lifesteal). There is no inventory: every reward is either kept (replacing the current piece) or discarded.

## 5.2. Stats screen

> Currently not reachable: B was reassigned to the sound toggle. The panel code is kept for a future entry point.

During the hero’s turn, B opens a single **Atributos** (*Attributes*) panel: one row per attribute with its full name (Vida, Ataque, Defensa, Crítico, Poder crítico, Turno extra, Suerte, Vampirismo) and its value right-aligned; shown in gold with “max” when capped. Combat is paused; B goes back.

# 6. Rewards and healing

## 6.1. Victory heal

After every won fight the hero automatically recovers a **percentage of max HP** (`CURA_VICTORIA`, 20 %), never above max. Shown as a floating number before the loot screen.

**Pre-boss heal:** when a boss appears (with the “- Jefe -” banner) the hero recovers **50 % of the missing HP** (`CURA_JEFE`), shown as a floating number.

## 6.2. Loot

Every defeated enemy drops exactly 1 reward: a gear piece (sword, helmet, chestplate, boots or shield) or a potion. The encounter level sets the overall item scale; luck improves the odds of better stats, not the number of items.

Potion chance: 25 % (to be balanced). Bosses never drop potions.

### Rarities

| **Rarity**        | **Colour**              | **Base weight** | **Power** | **Secondaries**    |
|-------------------|-------------------------|-----------------|-----------|--------------------|
| Common (Común)    | Light (steel)           | 70 %            | ×1.0      | 0 or 1 (50 %)      |
| Rare (Rara)       | Spectral (biome accent) | 25 %            | ×1.25     | 1 or 2 (40 % two)  |
| Epic (Épica)      | Gold                    | 5 %             | ×1.55     | Always 2           |

- **LUCK**: each point moves 1 point of weight from Common to Rare (70 %) and Epic (30 %). Common never drops below 10 %. Item names and the hero’s gear are drawn in the rarity colour, so the effect of luck is visible.
- **Bosses** always drop a Rare or Epic item.

### Item generation

- **Primary stat**: ATK on the sword, DEF on everything else. Value = base × slot factor × per-encounter growth × rarity × **random ±25 %**.
- **Secondary stats** (by rarity): CRIT, CRIT DMG, EXTRA, LUCK or VAMP (CRIT DMG on weapons only). Small, almost flat values that grow slowly with level so caps are not broken.
- **Material** (name): changes every 6 encounters (Madera, Hierro, Acero, Plata, Mitril, Rúnico, Astral — wood, iron, steel, silver, mithril, runic, astral).
- **Can a drop be worse?** Yes. Because of the ±25 % roll and different (or missing) secondaries, a higher-level item can be worse than the current one. The comparison makes this obvious.

## 6.3. Gear: compare and decide

The loot screen shows two side-by-side panels: **equipped item on the left, new item on the right**.

- Each panel shows the item type large (serif 9 bold) and its rarity below, both in the rarity colour.
- The **equipped panel shows only its own stats**, as they are.
- The **new panel** lists its stats and, at the end, any stats that would be lost (“--”). It is the only panel with comparisons: a green ▲ (better) or red ▼ (worse) arrow with the difference next to each value.
- Stat lists use the 5×7 pixel font with generous line spacing.
- **The panels are the buttons**: B switches, A confirms. The selected panel has a double gold frame with pulsing corners; the other is dimmed.
- Equipping discards the old piece; keeping discards the drop.

## 6.4. Potions: restore or max HP

Potions are fairly common (25 %) and cannot be stored. On a potion drop the player picks one of two panels (same layout as gear):

| **Option**              | **Effect**                                                                                     |
|-------------------------|------------------------------------------------------------------------------------------------|
| Recuperar (*Restore*)   | Restores 50 % of the missing HP (`POCION_RECUPERA`).                                           |
| Vida máx. (*Max HP*)    | Permanently raises max HP (`POCION_HPMAX`, +20 %). Current HP does not change.                 |

Each panel shows the resulting HP large and the current HP below.

> **EXAMPLE**
>
> HP 23/50. Restore → 37/50 (half of the 27 missing, rounded). Max HP → 23/60. A choice between surviving now and being tougher later.

Since items never change max HP, these potions are the only way to raise it, which avoids odd cases when swapping gear.

## 6.5. Undead talisman

Beating a boss has a `PROB_TALISMAN` (35 %) chance to drop the **undead talisman**, if the hero doesn't already carry one and isn't already cursed. A small spectral skull appears in the corner of the hero's HUD plate while it is carried.

If the hero dies while carrying it, the run doesn't end: the talisman breaks and the hero rises as a **skeleton** (“Maldición no muerta”) and keeps fighting the same enemy. The skeleton keeps all the gear but is more fragile and clumsier:

- max HP drops to `NO_MUERTO_HPMAX` (60 %) of what it was, and HP is filled to that new max;
- `NO_MUERTO_FALLO` (+10 %) miss chance;
- `NO_MUERTO_DEF` (−2) base defence;
- the special-attack counter is reset.

It only works once per run: a cursed hero can't get another talisman, and dying again is game over. Until the skeleton art exists, the hero is drawn with faded colours and a spectral outline.

# 7. Difficulty and progression

Each new enemy is a step up in difficulty. Enemy HP, attack and defence grow exponentially with the encounter number (HP ×1.07, ATK ×1.05, DEF ×1.065 per encounter). Rewards scale too, without guaranteeing that every new piece is better. Since the hero only gains max HP through potions, difficulty eventually wins: every run ends.

There is no XP, gold, shop or hero level in this version.

## 7.1. Regular enemies

Picked **at random** among four types on every non-boss encounter.

| **Type** (in-game)     | **Profile**                       | **HP** | **ATK** | **DEF** | **CRIT** | **Miss** | **Prepares** |
|------------------------|-----------------------------------|--------|---------|---------|----------|----------|--------------|
| Salteador (*Highwayman*) | Balanced                        | ×1.0   | ×1.0    | ×1.0    | 5 %      | 10 %     | 15 %         |
| Golem                  | Tank                              | ×1.4   | ×0.8    | ×1.7    | 3 %      | 12 %     | 10 %         |
| Huargo (*Warg*)        | Agile: high crit, low DEF         | ×0.8   | ×0.95   | ×0.6    | 20 %     | 5 %      | 10 %         |
| Troll                  | Brute: hits hard and telegraphs   | ×1.15  | ×1.25   | ×0.85   | 5 %      | 14 %     | 35 %         |

## 7.2. Bosses

A boss **always** appears every 10 encounters. There are 5 unique bosses with their own larger sprites (up to ~100 px), appearing in order; after the fifth the sequence repeats at higher stats. Their loot is at least Rare. Each boss entrance has a banner and a short jingle.

| **Encounter** | **Boss** (in-game)                    | **Visual concept**                                | **HP** | **ATK** | **DEF** | **CRIT** | **Miss** | **Prepares** |
|---------------|---------------------------------------|---------------------------------------------------|--------|---------|---------|----------|----------|--------------|
| 10            | Rey Ciervo (*Stag King*)              | Forest giant with antlers and a crown of roots    | ×2.0   | ×1.1    | ×1.2    | 8 %      | 10 %     | 20 %         |
| 20            | Dama Espectral (*Spectral Lady*)      | Floating apparition with a veil and chains        | ×1.5   | ×1.15   | ×0.9    | 25 %     | 4 %      | 20 %         |
| 30            | Caballero Negro (*Black Knight*)      | Dark mirror of the hero, greatsword, torn cape    | ×1.6   | ×1.1    | ×1.2    | 10 %     | 8 %      | 25 %         |
| 40            | La Tejedora (*The Weaver*)            | Colossal spider with a pale mask                  | ×1.7   | ×1.2    | ×1.0    | 15 %     | 6 %      | 25 %         |
| 50            | Rey Liche (*Lich King*)               | Crowned skeleton with a staff of spectral fire    | ×2.2   | ×1.25   | ×1.25   | 12 %     | 6 %      | 35 %         |

## 7.3. Biomes

The biome changes **after every boss**: encounters 1–10 Forest (*El Bosque*), 11–20 Graveyard (*El Cementerio*), 21–30 Castle (*El Castillo*), then it cycles. Each biome has its own 8-colour palette (§8.2) and layered background; the hero, enemies and UI all take the biome palette.

# 8. Screens and presentation

| **Screen**        | **Content**                                                                                                                                   |
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| Title             | Hand-drawn logo (gold embossed “STICK”, a sword as underline, “R P G” between diamonds) that drops in and lands with a shake and a flash; a light glint sweeps the letters, sparkles twinkle and embers rise; “Una leyenda oscura”, “- Pulsa A -”, record and sound toggle (B). |
| Biome intro       | Scenery, chapter card “Capítulo II” and biome name.                                                                                           |
| Combat            | Hero on the left, enemy on the right, HUD at the bottom, combat messages, floating numbers, full-width charge bar at the top while charging. |
| Boss intro        | Black band with “- Jefe -” and the boss name in gold.                                                                                         |
| Stats             | Single “Atributos” panel (§5.2).                                                                                                              |
| Gear loot         | Two selectable panels (§6.3).                                                                                                                 |
| Potion loot       | Two selectable panels (§6.4).                                                                                                                 |
| Pause             | “Pausa” and how to resume (PWR).                                                                                                              |
| Defeat            | “Has caído”, enemies defeated, highest hit, record (“Nuevo récord!” if beaten) and “A: volver”.                                              |

## 8.1. Visual language

- 8 colours per biome, same role per index in every biome (§8.2). Keep text and HUD contrast high.
- **Typography**: titles and screens use FreeSerif 9–18 pt (bold/italic); the HUD and stat lists use our own 5×7 pixel font in bold.
- **HUD** at the bottom (keeps the sky free for tall bosses), as mirrored iron panels:
  - Hero (left): heart + “CABALLERO”, HP number + bar.
  - Centre: encounter number in **Roman numerals** and the 3 special runes.
  - Enemy (right): name + skull (crown for bosses), bar + remaining HP number.
- **Panels**: iron plates with a black fill, light top edge and gold rivets; segmented HP bars; selections framed in gold.
- Sprites must be readable at 240 × 135; critical info (HP, selection, charge, crit, miss, extra turn) must be readable at a glance.
- No scrolling or complex selection during combat: the game waits for A press/release.

## 8.2. Palettes (8 colours per biome)

Each index has a fixed role; the biome only changes the hue. Sprites and backgrounds are drawn once and the game swaps the palette when entering another biome. See `docs/STYLE_GUIDE.md` for the full art guide and importable palette files.

| **#** | **Role**   | **Use**                                         | **Forest** | **Graveyard** | **Castle** |
|-------|------------|-------------------------------------------------|------------|---------------|------------|
| 0     | Black      | Outlines, panels, near silhouettes              | `#0A0E0D`  | `#0B0A12`     | `#0E0A0A`  |
| 1     | Dark       | Sky, shadows, bar backgrounds                   | `#1B2925`  | `#221D33`     | `#2A1E1D`  |
| 2     | Mid        | Mist, chainmail, steel shadow                   | `#3B5247`  | `#433B5C`     | `#52403A`  |
| 3     | Light      | Steel, stone, Common rarity                     | `#7A9380`  | `#827A9A`     | `#8E7E72`  |
| 4     | Bright     | Text, moon, highlights                          | `#D4DEC8`  | `#DAD4E6`     | `#E4DACA`  |
| 5     | Blood      | HP, damage, cape and plume                      | `#9C2E30`  | `#A02C3E`     | `#B8302A`  |
| 6     | Gold       | Criticals, perfect zone, selection, Epic rarity | `#D6A248`  | `#D8B462`     | `#ECA43A`  |
| 7     | Spectral   | Biome magic, runes, charge, Rare rarity         | `#9FE05A`  | `#72E4D4`     | `#A472E0`  |

Indices 8–15 are unused; **index 15 is sprite transparency**.

## 8.3. Sound and music

Built-in buzzer with a non-blocking note sequencer.

| **Moment**        | **Sound**                                                             |
|-------------------|-----------------------------------------------------------------------|
| Title screen      | Short jingle: A-minor fanfare (~1.6 s)                                |
| Boss appears      | Micro jingle: menacing tritone (~0.9 s) plus screen shake             |
| New biome         | Rising arpeggio with the chapter card                                 |
| Victory           | Short arpeggio while the enemy dissolves                              |
| Defeat            | Descending melody                                                     |
| Combat            | Hit, miss, critical, extra turn, charge, perfect zone, enemy footsteps, heavy-blow warning |

Sound can be muted from the title screen (B); the setting is saved.

## 8.4. Combat text timing

| **Event**                       | **Duration** |
|---------------------------------|--------------|
| Hit / miss                      | 450 ms       |
| Critical / heavy blow           | 650 ms       |
| Extra turn                      | 500 ms       |
| Texts inside a chain            | 300 ms       |
| Pause before the enemy turn     | 450 ms       |
| “SE PREPARA!”                   | 700 ms       |

All texts from one hit are merged into a single message (e.g. “PERFECTO CRITICO +TURNO”). Combat messages use serif 12 pt when they fit, 9 pt otherwise. Damage appears as a number floating up over the target.

## 8.5. Effects and animation

- **Breathing**: hero and enemies alternate two idle frames; ghosts (Spectral Lady, Lich King) float.
- **Entrance**: each enemy walks in from the right and eases to a stop.
- **Hits**: the hero’s slash lands ~180 ms after the input; the target recoils and flashes white.
- **Screen shake**: strong when the hero takes a critical, medium on heavy blows, own crits or perfect specials, slight on normal hits.
- **Flash**: a perfect special briefly brightens the whole palette.
- **Death**: the enemy dissolves with a dither pattern.
- **Palette fades** between encounters; chapter card when the biome changes.
- **Special charge**: the sword is raised with spectral sparks and the bar spans the top of the screen.

## 8.6. Persistence

Stored in NVS: the record (enemies defeated) and the sound setting. The current run is not saved; it is lost on power-off.

# 9. Current feature set

- Title with animated logo, music and sound toggle.
- Knight with five slots (shield empty at start); gear visible on the sprite and tinted by rarity.
- Turn-based combat: normal attack, special every 3 hits with a cyclic bar and perfect zone, misses, criticals (enemies too), chained extra turns with halving odds, telegraphed heavy blows, lifesteal.
- Proportional damage, stat caps, gear recalculation.
- Victory heal and one reward per fight (gear comparison with up to 3 stats, or potion choice).
- 4 random regular enemies and 5 unique bosses every 10 encounters; 3 biomes with their own palettes and backgrounds.
- Sound toggle at any moment (B), pause (PWR), defeat screen with persistent record.

# 10. Balance parameters

| **Parameter**   | **Still to tune**                                                                                  |
|-----------------|----------------------------------------------------------------------------------------------------|
| Attacks         | `T_UMBRAL_MS`, charge cycle length, `CARGA_MIN`, `CARGA_MAX`, `ZONA_PERFECTA`, `CARGA_PERFECTA`.   |
| Accuracy        | Base miss chance (hero and enemies), whether charging affects it, whether any item improves it.    |
| Criticals       | Base CRIT % and CRIT DMG; validate caps (50 % / +150 %).                                           |
| Extra turn      | Base chance; validate cap (35 %) and chain halving.                                                |
| Enemies         | Starting stats, growth curve, miss/crit chances.                                                   |
| Loot            | Category odds, potion chance, quality formula, exact LUCK effect.                                  |
| Healing         | `CURA_VICTORIA` and `POCION_HPMAX`.                                                                |
| Hero            | Starting stats and basic gear.                                                                     |
| Heavy blow      | Multiplier (×2.5) and per-type frequency.                                                          |

All values live in `src/core/balance.h`. The simulator (`make sim`) plays thousands of runs with a bot player to tune them.

**Current simulator results** (5000 runs; `skill` = probability of releasing in the perfect zone):

| **Player**                    | **Mean** | **p10** | **Median** | **p90** |
|-------------------------------|----------|---------|------------|---------|
| Normal attacks only           | 16.5     | 9       | 17         | 22      |
| Special without timing (0.0)  | 18.2     | 11      | 19         | 25      |
| Special, skill 0.5            | 20.8     | 14      | 20         | 29      |
| Special, skill 0.9            | 23.3     | 17      | 23         | 29      |

Skill pays off while charging without timing barely beats normal attacks. Around 35 % of runs end at a boss; this may need review.

# 11. Sprites and graphics

See **`docs/STYLE_GUIDE.md`** for the full art guide (palettes, sizes, anchors, layers, animation tags, shading rules and how to deliver new art).

## 11.1. Art direction

- Dark medieval fantasy: a sinister forest of dead trees under a pale moon, a graveyard, a castle hall.
- Characters **~64 px** tall (bosses up to ~100 px), side view, black outline, light from the top-left with 3 tones per material.
- The hero does not walk: he stands guard on the left. Enemies **walk in** from the right.

## 11.2. Layered background (parallax)

| **Layer**    | **Content (forest)**                                     | **Movement**                                  |
|--------------|----------------------------------------------------------|-----------------------------------------------|
| Sky          | Dithered gradient, moon, stars, mist                     | Fixed                                         |
| Clouds       | Procedural (periodic noise), 2 layers, new per biome; none indoors | Slow continuous drift (2 speeds)   |
| Far          | Pine line                                                | Small shift per encounter                     |
| Fog          | Dithered wisps                                           | Static                                        |
| Mid          | Twisted dead trees with moonlight rim                    | Medium shift per encounter                    |
| Ground       | Soil, stones, tufts                                      | Large shift per encounter                     |
| *Characters* |                                                          |                                               |
| Front        | Black ferns at the edges                                 | Largest shift                                 |

All layers are 240 px wide and tile horizontally.

## 11.3. Hero

Knight on an 80 × 72 px canvas (feet at (30, 71)). Animations: `idle` (2), `attack` (3: wind-up, slash, follow-through), `charge` (sword raised while charging) and `hit` (1).

| **Layer** (bottom to top)  | **Drawn when…**    | **Recolour**          |
|----------------------------|--------------------|-----------------------|
| cuerpo (cape, mail)        | always             | —                     |
| botas (greaves)            | boots equipped     | Boots rarity          |
| pechera                    | chestplate equipped| Chestplate rarity     |
| casco                      | helmet equipped    | Helmet rarity         |
| escudo                     | shield equipped    | Shield rarity         |
| brazo (arm + sword)        | always             | Sword rarity          |

Gear uses the Mid, Light and Bright tones; by rarity, Light becomes Spectral (Rare) or Gold (Epic).

## 11.4. Pipeline

1. `tools/art/*.py` generate the sprites, backgrounds and title logo (custom rasteriser with shading and outlines), plus a PC mock-up (`tools/art/mock.py`).
2. Output is written to `assets/sprites/*.aseprite` (hand-editable source); `make sprites` exports them to `src/gfx/*_gfx.h` (4 bpp).
3. The firmware draws directly into the 4 bpp frame buffer to move several 240 px layers at 30 fps.

`make art` regenerates everything from the scripts and **overwrites hand edits** in the `.aseprite` files. After editing an `.aseprite` by hand, run only `make sprites`.

# 12. Technical structure

| **Path**                | **Content**                                                            |
|-------------------------|------------------------------------------------------------------------|
| `src/core/`             | Game rules in plain C++, no Arduino (combat, loot, enemies)            |
| `src/core/balance.h`    | All balance parameters                                                 |
| `src/main.cpp`          | Firmware: states, input, rendering, UI, effects                        |
| `src/audio.*`           | Buzzer sequencer, jingles and effects                                  |
| `src/clouds.*`          | Procedural clouds                                                      |
| `src/palette.h`         | Palette roles (indices)                                                |
| `src/gfx/`              | 4 bpp drawing (`canvas4.h`), sprite types, exported headers (`*_gfx.h`) |
| `assets/sprites/`       | `.aseprite` sources                                                    |
| `tools/art/`            | Art generators, palettes, pixel font, mock-up                          |
| `tools/sprites/`        | Aseprite scripts: assemble and export to C                             |
| `tools/sim/`            | PC balance simulator                                                   |
| `tools/test/`           | Logic tests                                                            |
| `docs/`                 | Style guide, palettes and mock-ups                                     |

- Firmware: `pio run -t upload` (PlatformIO + M5Unified).
- PC: `make test` and `make sim ARGS="--skill 0.7"`.
- Art: `make sprites` (needs Aseprite; path configurable with `ASEPRITE=`), `make art` to regenerate from scripts.

> **DESIGN PRINCIPLE**
>
> Keep systems simple and decisions meaningful: when to release the special, how to use extra turns, which gear to keep, and whether to heal now or grow stronger later. Any new mechanic must justify its cost in screen space, controls and clarity.
