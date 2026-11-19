// Stick RPG — firmware para M5StickC Plus2.
// Escenas, entrada, render (lienzo de 4 bpp con paleta de 8 colores por bioma),
// efectos y sonido. Las reglas del juego viven en src/core.

#include <M5Unified.h>
#include <Preferences.h>

#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio.h"
#include "clouds.h"
#include "core/balance.h"
#include "core/combat.h"
#include "core/progression.h"
#include "gfx/assets.h"
#include "gfx/canvas4.h"
#include "palette.h"

namespace {

using gfx::Remap;

constexpr int W = 240;
constexpr int H = 135;
constexpr int FEET_Y = 113;     // fila bajo los pies
constexpr int HERO_FX = 50;     // pies del héroe
constexpr int ENEMY_FX = 186;   // pies del enemigo
constexpr int ENEMY_START = 300;
constexpr uint32_t FRAME_MS = 33;
constexpr uint32_t T_APAGAR_MS = 2000;
constexpr uint32_t T_ATENUAR_MS = 10000;
constexpr uint8_t BRILLO_PANTALLA = 120;
constexpr uint8_t BRILLO_PAUSA = 12;

constexpr uint32_t T_FADE = 320;
constexpr uint32_t T_BIOMA = 2200;
constexpr uint32_t T_ENTRADA = 1300;
constexpr uint32_t T_JEFE = 2000;
constexpr uint32_t T_GOLPE_HEROE = 180;    // hasta el frame del tajo
constexpr uint32_t T_GOLPE_ENEMIGO = 200;
constexpr uint32_t T_MUERTE = 800;
constexpr uint32_t T_VICTORIA = 700;

M5Canvas cv(&M5.Display);
gfx::Canvas4 c4;
Preferences prefs;
Rng rng;
Hero hero;
Enemy enemy;
Battle battle(hero, enemy, rng);

enum class St : uint8_t {
  TITLE, FADE, BIOME_INTRO, ENEMY_ENTER, BOSS_INTRO, HERO_TURN, HERO_STRIKE,
  SHOW, ENEMY_WAIT, ENEMY_STRIKE, ENEMY_DIE, VICTORY, LOOT_ITEM, LOOT_POTION,
  GAME_OVER
};
enum class FadeAct : uint8_t { NEW_GAME, NEXT_ENCOUNTER, TO_TITLE };

St st = St::TITLE;
uint32_t gt = 0, stT = 0;

// Transiciones.
St fadeNext = St::TITLE;
FadeAct fadeAct = FadeAct::TO_TITLE;
bool fadeDone = false;
float brightness = 1.0f, flashAmt = 0.0f;
int curBiome = 0, palBiome = -1;
float palBri = -1, palFlash = -1;

// Mensaje central y números flotantes.
char msg[28] = "";
uint8_t msgColor = pal::BRILLO;
St showNext = St::HERO_TURN;
uint32_t showDur = 0;

struct Floater {
  bool on = false;
  char text[12] = "";
  uint8_t color = pal::BRILLO;
  int x = 0, y = 0;
  uint32_t t0 = 0;
};
Floater floaters[3];

// Partida.
int nivel = 1, derrotados = 0, golpeMax = 0;
uint32_t record = 0;
bool nuevoRecord = false;

// UI y efectos.
bool statsOpen = false, paused = false;
uint32_t pausedAtMs = 0;
bool aArmed = false, charging = false, inPerfect = false;
uint32_t aPressMs = 0;
float chargeF = 0;
bool pendingSpecial = false, pendingPerfect = false;
float pendingMult = 1.0f;
constexpr uint32_t LEJOS = 0x40000000u;  // "hace mucho" para los temporizadores
uint32_t heroAtkT = 0, heroHitT = 0, enemyAtkT = 0, enemyHitT = 0;
uint32_t shakeT = 0, shakeDur = 0;
int shakeAmp = 0;
int enemyX = ENEMY_START;
Loot loot;
int sel = 0;

// ---------------------------------------------------------------- utilidades

uint32_t since(uint32_t t) { return gt - t; }
float lerp(float a, float b, float t) { return a + (b - a) * t; }
float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

void resetTimers() {
  heroAtkT = heroHitT = enemyAtkT = enemyHitT = gt - LEJOS;
}

void shake(int amp, uint32_t dur) {
  shakeAmp = amp;
  shakeDur = dur;
  shakeT = gt;
}

void floatText(const char* text, uint8_t color, int x, int y) {
  Floater* f = &floaters[0];
  for (Floater& c : floaters)
    if (!c.on) { f = &c; break; }
  f->on = true;
  std::snprintf(f->text, sizeof(f->text), "%s", text);
  f->color = color;
  f->x = x;
  f->y = y;
  f->t0 = gt;
}

void setState(St s);
void initTitle();

void show(const char* text, uint8_t color, uint32_t dur, St next) {
  std::snprintf(msg, sizeof(msg), "%s", text);
  msgColor = color;
  showDur = dur;
  showNext = next;
  setState(St::SHOW);
}

void fadeTo(FadeAct act) {
  fadeAct = act;
  fadeDone = false;
  setState(St::FADE);
}

const gfx::EnemyArt& art() { return gfx::kEnemyArt[static_cast<int>(enemy.type)]; }
int enemyTop() { return FEET_Y - art().ay; }

void roman(int n, char* out) {
  static const int v[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
  static const char* s[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL",
                            "X", "IX", "V", "IV", "I"};
  out[0] = 0;
  for (int i = 0; i < 13; ++i)
    while (n >= v[i] && std::strlen(out) < 12) {
      std::strcat(out, s[i]);
      n -= v[i];
    }
}

// ------------------------------------------------------------------- paleta

// Paleta del bioma con brillo (fundidos) y destello hacia el color claro.
void applyPalette() {
  if (curBiome == palBiome && brightness == palBri && flashAmt == palFlash)
    return;
  palBiome = curBiome;
  palBri = brightness;
  palFlash = flashAmt;
  const uint8_t(*p)[3] = gfx::kBiomePalette[curBiome];
  const uint8_t* wht = p[pal::BRILLO];
  for (int i = 0; i < 8; ++i) {
    uint8_t c[3];
    for (int k = 0; k < 3; ++k)
      c[k] = static_cast<uint8_t>(lerp(p[i][k], wht[k], flashAmt) * brightness);
    cv.setPaletteColor(i, c[0], c[1], c[2]);
  }
}

// ------------------------------------------------------------------- partida

void startEncounter() {
  enemy = makeEnemy(rng, nivel);
  battle.reset();
  enemyX = ENEMY_START;
  resetTimers();
  for (Floater& f : floaters) f.on = false;
}

void newGame() {
  hero = makeHero();
  nivel = 1;
  derrotados = 0;
  golpeMax = 0;
  nuevoRecord = false;
  curBiome = -1;
}

// Llamado con la pantalla en negro durante un fundido.
void fadeMid() {
  switch (fadeAct) {
    case FadeAct::NEW_GAME:
      newGame();
      [[fallthrough]];
    case FadeAct::NEXT_ENCOUNTER: {
      startEncounter();
      int b = static_cast<int>(biomaDe(nivel));
      fadeNext = b != curBiome ? St::BIOME_INTRO : St::ENEMY_ENTER;
      if (b != curBiome) clouds::generate(esp_random());
      curBiome = b;
      break;
    }
    case FadeAct::TO_TITLE:
      curBiome = 0;
      clouds::generate(esp_random());
      hero = makeHero();
      resetTimers();
      fadeNext = St::TITLE;
      break;
  }
}

void onEnter(St s) {
  switch (s) {
    case St::TITLE:
      initTitle();
      audio::jingleInicio();
      break;
    case St::BIOME_INTRO:
      audio::jingleBioma();
      break;
    case St::BOSS_INTRO:
      audio::jingleJefe();
      shake(2, 600);
      break;
    case St::HERO_TURN:
      aArmed = charging = inPerfect = false;
      statsOpen = false;
      break;
    case St::ENEMY_DIE:
      ++derrotados;
      audio::jingleVictoria();
      break;
    case St::VICTORY: {
      int healed = healVictory(hero);
      char buf[12];
      std::snprintf(buf, sizeof(buf), "+%d", healed);
      if (healed > 0) floatText(buf, pal::ESPECTRAL, HERO_FX, FEET_Y - 82);
      break;
    }
    case St::GAME_OVER:
      if (static_cast<uint32_t>(derrotados) > record) {
        record = derrotados;
        nuevoRecord = true;
        prefs.putUInt("record", record);
      }
      audio::jingleDerrota();
      break;
    default:
      break;
  }
}

void setState(St s) {
  st = s;
  stT = gt;
  onEnter(s);
}

// ------------------------------------------------------------------- combate

void resolveHeroAttack() {
  HeroTurnResult r = pendingSpecial ? battle.heroSpecial(pendingMult)
                                    : battle.heroAttack();
  if (r.atk.damage > golpeMax) golpeMax = r.atk.damage;
  int fx = ENEMY_FX - 6, fy = enemyTop() - 4;
  if (fy < 20) fy = 20;

  char buf[28] = "";
  uint8_t color = pal::BRILLO;
  uint32_t dur = bal::T_TEXTO_GOLPE;
  if (!r.atk.hit) {
    floatText("FALLO", pal::CLARO, fx, fy);
    audio::sfx(220, 60);
  } else {
    char d[12];
    std::snprintf(d, sizeof(d), "%d", r.atk.damage);
    floatText(d, r.atk.crit ? pal::ORO : pal::BRILLO, fx, fy);
    enemyHitT = gt;
    if (r.healed > 0) {
      std::snprintf(d, sizeof(d), "+%d", r.healed);
      floatText(d, pal::ESPECTRAL, HERO_FX, FEET_Y - 82);
    }
    if (pendingPerfect) {
      std::strcat(buf, "PERFECTO ");
      flashAmt = 0.8f;
      shake(3, 260);
    }
    if (r.atk.crit) {
      std::strcat(buf, "CRITICO ");
      dur = bal::T_TEXTO_CRITICO;
      shake(2, 200);
    }
    if (buf[0]) color = pal::ORO;
    audio::sfx(r.atk.crit || pendingPerfect ? 1320 : 660,
               r.atk.crit ? 90 : 50);
  }
  pendingSpecial = pendingPerfect = false;
  if (r.enemyDead) {
    show(buf, color, dur, St::ENEMY_DIE);
  } else if (r.extraTurn) {
    std::strcat(buf, "+TURNO");
    uint32_t d = battle.chain() > 1 ? bal::T_TEXTO_CADENA : bal::T_TEXTO_EXTRA;
    show(buf, buf[0] == '+' ? pal::ESPECTRAL : pal::ORO, d > dur ? d : dur,
         St::HERO_TURN);
    audio::sfx(990, 60);
  } else {
    show(buf, color, dur, St::ENEMY_WAIT);
  }
}

void startHeroAttack(bool special, float mult, bool perfect) {
  pendingSpecial = special;
  pendingMult = mult;
  pendingPerfect = perfect;
  heroAtkT = gt;
  audio::sfx(special ? 520 : 400, 30);
  setState(St::HERO_STRIKE);
}

void resolveEnemyTurn() {
  EnemyTurnResult r = battle.enemyTurn();
  if (r.action == EnemyAction::PREPARAR) {
    enemyAtkT = gt - LEJOS;
    audio::sfx(300, 160);
    show("SE PREPARA!", pal::SANGRE, 700, St::HERO_TURN);
    return;
  }
  bool heavy = r.action == EnemyAction::GOLPE_FUERTE;
  const char* label = heavy ? "GOLPE FUERTE" : "";
  if (!r.atk.hit) {
    floatText("FALLA", pal::CLARO, HERO_FX, FEET_Y - 82);
    audio::sfx(220, 60);
  } else {
    char d[12];
    std::snprintf(d, sizeof(d), "%d", r.atk.damage);
    floatText(d, pal::SANGRE, HERO_FX, FEET_Y - 82);
    heroHitT = gt;
    if (r.atk.crit) {
      label = heavy ? "GOLPE FUERTE CRITICO" : "CRITICO";
      shake(5, 380);  // el golpe crítico recibido hace temblar la pantalla
    } else if (heavy) {
      shake(3, 260);
    } else {
      shake(1, 120);
    }
    audio::sfx(r.atk.crit ? 110 : 160, r.atk.crit ? 160 : 90, true);
  }
  uint8_t color = heavy || r.atk.crit ? pal::SANGRE : pal::BRILLO;
  uint32_t dur = label[0] ? bal::T_TEXTO_CRITICO : bal::T_TEXTO_GOLPE;
  show(label, color, dur, r.heroDead ? St::GAME_OVER : St::HERO_TURN);
}

// ------------------------------------------------------------------- update

void updateTitle() {
  if (M5.BtnB.wasPressed()) {
    audio::setEnabled(!audio::enabled());
    prefs.putBool("sound", audio::enabled());
    audio::sfx(880, 60);
  }
  if (M5.BtnA.wasPressed() && since(stT) > 300) {
    audio::sfx(660, 60, true);
    fadeTo(FadeAct::NEW_GAME);
  }
}

void updateFade() {
  uint32_t t = since(stT);
  if (t < T_FADE) {
    brightness = 1.0f - t / static_cast<float>(T_FADE);
    return;
  }
  if (!fadeDone) {
    fadeDone = true;
    fadeMid();
  }
  float u = (t - T_FADE) / static_cast<float>(T_FADE);
  brightness = clamp01(u);
  if (u >= 1.0f) {
    brightness = 1.0f;
    setState(fadeNext);
  }
}

void updateHeroTurn() {
  if (!charging && M5.BtnB.wasPressed()) {
    statsOpen = !statsOpen;
    audio::sfx(770, 30);
    return;
  }
  if (statsOpen) return;

  // Sin el especial disponible (§4.3), A ataca al pulsar.
  if (M5.BtnA.wasPressed() && !hero.especialListo()) {
    startHeroAttack(false, 1.0f, false);
    return;
  }
  if (M5.BtnA.wasPressed()) {
    aArmed = true;
    aPressMs = millis();
    charging = inPerfect = false;
  }
  if (!aArmed) return;
  uint32_t held = millis() - aPressMs;
  if (M5.BtnA.isPressed()) {
    if (held > bal::T_UMBRAL_MS) {
      if (!charging) audio::sfx(600, 30);
      charging = true;
      chargeF = chargeFill(held);
      bool perf = chargeIsPerfect(chargeF);
      if (perf && !inPerfect) audio::sfx(1760, 40);
      inPerfect = perf;
    }
  } else {
    bool charged = held > bal::T_UMBRAL_MS;
    float f = chargeFill(held);
    aArmed = charging = false;
    startHeroAttack(charged, charged ? chargeMultiplier(f) : 1.0f,
                    charged && chargeIsPerfect(f));
  }
}

void updateLoot(bool potion) {
  if (since(stT) < 300) return;
  if (M5.BtnB.wasPressed()) {
    sel ^= 1;
    audio::sfx(770, 30);
  }
  if (M5.BtnA.wasPressed()) {
    audio::sfx(990, 60);
    if (potion) {
      if (sel == 0) potionRecover(hero);
      else potionMaxHp(hero);
    } else if (sel == 1) {
      equipItem(hero, loot.item);
    }
    ++nivel;
    fadeTo(FadeAct::NEXT_ENCOUNTER);
  }
}

void update() {
  for (Floater& f : floaters) f.on = f.on && since(f.t0) < 800;
  if (flashAmt > 0) flashAmt = flashAmt > 0.08f ? flashAmt - 0.08f : 0.0f;

  switch (st) {
    case St::TITLE: updateTitle(); break;
    case St::FADE: updateFade(); break;
    case St::BIOME_INTRO:
      if (since(stT) > T_BIOMA || (since(stT) > 600 && M5.BtnA.wasPressed()))
        setState(St::ENEMY_ENTER);
      break;
    case St::ENEMY_ENTER: {
      float u = clamp01(since(stT) / static_cast<float>(T_ENTRADA));
      float e = 1 - (1 - u) * (1 - u);  // frena al llegar
      enemyX = static_cast<int>(lerp(ENEMY_START, ENEMY_FX, e));
      const gfx::Anim& wa = art().walk;
      if ((since(stT) % (wa.ms * 2)) < FRAME_MS && u < 0.9f) audio::sfx(140, 20);
      if (u >= 1) setState(isBoss(enemy.type) ? St::BOSS_INTRO : St::HERO_TURN);
      break;
    }
    case St::BOSS_INTRO:
      if (since(stT) > T_JEFE) setState(St::HERO_TURN);
      break;
    case St::HERO_TURN: updateHeroTurn(); break;
    case St::HERO_STRIKE:
      if (since(stT) >= T_GOLPE_HEROE) resolveHeroAttack();
      break;
    case St::SHOW:
      if (since(stT) >= showDur) setState(showNext);
      break;
    case St::ENEMY_WAIT:
      if (since(stT) >= bal::T_TURNO_ENEMIGO) {
        enemyAtkT = gt;
        setState(St::ENEMY_STRIKE);
      }
      break;
    case St::ENEMY_STRIKE:
      if (since(stT) >= T_GOLPE_ENEMIGO) resolveEnemyTurn();
      break;
    case St::ENEMY_DIE:
      if (since(stT) >= T_MUERTE) setState(St::VICTORY);
      break;
    case St::VICTORY:
      if (since(stT) >= T_VICTORIA) {
        loot = rollLoot(rng, nivel, hero.eff(Stat::LUCK), isBossLevel(nivel));
        sel = 0;
        setState(loot.pocion ? St::LOOT_POTION : St::LOOT_ITEM);
      }
      break;
    case St::LOOT_ITEM: updateLoot(false); break;
    case St::LOOT_POTION: updateLoot(true); break;
    case St::GAME_OVER:
      if (since(stT) > 900 && M5.BtnA.wasPressed()) {
        audio::sfx(660, 60, true);
        fadeTo(FadeAct::TO_TITLE);
      }
      break;
  }
}

// ------------------------------------------------------------------- render

const char* const HEART[] = {".##.##.", "#rr#rr#", "#rrrrr#", ".#rrr#.",
                             "..#r#..", "...#..."};
const char* const SKULL[] = {".#####.", "#ccccc#", "#c#c#c#", "#ccccc#",
                             ".#c#c#.", "..###.."};
const char* const CROWN[] = {"o..o..o", "oo.o.oo", "ooooooo", "o#o#o#o",
                             "ooooooo"};
const char* const RUNE[] = {"...#...", "..#o#..", ".#ooo#.", "#ooooo#",
                            ".#ooo#.", "..#o#..", "...#..."};

void icon(const char* const* rows, int n, int x, int y, char k1, uint8_t c1,
          char k2, uint8_t c2) {
  for (int j = 0; j < n; ++j)
    for (int i = 0; rows[j][i]; ++i) {
      char ch = rows[j][i];
      if (ch == k1) c4.pset(x + i, y + j, c1);
      else if (ch == k2) c4.pset(x + i, y + j, c2);
    }
}

// Placa de hierro con remaches dorados.
void panel(int x0, int y0, int x1, int y1) {
  c4.rect(x0, y0, x1 - x0 + 1, y1 - y0 + 1, pal::NEGRO);
  for (int x = x0 + 1; x < x1; ++x) {
    c4.pset(x, y0 + 1, pal::CLARO);
    c4.pset(x, y1 - 1, pal::OSCURO);
  }
  for (int y = y0 + 1; y < y1; ++y) {
    c4.pset(x0 + 1, y, pal::MEDIO);
    c4.pset(x1 - 1, y, pal::OSCURO);
  }
  c4.pset(x0 + 1, y0 + 1, pal::ORO);
  c4.pset(x1 - 1, y0 + 1, pal::ORO);
  c4.pset(x0 + 1, y1 - 1, pal::ORO);
  c4.pset(x1 - 1, y1 - 1, pal::ORO);
}

void bar(int x0, int y0, int w, int h, int v, int max, uint8_t color) {
  c4.rect(x0, y0, w, h, pal::OSCURO);
  int fill = max > 0 ? (w - 2) * v / max : 0;
  if (v > 0 && fill < 1) fill = 1;
  for (int x = x0 + 1; x < x0 + 1 + fill; ++x) {
    bool gap = (x - x0) % 6 == 0;
    for (int y = y0 + 1; y < y0 + h - 1; ++y)
      c4.pset(x, y, gap ? pal::NEGRO : color);
    if (!gap && (x & 1)) c4.pset(x, y0 + 1, pal::BRILLO);
  }
}

// Texto con fuente Serif (GFX). Fuera de los paneles lleva contorno negro.
enum Out : uint8_t { NONE, SHADOW, FULL };
const lgfx::IFont* const S9 = &fonts::FreeSerif9pt7b;
const lgfx::IFont* const I9 = &fonts::FreeSerifItalic9pt7b;
const lgfx::IFont* const B9 = &fonts::FreeSerifBold9pt7b;
const lgfx::IFont* const B12 = &fonts::FreeSerifBold12pt7b;
const lgfx::IFont* const B18 = &fonts::FreeSerifBold18pt7b;

void txt(const lgfx::IFont* font, const char* s, int x, int y, uint8_t color,
         uint8_t datum = middle_center, Out out = FULL) {
  cv.setFont(font);
  cv.setTextDatum(datum);
  if (out != NONE) {
    cv.setTextColor(pal::NEGRO);
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx)
        if ((dx || dy) && (out == FULL || (dx == 1 && dy == 1)))
          cv.drawString(s, x + dx, y + dy);
  }
  cv.setTextColor(color);
  cv.drawString(s, x, y);
}

void serif(const lgfx::IFont* font, const char* s, int x, int y, uint8_t color) {
  txt(font, s, x, y, color);
}

Remap rarityRemap(const Item& it) {
  Remap r = Remap::identity();
  if (!it.present) return r;
  if (it.rareza == Rareza::RARA) r.to[pal::CLARO] = pal::ESPECTRAL;
  else if (it.rareza == Rareza::EPICA) r.to[pal::CLARO] = pal::ORO;
  return r;
}

uint8_t rarityColor(Rareza r) {
  switch (r) {
    case Rareza::RARA: return pal::ESPECTRAL;
    case Rareza::EPICA: return pal::ORO;
    default: return pal::CLARO;
  }
}

void drawHero() {
  namespace H = gfx::heroe;
  int x0 = HERO_FX - gfx::HERO_AX, y0 = FEET_Y - gfx::HERO_AY;
  int frame;
  bool flash = false;
  bool isCharging = charging && st == St::HERO_TURN;
  if (st == St::GAME_OVER) {
    frame = H::HIT.from;
  } else if (since(heroHitT) < 280) {
    frame = H::HIT.from;
    x0 -= 2;
    flash = (gt / 60) % 2 == 0;
  } else if (since(heroAtkT) < H::ATTACK.duration() + 60) {
    frame = H::ATTACK.frameAt(since(heroAtkT), false);
  } else if (isCharging) {
    frame = H::CHARGE.from;
  } else {
    frame = H::IDLE.frameAt(gt);  // respiración
  }
  static const Remap white = Remap::solid(pal::BRILLO);
  struct { int layer; int slot; } capas[] = {
      {H::CUERPO, -1}, {H::BOTAS, (int)Slot::BOTAS},
      {H::PECHERA, (int)Slot::PECHERA}, {H::CASCO, (int)Slot::CASCO},
      {H::ESCUDO, (int)Slot::ESCUDO}, {H::BRAZO, (int)Slot::ESPADA},
  };
  for (auto& c : capas) {
    Remap r = Remap::identity();
    if (c.slot >= 0) {
      const Item& it = hero.equip[c.slot];
      if (!it.present && c.layer != H::BRAZO) continue;
      r = rarityRemap(it);
    }
    c4.blit(H::sheet, c.layer, frame, x0, y0, flash ? white : r);
  }
  // Chispas espectrales mientras se carga el especial.
  if (isCharging) {
    for (int i = 0; i < 12; ++i) {
      int sx = x0 + 24 + rng.range(-10, 10);
      int sy = y0 - 4 + rng.range(-8, 22);
      c4.pset(sx, sy, inPerfect ? pal::ORO : (i & 1 ? pal::ESPECTRAL : pal::BRILLO));
    }
  }
}

void drawEnemy(int dissolve = 0) {
  const gfx::EnemyArt& a = art();
  int x0 = enemyX - a.ax, y0 = FEET_Y - a.ay;
  int frame;
  bool flash = false;
  if (st == St::ENEMY_ENTER) {
    frame = a.walk.frameAt(since(stT));
  } else if (st == St::ENEMY_DIE) {
    frame = a.hit.from;
  } else if (since(enemyHitT) < 280) {
    frame = a.hit.from;
    x0 += 3;
    flash = (gt / 60) % 2 == 0;
  } else if (since(enemyAtkT) < a.attack.duration() + 80) {
    frame = a.attack.frameAt(since(enemyAtkT), false);
    x0 -= 6;
  } else if (enemy.preparado) {
    frame = a.prepare.from;
  } else {
    frame = a.idle.frameAt(gt);  // respiración
  }
  static const Remap white = Remap::solid(pal::BRILLO);
  c4.blit(*a.sheet, 0, frame, x0, y0, flash ? white : Remap::identity(),
          dissolve);
  if (enemy.preparado && st != St::ENEMY_DIE && (gt / 220) % 2 == 0) {
    int ty = y0 < 24 ? 24 : y0;
    serif(&fonts::FreeSerifBold12pt7b, "!", enemyX, ty, pal::SANGRE);
  }
}

void drawBackground(int fromLayer, int toLayer) {
  const gfx::Sheet& bg = *gfx::kBiomeBg[curBiome];
  for (int l = fromLayer; l < toLayer; ++l) {
    c4.blitWrap(bg, l, gfx::kParallax[l] * nivel);
    // Nubes procedurales entre el cielo y la capa lejana (no en interiores).
    if (l == 0 && curBiome != static_cast<int>(Bioma::CASTILLO))
      clouds::draw(c4, gt);
  }
}

const char* const RUNE5[] = {"..#..", ".#o#.", "#ooo#", ".#o#.", "..#.."};

void drawHud() {
  char buf[24];
  // Héroe (espejo de la placa del enemigo): nombre arriba; vida y barra debajo.
  panel(1, 112, 98, 134);
  icon(HEART, 6, 5, 116, '#', pal::NEGRO, 'r', pal::SANGRE);
  c4.label("CABALLERO", 15, 116, pal::BRILLO, pal::NEGRO);
  std::snprintf(buf, sizeof(buf), "%d/%d", hero.hp, hero.hpMax);
  int hw = gfx::Canvas4::textWidth(buf, 1, true);
  c4.label(buf, 4, 125, pal::BRILLO, pal::NEGRO);
  bar(hw + 7, 125, 95 - hw - 7, 6, hero.hp, hero.hpMax, pal::SANGRE);

  // Centro: encuentro en números romanos y runas del especial.
  panel(100, 112, 135, 134);
  roman(nivel, buf);
  c4.label(buf, 118, 116, isBossLevel(nivel) ? pal::SANGRE : pal::ORO,
           pal::NEGRO, 1);
  bool ready = hero.especialListo();
  for (int i = 0; i < bal::ESPECIAL_CADA; ++i) {
    bool on = i < hero.golpesEspecial;
    bool blink = ready && (gt / 250) % 2 == 0;
    icon(RUNE5, 5, 108 + i * 7, 125, '#', on ? pal::MEDIO : pal::OSCURO, 'o',
         on ? (blink ? pal::BRILLO : pal::ESPECTRAL) : pal::NEGRO);
  }

  // Enemigo: nombre arriba; barra y vida restante debajo.
  panel(137, 112, 238, 134);
  bool boss = isBoss(enemy.type);
  if (boss) icon(CROWN, 5, 228, 117, 'o', pal::ORO, '#', pal::SANGRE);
  else icon(SKULL, 6, 228, 116, '#', pal::NEGRO, 'c', pal::CLARO);
  const char* name = enemyName(enemy.type);
  bool fits = gfx::Canvas4::textWidth(name, 1, true) <= 84;
  c4.label(name, 224, 116, boss ? pal::ORO : pal::BRILLO, pal::NEGRO, 2, 1,
           false, fits);
  std::snprintf(buf, sizeof(buf), "%d/%d", enemy.hp, enemy.hpMax);
  int nw = gfx::Canvas4::textWidth(buf, 1, true);
  c4.label(buf, 235, 125, pal::BRILLO, pal::NEGRO, 2);
  bar(140, 125, 235 - nw - 143, 6, enemy.hp, enemy.hpMax, pal::SANGRE);
}

void drawChargeBar() {
  int x0 = 12, y0 = 28, w = 76;
  panel(x0 - 3, y0 - 3, x0 + w + 2, y0 + 9);
  int zone = static_cast<int>(w * bal::ZONA_PERFECTA);
  c4.rect(x0, y0, w, 7, pal::OSCURO);
  c4.shade(x0 + zone, y0, w - zone, 7, pal::ORO, 8);
  int f = static_cast<int>(w * chargeF);
  c4.rect(x0, y0 + 1, f, 5, inPerfect ? pal::ORO : pal::ESPECTRAL);
  for (int y = y0 - 2; y < y0 + 9; ++y) c4.pset(x0 + zone, y, pal::ORO);
}

void drawFloaters() {
  for (const Floater& f : floaters) {
    if (!f.on) continue;
    int dy = static_cast<int>(since(f.t0) / 22);
    c4.label(f.text, f.x, f.y - dy, f.color, pal::NEGRO, 1, 2, true, false);
  }
}

void drawStatsPanel() {
  panel(4, 4, 235, 109);
  txt(B9, "Caballero", 12, 15, pal::ORO, middle_left, NONE);
  c4.label("B: CERRAR", 228, 12, pal::MEDIO, pal::NEGRO, 2, 1, false, false);
  char buf[28];
  std::snprintf(buf, sizeof(buf), "Vida %d/%d", hero.hp, hero.hpMax);
  txt(B9, buf, 12, 32, pal::SANGRE, middle_left, NONE);
  // Estadísticas en dos columnas (ATK..VAMP sin la vida).
  for (int i = 0; i < STAT_COUNT; ++i) {
    Stat s = static_cast<Stat>(i);
    int col = i / 4, row = i % 4;
    bool capped = hero.raw(s) > hero.eff(s);
    std::snprintf(buf, sizeof(buf), "%s %d%s", statName(s), hero.eff(s),
                  statIsPercent(s) ? "%" : "");
    txt(S9, buf, 12 + col * 58, 49 + row * 14, capped ? pal::ORO : pal::BRILLO,
        middle_left, NONE);
  }
  for (int y = 24; y < 104; ++y)
    if (y % 2) c4.pset(124, y, pal::MEDIO);
  for (int i = 0; i < SLOT_COUNT; ++i) {
    itemName(hero.equip[i], buf, sizeof(buf));
    txt(S9, buf, 130, 32 + i * 16,
        hero.equip[i].present ? rarityColor(hero.equip[i].rareza) : pal::MEDIO,
        middle_left, NONE);
  }
}

void button(const char* label, int cx, int y, bool selected) {
  cv.setFont(B9);
  int w = cv.textWidth(label) + 18;
  int x0 = cx - w / 2;
  if (selected) {
    panel(x0, y, x0 + w, y + 16);
    c4.frame(x0, y, w + 1, 17, pal::ORO);
    c4.label(">", x0 + 4, y + 5, pal::ORO, pal::NEGRO);
  } else {
    c4.rect(x0, y, w + 1, 17, pal::NEGRO);
    c4.frame(x0, y, w + 1, 17, pal::OSCURO);
  }
  txt(B9, label, cx + 3, y + 8, selected ? pal::BRILLO : pal::MEDIO,
      middle_center, NONE);
}

void drawSceneNoEnemy() {
  drawBackground(0, gfx::BG_LAYER_PERSONAJES);
  drawHero();
  drawBackground(gfx::BG_LAYER_PERSONAJES, 6);
}


// Etiqueta de mejora/empeora: fondo de color, flecha y número en negrita.
void badge(int x, int y, int d) {
  bool up = d > 0;
  uint8_t bgc = up ? pal::ESPECTRAL : pal::SANGRE;
  uint8_t fg = up ? pal::NEGRO : pal::BRILLO;
  char num[12];
  std::snprintf(num, sizeof(num), "%d", up ? d : -d);
  int w = 10 + gfx::Canvas4::textWidth(num, 1, true) + 3;
  c4.rect(x + 1, y, w - 2, 11, bgc);
  c4.rect(x, y + 1, w, 9, bgc);
  for (int k = 0; k < 4; ++k)  // flecha
    for (int j = -k; j <= k; ++j)
      c4.pset(x + 5 + j, up ? y + 3 + k : y + 7 - k, fg);
  c4.text(num, x + 10, y + 2, fg, 1, true);
}

// Placa de un objeto en la pantalla de botín. La placa es el propio botón:
// la elegida se enmarca en oro y la otra se atenúa.
void lootColumn(const Item& it, const Item* other, int x0, int x1,
                const Stat* list, int n, bool selected, bool isNew) {
  const int y0 = 3, y1 = 131;
  panel(x0, y0, x1, y1);
  int cx = (x0 + x1) / 2;
  char buf[40];
  if (it.present) {
    uint8_t rc = rarityColor(it.rareza);
    txt(B12, slotName(it.slot), cx, 22, rc, middle_center, NONE);
    txt(I9, rarezaName(it.rareza), cx, 40, rc, middle_center, NONE);
  } else {
    txt(B12, "Vacio", cx, 22, pal::MEDIO, middle_center, NONE);
    txt(I9, "sin objeto", cx, 40, pal::MEDIO, middle_center, NONE);
  }
  for (int x = x0 + 6; x < x1 - 5; ++x)
    if (x % 2) c4.pset(x, 50, pal::MEDIO);

  int step = n > 5 ? 12 : 13;
  for (int i = 0; i < n; ++i) {
    Stat s = list[i];
    int y = 62 + i * step;
    int v = it.present ? it.get(s) : 0;
    int vx = isNew ? x1 - 33 : x1 - 8;
    txt(S9, statName(s), x0 + 8, y, pal::CLARO, middle_left, NONE);
    if (v) {
      std::snprintf(buf, sizeof(buf), "%+d%s", v, statIsPercent(s) ? "%" : "");
      txt(B9, buf, vx, y, pal::BRILLO, middle_right, NONE);
    } else {
      txt(S9, "--", vx, y, pal::MEDIO, middle_right, NONE);
    }
    if (isNew) {
      int a = other && other->present ? other->get(s) : 0;
      int d = v - a;
      if (d) badge(x1 - 29, y - 5, d);
    }
  }

  if (selected) {
    c4.frame(x0, y0, x1 - x0 + 1, y1 - y0 + 1, pal::ORO);
    c4.frame(x0 + 2, y0 + 2, x1 - x0 - 3, y1 - y0 - 3, pal::ORO);
    // Marcas en las esquinas que laten suavemente.
    if ((gt / 400) % 2 == 0)
      for (int k = 0; k < 6; ++k) {
        c4.pset(x0 + 3 + k, y0 + 3, pal::BRILLO);
        c4.pset(x0 + 3, y0 + 3 + k, pal::BRILLO);
        c4.pset(x1 - 3 - k, y1 - 3, pal::BRILLO);
        c4.pset(x1 - 3, y1 - 3 - k, pal::BRILLO);
      }
  } else {
    c4.shade(x0, y0, x1 - x0 + 1, y1 - y0 + 1, pal::NEGRO, 7);
  }
}

void drawLootItem() {
  drawSceneNoEnemy();
  c4.shade(0, 0, W, H, pal::NEGRO, 11);
  const Item& cur = hero.equip[static_cast<int>(loot.item.slot)];
  const Item& nue = loot.item;

  // Filas comunes a ambas placas para que queden alineadas.
  Stat list[2 * MAX_MODS];
  int n = 0;
  auto add = [&](Stat s) {
    for (int i = 0; i < n; ++i) if (list[i] == s) return;
    if (n < 2 * MAX_MODS) list[n++] = s;
  };
  for (int i = 0; i < nue.nMods; ++i) add(nue.mods[i].stat);
  for (int i = 0; cur.present && i < cur.nMods; ++i) add(cur.mods[i].stat);

  lootColumn(cur, nullptr, 3, 117, list, n, sel == 0, false);
  lootColumn(nue, &cur, 122, 236, list, n, sel == 1, true);
}

void drawLootPotion() {
  drawSceneNoEnemy();
  c4.shade(0, 0, W, H, pal::NEGRO, 11);
  char buf[28];
  panel(2, 2, 237, 132);
  txt(B12, "Pocion", W / 2, 16, pal::ORO, middle_center, NONE);
  std::snprintf(buf, sizeof(buf), "Vida %d/%d", hero.hp, hero.hpMax);
  txt(B9, buf, W / 2, 36, pal::SANGRE, middle_center, NONE);
  int g = potionMaxGain(hero);
  int lx = W / 4 + 4, rx = W * 3 / 4 - 4;
  txt(S9, "Restaura toda", lx, 56, pal::CLARO, middle_center, NONE);
  txt(S9, "la vida", lx, 70, pal::CLARO, middle_center, NONE);
  std::snprintf(buf, sizeof(buf), "%d/%d", hero.hpMax, hero.hpMax);
  txt(B9, buf, lx, 88, pal::ESPECTRAL, middle_center, NONE);
  std::snprintf(buf, sizeof(buf), "+%d vida max.", g);
  txt(S9, buf, rx, 56, pal::CLARO, middle_center, NONE);
  txt(S9, "permanente", rx, 70, pal::CLARO, middle_center, NONE);
  std::snprintf(buf, sizeof(buf), "%d/%d", hero.hp + g, hero.hpMax + g);
  txt(B9, buf, rx, 88, pal::ESPECTRAL, middle_center, NONE);
  for (int y = 50; y < 96; ++y)
    if (y % 2) c4.pset(W / 2, y, pal::MEDIO);
  button("Recuperar", lx, 109, sel == 0);
  button("Vida max.", rx, 109, sel == 1);
}

// ---- Pantalla de título: logo que cae, reflejo, destellos y ascuas.
struct Ember { int16_t x; uint16_t phase; uint8_t ms; uint8_t c; };
Ember embers[22];
bool titleLanded = false;

void initTitle() {
  titleLanded = false;
  for (Ember& e : embers) {
    e.x = static_cast<int16_t>(rng.range(0, W - 1));
    e.phase = static_cast<uint16_t>(rng.range(0, 4000));
    e.ms = static_cast<uint8_t>(rng.range(18, 40));
    e.c = rng.chance(60) ? pal::ORO : (rng.chance(50) ? pal::SANGRE : pal::BRILLO);
  }
}

float easeOutBack(float u) {
  const float c1 = 1.70158f, c3 = c1 + 1;
  float t = u - 1;
  return 1 + c3 * t * t * t + c1 * t * t;
}

bool logoPixel(int sx, int sy) {
  const gfx::Sheet& s = gfx::titulo::sheet;
  if (sx < 0 || sy < 0 || sx >= s.w || sy >= s.h) return false;
  uint8_t b = s.cel(0, 0)[sy * (s.w / 2) + (sx >> 1)];
  uint8_t c = (sx & 1) ? (b & 0x0F) : (b >> 4);
  return c != s.key && c != pal::NEGRO;
}

void sparkle(int x, int y, uint32_t phase) {
  uint32_t t = (gt + phase) % 1400;
  int r = t < 350 ? static_cast<int>(t / 117) : (t < 700 ? static_cast<int>((700 - t) / 117) : -1);
  if (r < 0) return;
  c4.pset(x, y, pal::BRILLO);
  for (int k = 1; k <= r; ++k) {
    uint8_t c = k == r ? pal::ORO : pal::BRILLO;
    c4.pset(x + k, y, c);
    c4.pset(x - k, y, c);
    c4.pset(x, y + k, c);
    c4.pset(x, y - k, c);
  }
}

void drawTitle() {
  drawBackground(0, 6);
  // Viñeteado: bordes más oscuros.
  c4.shade(0, 0, 18, H, pal::NEGRO, 8);
  c4.shade(W - 18, 0, 18, H, pal::NEGRO, 8);
  c4.shade(18, 0, W - 36, 8, pal::NEGRO, 6);
  c4.shade(0, 96, W, H - 96, pal::NEGRO, 6);

  // Ascuas que suben desde el suelo.
  for (const Ember& e : embers) {
    uint32_t t = gt + e.phase;
    int y = H - static_cast<int>((t / e.ms) % (H + 10));
    int sway = static_cast<int>((t / 260) % 4);
    int x = e.x + (sway == 1 ? 1 : (sway == 3 ? -1 : 0));
    c4.pset(x, y, e.c);
    if (e.c == pal::ORO && y + 1 < H) c4.pset(x, y + 1, pal::SANGRE);
  }

  // Logo: cae con rebote y al aterrizar tiembla y destella.
  const gfx::Sheet& logo = gfx::titulo::sheet;
  int lx = (W - logo.w) / 2, ly = 8;
  float u = clamp01(since(stT) / 750.0f);
  int y = static_cast<int>(lerp(-logo.h - 4, ly, easeOutBack(u)));
  if (u >= 1 && !titleLanded) {
    titleLanded = true;
    shake(3, 320);
    flashAmt = 0.7f;
  }
  c4.blit(logo, 0, 0, lx, y, Remap::identity());

  if (titleLanded) {
    // Reflejo diagonal que recorre las letras cada 2,8 s.
    uint32_t t = (since(stT) + 2000) % 2800;
    if (t < 650) {
      int band = static_cast<int>(t * (logo.w + logo.h) / 650) - logo.h;
      for (int sy = 0; sy < logo.h; ++sy)
        for (int k = 0; k < 6; ++k) {
          int sx = band + k + sy / 2;
          if (logoPixel(sx, sy)) c4.pset(lx + sx, ly + sy, pal::BRILLO);
        }
    }
    // Destellos alrededor del logo.
    sparkle(lx + 8, ly + 6, 0);
    sparkle(lx + logo.w - 10, ly + 2, 500);
    sparkle(lx + logo.w - 2, ly + 44, 900);
    sparkle(lx + 2, ly + 52, 300);
    sparkle(lx + logo.w / 2 + 30, ly + 30, 1100);

    if (since(stT) > 1100) {
      txt(I9, "Una leyenda oscura", W / 2, 90, pal::CLARO);
      bool on = (gt / 450) % 2 == 0;
      txt(B9, "- Pulsa A -", W / 2, 110, on ? pal::ORO : pal::BRILLO);
    }
  }
  char buf[24];
  std::snprintf(buf, sizeof(buf), "RECORD %u", static_cast<unsigned>(record));
  c4.label(buf, 6, 126, pal::MEDIO, pal::NEGRO, 0);
  c4.label(audio::enabled() ? "B: SONIDO SI" : "B: SONIDO NO", 234, 126,
           pal::MEDIO, pal::NEGRO, 2);
}

void drawBiomeIntro() {
  drawSceneNoEnemy();
  float u = clamp01(since(stT) / 400.0f);
  c4.shade(0, 30, W, 50, pal::NEGRO, static_cast<int>(12 * u));
  if (u >= 1) {
    char num[16], cap[28];
    roman((nivel - 1) / bal::JEFE_CADA + 1, num);
    std::snprintf(cap, sizeof(cap), "Capitulo %s", num);
    txt(I9, cap, W / 2, 41, pal::MEDIO);
    txt(B12, biomaName(biomaDe(nivel)), W / 2, 62, pal::ESPECTRAL);
  }
}

void drawBossBanner() {
  float u = clamp01(since(stT) / 300.0f);
  int hh = static_cast<int>(20 * u);
  c4.shade(0, 52 - hh, W, hh * 2, pal::NEGRO, 12);
  for (int x = 0; x < W; ++x) {
    c4.pset(x, 52 - hh, pal::SANGRE);
    c4.pset(x, 51 + hh, pal::SANGRE);
  }
  if (u >= 1) {
    txt(I9, "- Jefe -", W / 2, 40, pal::SANGRE);
    txt(B12, enemyName(enemy.type), W / 2, 59, pal::ORO);
  }
}

void drawGameOver() {
  drawSceneNoEnemy();
  float u = clamp01(since(stT) / 600.0f);
  c4.shade(0, 0, W, H, pal::NEGRO, static_cast<int>(12 * u));
  if (u < 1) return;
  txt(B18, "Has caido", W / 2, 26, pal::SANGRE);
  char buf[32];
  std::snprintf(buf, sizeof(buf), "Enemigos derrotados: %d", derrotados);
  txt(S9, buf, W / 2, 52, pal::BRILLO);
  std::snprintf(buf, sizeof(buf), "Golpe maximo: %d", golpeMax);
  txt(S9, buf, W / 2, 67, pal::CLARO);
  std::snprintf(buf, sizeof(buf), "Record: %u", static_cast<unsigned>(record));
  txt(S9, buf, W / 2, 82, pal::ESPECTRAL);
  if (nuevoRecord && (gt / 300) % 2 == 0)
    txt(B9, "Nuevo record!", W / 2, 98, pal::ORO);
  txt(S9, "A: volver", W / 2, 120, pal::MEDIO);
}

void drawCombat(St view) {
  drawBackground(0, gfx::BG_LAYER_PERSONAJES);
  bool gone = view == St::VICTORY;
  if (!gone) {
    int dissolve = 0;
    if (view == St::ENEMY_DIE)
      dissolve = static_cast<int>(17 * since(stT) / T_MUERTE);
    if (dissolve <= 16) drawEnemy(dissolve);
  }
  drawHero();
  drawBackground(gfx::BG_LAYER_PERSONAJES, 6);
  drawHud();
  if (view == St::SHOW && msg[0])
{
    cv.setFont(B12);
    txt(cv.textWidth(msg) <= W - 12 ? B12 : B9, msg, W / 2, 50, msgColor);
  }
  if (view == St::HERO_TURN) {
    if (charging) drawChargeBar();
    if (statsOpen) drawStatsPanel();
  }
  if (view == St::BOSS_INTRO) drawBossBanner();
  drawFloaters();
}

void render() {
  applyPalette();
  St view = st;
  if (st == St::FADE) {
    // Durante el fundido se ve la escena de origen y después la de destino.
    if (!fadeDone)
      view = fadeAct == FadeAct::NEW_GAME   ? St::TITLE
             : fadeAct == FadeAct::TO_TITLE ? St::GAME_OVER
                                            : St::VICTORY;
    else
      view = fadeNext == St::ENEMY_ENTER ? St::VICTORY : fadeNext;
  }
  switch (view) {
    case St::TITLE: drawTitle(); break;
    case St::BIOME_INTRO: drawBiomeIntro(); break;
    case St::LOOT_ITEM: drawLootItem(); break;
    case St::LOOT_POTION: drawLootPotion(); break;
    case St::GAME_OVER: drawGameOver(); break;
    default: drawCombat(view); break;
  }
  if (paused) {
    panel(70, 44, 169, 90);
    txt(B12, "Pausa", W / 2, 60, pal::ORO, middle_center, NONE);
    txt(S9, "PWR: seguir", W / 2, 78, pal::MEDIO, middle_center, NONE);
  }

  // Temblor de pantalla.
  int sx = 0, sy = 0;
  if (since(shakeT) < shakeDur && !paused) {
    float k = 1.0f - since(shakeT) / static_cast<float>(shakeDur);
    int a = static_cast<int>(shakeAmp * k + 0.5f);
    if (a > 0) {
      sx = rng.range(-a, a);
      sy = rng.range(-a, a);
    }
  }
  static bool wasShaking = false;
  if (sx || sy || wasShaking) M5.Display.fillScreen(TFT_BLACK);
  wasShaking = sx || sy;
  cv.pushSprite(sx, sy);
}

void handlePower() {
  if (M5.BtnPWR.pressedFor(T_APAGAR_MS)) {
    M5.Display.setBrightness(0);
    M5.Power.powerOff();
  }
  if (M5.BtnPWR.wasClicked()) {
    paused = !paused;
    pausedAtMs = millis();
    aArmed = charging = false;
    M5.Display.setBrightness(BRILLO_PANTALLA);
    audio::sfx(paused ? 520 : 780, 40, true);
  }
  if (paused && millis() - pausedAtMs > T_ATENUAR_MS)
    M5.Display.setBrightness(BRILLO_PAUSA);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(BRILLO_PANTALLA);
  M5.Display.fillScreen(TFT_BLACK);

  cv.setColorDepth(4);
  cv.createSprite(W, H);
  // Orden de los nibbles en el búfer de 4 bpp.
  cv.fillScreen(0);
  cv.drawPixel(0, 0, 1);
  uint8_t* buf = static_cast<uint8_t*>(cv.getBuffer());
  c4.attach(buf, W, H, buf[0] == 0x10);
  for (int i = 8; i < 16; ++i) cv.setPaletteColor(i, 0, 0, 0);

  prefs.begin("stickrpg", false);
  record = prefs.getUInt("record", 0);
  audio::begin(prefs.getBool("sound", true));

  rng.reseed((static_cast<uint64_t>(esp_random()) << 32) | esp_random());
  hero = makeHero();
  clouds::generate(esp_random());
  resetTimers();
  setState(St::TITLE);
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  uint32_t dt = now - last;
  last = now;

  M5.update();
  handlePower();
  audio::update();
  if (!paused) {
    gt += dt;
    update();
  }
  render();

  uint32_t spent = millis() - now;
  if (spent < FRAME_MS) delay(FRAME_MS - spent);
}
