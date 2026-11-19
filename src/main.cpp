// Stick RPG — firmware para M5StickC Plus2.
// Estados, entrada, render y sonido. Las reglas del juego viven en src/core.
// Los gráficos son provisionales (primitivas) hasta tener los sprites.

#include <M5Unified.h>
#include <Preferences.h>

#include <cstdio>
#include <cstring>

#include "core/balance.h"
#include "core/combat.h"
#include "core/progression.h"
#include "gfx/heroe_gfx.h"
#include "palette.h"

namespace {

constexpr int W = 240;
constexpr int H = 135;
constexpr int SUELO_Y = 108;
constexpr int HEROE_X = 42;
constexpr int ENEMIGO_X = 182;
constexpr uint32_t FRAME_MS = 33;
constexpr uint32_t T_APAGAR_MS = 2000;  // mantener PWR para apagar
constexpr uint32_t T_ATENUAR_MS = 10000;
constexpr uint8_t BRILLO = 110;
constexpr uint8_t BRILLO_PAUSA = 12;

M5Canvas cv(&M5.Display);
Preferences prefs;
Rng rng;
Hero hero;
Enemy enemy;
Battle battle(hero, enemy, rng);

enum class St : uint8_t {
  START, RUN, HERO_TURN, SHOW, ENEMY_WAIT, VICTORY, LOOT_ITEM, LOOT_POTION,
  GAME_OVER
};

St st = St::START;
uint32_t gt = 0;   // tiempo de juego (se detiene en pausa)
uint32_t stT = 0;  // inicio del estado actual

// Mensaje central y número flotante.
struct Msg { char text[28]; uint8_t color; } msg = {"", pal::BLANCO};
St showNext = St::HERO_TURN;
uint32_t showDur = 0;

struct Floater {
  bool on = false;
  char text[12] = "";
  uint8_t color = pal::BLANCO;
  int x = 0, y = 0;
  uint32_t t0 = 0;
} floater;

// Partida.
int nivel = 1, derrotados = 0, golpeMax = 0;
uint32_t record = 0;
bool nuevoRecord = false;
bool soundOn = true;

// Estado de UI.
bool statsOpen = false;
bool paused = false;
uint32_t pausedAtMs = 0;
bool aArmed = false, charging = false, inPerfect = false;
uint32_t aPressMs = 0;
float chargeF = 0;
uint32_t heroAtkT = 0, enemyAtkT = 0, enemyHitT = 0, heroHitT = 0;
Loot loot;
int sel = 0;

// Transición.
float scrollX = 0, scrollBase = 0;
int heroX = 20, enemyX = 300;

// ---------------------------------------------------------------- utilidades

void beep(float freq, uint32_t ms) {
  if (soundOn) M5.Speaker.tone(freq, ms);
}

void setState(St s);

void show(const char* text, uint8_t color, uint32_t dur, St next) {
  std::snprintf(msg.text, sizeof(msg.text), "%s", text);
  msg.color = color;
  showDur = dur;
  showNext = next;
  setState(St::SHOW);
}

void floatText(const char* text, uint8_t color, int x, int y) {
  floater.on = true;
  std::snprintf(floater.text, sizeof(floater.text), "%s", text);
  floater.color = color;
  floater.x = x;
  floater.y = y;
  floater.t0 = gt;
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

uint8_t rarezaColor(Rareza r) {
  switch (r) {
    case Rareza::RARA: return pal::AZUL;
    case Rareza::EPICA: return pal::MORADO;
    default: return pal::GRIS_CLARO;
  }
}

void text(const char* s, int x, int y, uint8_t color, int size = 1,
          uint8_t datum = top_left) {
  cv.setTextSize(size);
  cv.setTextDatum(datum);
  cv.setTextColor(color);
  cv.drawString(s, x, y);
}

void textShadow(const char* s, int x, int y, uint8_t color, int size,
                uint8_t datum) {
  text(s, x + 1, y + 1, pal::NEGRO, size, datum);
  text(s, x, y, color, size, datum);
}

// ------------------------------------------------------------------- partida

void startRun() {
  enemy = makeEnemy(rng, nivel);
  battle.reset();
  scrollBase = scrollX;
  heroX = 20;
  enemyX = 300;
  setState(St::RUN);
}

void newGame() {
  hero = makeHero();
  nivel = 1;
  derrotados = 0;
  golpeMax = 0;
  nuevoRecord = false;
  startRun();
}

void onEnter(St s) {
  switch (s) {
    case St::HERO_TURN:
      aArmed = charging = inPerfect = false;
      statsOpen = false;
      break;
    case St::VICTORY: {
      ++derrotados;
      int healed = healVictory(hero);
      char buf[12];
      std::snprintf(buf, sizeof(buf), "+%d HP", healed);
      if (healed > 0) floatText(buf, pal::VERDE, heroX, SUELO_Y - 50);
      beep(880, 80);
      break;
    }
    case St::GAME_OVER:
      if (static_cast<uint32_t>(derrotados) > record) {
        record = derrotados;
        nuevoRecord = true;
        prefs.putUInt("record", record);
      }
      beep(196, 400);
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

void doHeroAttack(float mult, bool perfect) {
  HeroTurnResult r = battle.heroAttack(mult);
  heroAtkT = gt;
  if (r.atk.damage > golpeMax) golpeMax = r.atk.damage;

  char buf[28] = "";
  uint8_t color = pal::BLANCO;
  uint32_t dur = bal::T_TEXTO_GOLPE;
  if (!r.atk.hit) {
    floatText("FALLO", pal::GRIS_CLARO, enemyX, SUELO_Y - 50);
    beep(220, 60);
  } else {
    char d[12];
    std::snprintf(d, sizeof(d), "-%d", r.atk.damage);
    floatText(d, r.atk.crit ? pal::AMARILLO : pal::BLANCO, enemyX,
              SUELO_Y - 50);
    enemyHitT = gt;
    if (perfect) std::strcat(buf, "PERFECTO ");
    if (r.atk.crit) {
      std::strcat(buf, "CRITICO ");
      dur = bal::T_TEXTO_CRITICO;
    }
    if (buf[0]) color = pal::AMARILLO;
    beep(r.atk.crit ? 1320 : 660, r.atk.crit ? 90 : 50);
  }
  if (r.enemyDead) {
    show(buf, color, dur, St::VICTORY);
  } else if (r.extraTurn) {
    std::strcat(buf, "+TURNO");
    // Dentro de una cadena los textos duran menos (§8.4).
    uint32_t d = battle.chain() > 1 ? bal::T_TEXTO_CADENA : bal::T_TEXTO_EXTRA;
    show(buf, buf[0] == '+' ? pal::AZUL : pal::AMARILLO, d > dur ? d : dur,
         St::HERO_TURN);
    beep(990, 60);
  } else {
    show(buf, color, dur, St::ENEMY_WAIT);
  }
}

void doEnemyTurn() {
  EnemyTurnResult r = battle.enemyTurn();
  if (r.action == EnemyAction::PREPARAR) {
    beep(440, 120);
    show("SE PREPARA!", pal::NARANJA, 650, St::HERO_TURN);
    return;
  }
  enemyAtkT = gt;
  const char* label = r.action == EnemyAction::GOLPE_FUERTE ? "GOLPE FUERTE" : "";
  if (!r.atk.hit) {
    floatText("FALLA", pal::GRIS_CLARO, heroX, SUELO_Y - 50);
    beep(220, 60);
  } else {
    char d[12];
    std::snprintf(d, sizeof(d), "-%d", r.atk.damage);
    floatText(d, pal::ROJO, heroX, SUELO_Y - 50);
    heroHitT = gt;
    if (r.atk.crit) label = r.action == EnemyAction::GOLPE_FUERTE
                                ? "GOLPE FUERTE CRIT" : "CRITICO";
    beep(r.atk.crit ? 150 : 180, 90);
  }
  uint8_t color = r.action == EnemyAction::GOLPE_FUERTE ? pal::NARANJA : pal::ROJO;
  uint32_t dur = label[0] ? bal::T_TEXTO_CRITICO : bal::T_TEXTO_GOLPE;
  show(label, color, dur, r.heroDead ? St::GAME_OVER : St::HERO_TURN);
}

// ------------------------------------------------------------------- update

void updateStart() {
  if (M5.BtnB.wasPressed()) {
    soundOn = !soundOn;
    prefs.putBool("sound", soundOn);
    beep(880, 60);
  }
  if (M5.BtnA.wasPressed()) {
    beep(660, 60);
    newGame();
  }
  scrollX += 0.5f;
}

void updateRun() {
  uint32_t t = gt - stT;
  if (t < bal::T_CARRERA) {
    heroX = static_cast<int>(lerp(20, 190, t / static_cast<float>(bal::T_CARRERA)));
  } else if (t < bal::T_CARRERA + bal::T_SCROLL) {
    float u = (t - bal::T_CARRERA) / static_cast<float>(bal::T_SCROLL);
    heroX = static_cast<int>(lerp(190, HEROE_X, u));
    enemyX = static_cast<int>(lerp(280, ENEMIGO_X, u));
    scrollX = scrollBase + u * 160;
  } else {
    heroX = HEROE_X;
    enemyX = ENEMIGO_X;
    setState(St::HERO_TURN);
  }
}

void updateHeroTurn() {
  if (!charging && M5.BtnB.wasPressed()) {
    statsOpen = !statsOpen;
    beep(770, 30);
    return;
  }
  if (statsOpen) return;

  if (M5.BtnA.wasPressed()) {
    aArmed = true;
    aPressMs = millis();
    charging = inPerfect = false;
  }
  if (!aArmed) return;
  uint32_t held = millis() - aPressMs;
  if (M5.BtnA.isPressed()) {
    if (held > bal::T_UMBRAL_MS) {
      charging = true;
      chargeF = chargeFill(held);
      bool perf = chargeIsPerfect(chargeF);
      if (perf && !inPerfect) beep(1760, 40);
      inPerfect = perf;
    }
  } else if (M5.BtnA.wasReleased() || !M5.BtnA.isPressed()) {
    bool charged = held > bal::T_UMBRAL_MS;
    float f = chargeFill(held);
    aArmed = charging = false;
    doHeroAttack(charged ? chargeMultiplier(f) : 1.0f,
                 charged && chargeIsPerfect(f));
  }
}

void updateLoot(bool potion) {
  if (gt - stT < 250) return;  // evita confirmar sin querer
  if (M5.BtnB.wasPressed()) {
    sel ^= 1;
    beep(770, 30);
  }
  if (M5.BtnA.wasPressed()) {
    beep(990, 60);
    if (potion) {
      if (sel == 0) potionRecover(hero);
      else potionMaxHp(hero);
    } else if (sel == 1) {
      equipItem(hero, loot.item);
    }
    ++nivel;
    startRun();
  }
}

void update() {
  floater.on = floater.on && gt - floater.t0 < 700;
  switch (st) {
    case St::START: updateStart(); break;
    case St::RUN: updateRun(); break;
    case St::HERO_TURN: updateHeroTurn(); break;
    case St::SHOW:
      if (gt - stT >= showDur) setState(showNext);
      break;
    case St::ENEMY_WAIT:
      if (gt - stT >= bal::T_TURNO_ENEMIGO) doEnemyTurn();
      break;
    case St::VICTORY:
      if (gt - stT >= 700) {
        loot = rollLoot(rng, nivel, hero.eff(Stat::LUCK), isBossLevel(nivel));
        sel = 0;
        setState(loot.pocion ? St::LOOT_POTION : St::LOOT_ITEM);
      }
      break;
    case St::LOOT_ITEM: updateLoot(false); break;
    case St::LOOT_POTION: updateLoot(true); break;
    case St::GAME_OVER:
      if (gt - stT > 600 && M5.BtnA.wasPressed()) {
        beep(660, 60);
        setState(St::START);
      }
      break;
  }
}

// ------------------------------------------------------------------- render

void drawBackground() {
  cv.fillScreen(pal::NOCHE);
  // Montañas lejanas (parallax lento).
  int off = static_cast<int>(scrollX * 0.3f) % 80;
  for (int x = -off - 80; x < W + 80; x += 80) {
    cv.fillTriangle(x, SUELO_Y, x + 40, 52, x + 80, SUELO_Y, pal::PIZARRA);
  }
  // Suelo.
  cv.fillRect(0, SUELO_Y, W, H - SUELO_Y, pal::MARRON_OSC);
  cv.fillRect(0, SUELO_Y, W, 3, pal::VERDE_OSC);
  int g = static_cast<int>(scrollX) % 24;
  for (int x = -g; x < W; x += 24) {
    cv.fillRect(x, SUELO_Y + 9, 8, 2, pal::MARRON);
    cv.fillRect(x + 12, SUELO_Y + 18, 6, 2, pal::MARRON);
  }
}

// Recoloreado del equipo por rareza: el sprite usa GRIS_CLARO / GRIS.
gfx::Remap remapRareza(const Item& it) {
  gfx::Remap r = gfx::Remap::identity();
  if (!it.present) return r;
  if (it.rareza == Rareza::RARA) {
    r.to[pal::GRIS_CLARO] = pal::AZUL;
    r.to[pal::GRIS] = pal::PIZARRA;
  } else if (it.rareza == Rareza::EPICA) {
    r.to[pal::GRIS_CLARO] = pal::MORADO;
    r.to[pal::GRIS] = pal::PIZARRA;
  }
  return r;
}

void drawHero(int x, bool running) {
  namespace H = gfx::heroe;
  // Punto de apoyo: pies en (13, 35) dentro del frame.
  int x0 = x - 13, y0 = SUELO_Y - 35;
  int frame;
  bool flash = false;
  if (gt - heroHitT < 250) {
    frame = H::HIT.from;
    x0 -= 2;
    flash = (gt / 50) % 2 == 0;
  } else if (gt - heroAtkT < H::ATTACK.duration()) {
    frame = H::ATTACK.frameAt(gt - heroAtkT, false);
  } else if (running) {
    frame = H::RUN.frameAt(gt);
  } else {
    frame = H::IDLE.frameAt(gt);
  }

  gfx::Remap blanco = gfx::Remap::identity();
  for (auto& c : blanco.to) c = pal::BLANCO;

  struct { int layer; int slot; } capas[] = {
      {H::ESCUDO, (int)Slot::ESCUDO}, {H::CUERPO, -1},
      {H::BOTAS, (int)Slot::BOTAS},   {H::PECHERA, (int)Slot::PECHERA},
      {H::CASCO, (int)Slot::CASCO},   {H::BRAZO, (int)Slot::ESPADA},
  };
  for (auto& c : capas) {
    gfx::Remap r = gfx::Remap::identity();
    if (c.slot >= 0) {
      const Item& it = hero.equip[c.slot];
      // El brazo se dibuja siempre; el resto solo si la pieza está equipada.
      if (!it.present && c.layer != H::BRAZO) continue;
      r = remapRareza(it);
    }
    gfx::drawCel(cv, H::sheet, c.layer, frame, x0, y0, flash ? blanco : r);
  }
}

void drawEnemy(int x) {
  int y = SUELO_Y;
  if (gt - enemyAtkT < 180) x -= 10;
  if (gt - enemyHitT < 250) x += ((gt / 40) % 2) ? 2 : -2;
  switch (enemy.type) {
    case EnemyType::BANDIDO:
      cv.drawLine(x, y - 13, x + 3, y - 1, pal::PIEL);
      cv.drawLine(x - 1, y - 13, x - 3, y - 1, pal::PIEL);
      cv.fillRect(x - 3, y - 25, 7, 13, pal::ROJO);
      cv.fillCircle(x, y - 30, 4, pal::PIEL);
      cv.fillRect(x - 4, y - 33, 9, 2, pal::ROJO);
      cv.drawLine(x - 4, y - 20, x - 11, y - 24, pal::GRIS_CLARO);
      break;
    case EnemyType::GOLEM:
      cv.fillRoundRect(x - 15, y - 36, 30, 36, 5, pal::GRIS);
      cv.fillRect(x - 9, y - 30, 4, 3, pal::AMARILLO);
      cv.fillRect(x + 1, y - 30, 4, 3, pal::AMARILLO);
      cv.fillRect(x - 15, y - 14, 30, 2, pal::PIZARRA);
      break;
    case EnemyType::LOBO:
      cv.fillEllipse(x + 2, y - 11, 14, 6, pal::MARRON);
      cv.fillCircle(x - 13, y - 16, 5, pal::MARRON);
      cv.fillTriangle(x - 15, y - 25, x - 12, y - 19, x - 9, y - 22, pal::MARRON);
      cv.fillRect(x - 21, y - 16, 5, 3, pal::MARRON);
      cv.fillRect(x - 14, y - 18, 2, 2, pal::AMARILLO);
      for (int i = 0; i < 4; ++i) cv.fillRect(x - 9 + i * 7, y - 6, 2, 6, pal::MARRON);
      break;
    case EnemyType::OGRO:
      cv.fillRoundRect(x - 13, y - 38, 26, 28, 6, pal::VERDE_OSC);
      cv.fillCircle(x, y - 42, 7, pal::VERDE_OSC);
      cv.fillRect(x - 4, y - 44, 2, 2, pal::AMARILLO);
      cv.fillRect(x - 10, y - 10, 6, 10, pal::VERDE_OSC);
      cv.fillRect(x + 4, y - 10, 6, 10, pal::VERDE_OSC);
      cv.fillRoundRect(x - 26, y - 40, 6, 20, 3, pal::MARRON);
      break;
    case EnemyType::JEFE:
    default:
      cv.fillRoundRect(x - 14, y - 44, 28, 32, 4, pal::MORADO);
      cv.fillRect(x - 10, y - 12, 7, 12, pal::MORADO);
      cv.fillRect(x + 3, y - 12, 7, 12, pal::MORADO);
      cv.fillCircle(x, y - 50, 7, pal::GRIS);
      cv.fillRect(x - 5, y - 52, 3, 2, pal::ROJO);
      cv.fillTriangle(x - 7, y - 57, x - 4, y - 63, x - 1, y - 57, pal::AMARILLO);
      cv.fillTriangle(x + 1, y - 57, x + 4, y - 63, x + 7, y - 57, pal::AMARILLO);
      cv.drawLine(x - 16, y - 30, x - 28, y - 58, pal::BLANCO);
      break;
  }
  // Aviso de golpe fuerte (§4.5).
  if (enemy.preparado && (gt / 200) % 2 == 0)
    textShadow("!", x, y - 66, pal::NARANJA, 2, middle_center);
}

void drawBar(int x, int y, int w, int h, int v, int max, uint8_t color) {
  cv.fillRect(x, y, w, h, pal::NEGRO);
  int f = max > 0 ? (w - 2) * v / max : 0;
  if (f > 0) cv.fillRect(x + 1, y + 1, f, h - 2, color);
  cv.drawRect(x, y, w, h, pal::GRIS_CLARO);
}

void drawHud(bool withEnemy) {
  char buf[24];
  drawBar(4, 4, 96, 7, hero.hp, hero.hpMax, pal::VERDE);
  std::snprintf(buf, sizeof(buf), "HP %d/%d", hero.hp, hero.hpMax);
  text(buf, 4, 13, pal::BLANCO);
  std::snprintf(buf, sizeof(buf), "#%d", nivel);
  text(buf, W / 2, 4, isBossLevel(nivel) ? pal::MORADO : pal::GRIS_CLARO, 1,
       top_center);
  if (withEnemy) {
    drawBar(W - 100, 4, 96, 7, enemy.hp, enemy.hpMax, pal::ROJO);
    std::snprintf(buf, sizeof(buf), "%s %d", enemyName(enemy.type), enemy.hp);
    text(buf, W - 4, 13, pal::BLANCO, 1, top_right);
  }
}

void drawChargeBar() {
  int x = 50, y = 122, w = 140, h = 9;
  cv.fillRect(x, y, w, h, pal::NEGRO);
  int zx = x + 1 + static_cast<int>((w - 2) * bal::ZONA_PERFECTA);
  cv.fillRect(zx, y + 1, x + w - 1 - zx, h - 2, pal::MARRON);
  int f = static_cast<int>((w - 2) * chargeF);
  cv.fillRect(x + 1, y + 1, f, h - 2, inPerfect ? pal::AMARILLO : pal::AZUL);
  cv.drawRect(x, y, w, h, pal::BLANCO);
  cv.drawFastVLine(zx, y - 2, h + 4, pal::AMARILLO);
}

void drawStatsPanel() {
  cv.fillRect(14, 22, 212, 104, pal::NEGRO);
  cv.drawRect(14, 22, 212, 104, pal::BLANCO);
  text("ESTADISTICAS", 20, 26, pal::AMARILLO);
  text("B: cerrar", 220, 26, pal::GRIS_CLARO, 1, top_right);
  char buf[28];
  int y = 40;
  std::snprintf(buf, sizeof(buf), "HP  %d/%d", hero.hp, hero.hpMax);
  text(buf, 20, y, pal::VERDE);
  for (int i = 0; i < STAT_COUNT; ++i) {
    Stat s = static_cast<Stat>(i);
    y += 11;
    bool capped = hero.raw(s) > hero.eff(s);
    std::snprintf(buf, sizeof(buf), "%-5s %d%s%s", statName(s), hero.eff(s),
                  statIsPercent(s) ? "%" : "", capped ? " tope" : "");
    text(buf, 20, y, capped ? pal::NARANJA : pal::BLANCO);
  }
  for (int i = 0; i < SLOT_COUNT; ++i) {
    itemName(hero.equip[i], buf, sizeof(buf));
    text(buf, 106, 40 + i * 13, hero.equip[i].present
                                    ? rarezaColor(hero.equip[i].rareza)
                                    : pal::GRIS);
  }
}

void fmtStat(char* buf, int len, Stat s, int v) {
  std::snprintf(buf, len, "%s %+d%s", statName(s), v, statIsPercent(s) ? "%" : "");
}

void drawOption(const char* label, int cx, int y, bool selected) {
  int w = static_cast<int>(std::strlen(label)) * 6 + 12;
  if (selected) cv.fillRect(cx - w / 2, y - 2, w, 12, pal::AMARILLO);
  else cv.drawRect(cx - w / 2, y - 2, w, 12, pal::GRIS);
  text(label, cx, y, selected ? pal::NEGRO : pal::GRIS_CLARO, 1, top_center);
}

void drawLootItem() {
  cv.fillScreen(pal::NOCHE);
  const Item& cur = hero.equip[static_cast<int>(loot.item.slot)];
  const Item& nue = loot.item;
  char buf[28];
  text("BOTIN", 4, 3, pal::AMARILLO);
  text(rarezaName(nue.rareza), W - 4, 3, rarezaColor(nue.rareza), 1, top_right);
  cv.drawFastVLine(W / 2, 16, 92, pal::PIZARRA);
  text("ACTUAL", 6, 18, pal::GRIS);
  text("NUEVO", W / 2 + 6, 18, pal::GRIS);
  itemName(cur, buf, sizeof(buf));
  text(buf, 6, 30, cur.present ? rarezaColor(cur.rareza) : pal::GRIS);
  itemName(nue, buf, sizeof(buf));
  text(buf, W / 2 + 6, 30, rarezaColor(nue.rareza));

  // Unión de estadísticas de ambos objetos (máx. 4 líneas).
  Stat list[4];
  int n = 0;
  auto add = [&](Stat s) {
    for (int i = 0; i < n; ++i) if (list[i] == s) return;
    if (n < 4) list[n++] = s;
  };
  for (int i = 0; i < cur.nMods && cur.present; ++i) add(cur.mods[i].stat);
  for (int i = 0; i < nue.nMods; ++i) add(nue.mods[i].stat);

  for (int i = 0; i < n; ++i) {
    Stat s = list[i];
    int y = 46 + i * 13;
    int cv0 = cur.present ? cur.get(s) : 0, nv = nue.get(s);
    if (cv0) { fmtStat(buf, sizeof(buf), s, cv0); text(buf, 6, y, pal::BLANCO); }
    else text("-", 6, y, pal::GRIS);
    if (nv) { fmtStat(buf, sizeof(buf), s, nv); text(buf, W / 2 + 6, y, pal::BLANCO); }
    else { std::snprintf(buf, sizeof(buf), "%s --", statName(s)); text(buf, W / 2 + 6, y, pal::GRIS); }
    int d = nv - cv0;
    int ax = W - 30;
    if (d > 0) {
      cv.fillTriangle(ax, y + 6, ax + 3, y, ax + 6, y + 6, pal::VERDE);
      std::snprintf(buf, sizeof(buf), "%d", d);
      text(buf, ax + 9, y, pal::VERDE);
    } else if (d < 0) {
      cv.fillTriangle(ax, y, ax + 3, y + 6, ax + 6, y, pal::ROJO);
      std::snprintf(buf, sizeof(buf), "%d", -d);
      text(buf, ax + 9, y, pal::ROJO);
    }
  }
  drawOption("CONSERVAR", W / 4, 112, sel == 0);
  drawOption("EQUIPAR", W * 3 / 4, 112, sel == 1);
  text("B: cambiar  A: confirmar", W / 2, 127, pal::GRIS, 1, top_center);
}

void drawLootPotion() {
  cv.fillScreen(pal::NOCHE);
  char buf[28];
  text("POCION", W / 2, 6, pal::AMARILLO, 2, top_center);
  std::snprintf(buf, sizeof(buf), "Vida %d/%d", hero.hp, hero.hpMax);
  text(buf, W / 2, 30, pal::BLANCO, 1, top_center);
  int g = potionMaxGain(hero);
  text("Restaura toda", W / 4, 52, pal::GRIS_CLARO, 1, top_center);
  text("la vida", W / 4, 62, pal::GRIS_CLARO, 1, top_center);
  std::snprintf(buf, sizeof(buf), "-> %d/%d", hero.hpMax, hero.hpMax);
  text(buf, W / 4, 78, pal::VERDE, 1, top_center);
  std::snprintf(buf, sizeof(buf), "+%d vida max.", g);
  text(buf, W * 3 / 4, 52, pal::GRIS_CLARO, 1, top_center);
  text("permanente", W * 3 / 4, 62, pal::GRIS_CLARO, 1, top_center);
  std::snprintf(buf, sizeof(buf), "-> %d/%d", hero.hp + g, hero.hpMax + g);
  text(buf, W * 3 / 4, 78, pal::VERDE, 1, top_center);
  drawOption("RECUPERAR", W / 4, 100, sel == 0);
  drawOption("VIDA MAX", W * 3 / 4, 100, sel == 1);
  text("B: cambiar  A: confirmar", W / 2, 122, pal::GRIS, 1, top_center);
}

void drawStart() {
  drawBackground();
  char buf[28];
  textShadow("STICK RPG", W / 2, 34, pal::AMARILLO, 3, middle_center);
  if ((gt / 500) % 2 == 0) textShadow("A: comenzar", W / 2, 66, pal::BLANCO, 1, middle_center);
  std::snprintf(buf, sizeof(buf), "Record: %u", static_cast<unsigned>(record));
  textShadow(buf, W / 2, 82, pal::GRIS_CLARO, 1, middle_center);
  text(soundOn ? "B: sonido ON" : "B: sonido OFF", W / 2, 124, pal::GRIS_CLARO,
       1, middle_center);
}

void drawGameOver() {
  cv.fillScreen(pal::NEGRO);
  char buf[28];
  text("GAME OVER", W / 2, 22, pal::ROJO, 3, middle_center);
  std::snprintf(buf, sizeof(buf), "Derrotados: %d", derrotados);
  text(buf, W / 2, 52, pal::BLANCO, 1, middle_center);
  std::snprintf(buf, sizeof(buf), "Record: %u", static_cast<unsigned>(record));
  text(buf, W / 2, 66, pal::GRIS_CLARO, 1, middle_center);
  if (nuevoRecord && (gt / 300) % 2 == 0)
    text("NUEVO RECORD!", W / 2, 80, pal::AMARILLO, 1, middle_center);
  std::snprintf(buf, sizeof(buf), "Golpe max.: %d", golpeMax);
  text(buf, W / 2, 94, pal::GRIS_CLARO, 1, middle_center);
  text("A: reiniciar", W / 2, 118, pal::BLANCO, 1, middle_center);
}

void render() {
  switch (st) {
    case St::START: drawStart(); break;
    case St::LOOT_ITEM: drawLootItem(); break;
    case St::LOOT_POTION: drawLootPotion(); break;
    case St::GAME_OVER: drawGameOver(); break;
    default: {
      drawBackground();
      bool fighting = st != St::RUN || gt - stT >= bal::T_CARRERA;
      if (fighting) drawEnemy(enemyX);
      drawHero(heroX, st == St::RUN);
      drawHud(fighting);
      if (st == St::SHOW && msg.text[0])
        textShadow(msg.text, W / 2, 44, msg.color, 2, middle_center);
      if (st == St::HERO_TURN) {
        if (charging) drawChargeBar();
        else text("A: atacar  B: stats", W / 2, 124, pal::GRIS_CLARO, 1, middle_center);
        if (statsOpen) drawStatsPanel();
      }
      break;
    }
  }
  if (floater.on) {
    int dy = static_cast<int>((gt - floater.t0) / 25);
    textShadow(floater.text, floater.x, floater.y - dy, floater.color, 2,
               middle_center);
  }
  if (paused) {
    cv.fillRect(60, 46, 120, 42, pal::NEGRO);
    cv.drawRect(60, 46, 120, 42, pal::BLANCO);
    text("PAUSA", W / 2, 58, pal::AMARILLO, 2, middle_center);
    text("PWR: seguir", W / 2, 78, pal::GRIS_CLARO, 1, middle_center);
  }
  cv.pushSprite(0, 0);
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
    M5.Display.setBrightness(BRILLO);
    beep(paused ? 520 : 780, 40);
  }
  if (paused && millis() - pausedAtMs > T_ATENUAR_MS)
    M5.Display.setBrightness(BRILLO_PAUSA);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(BRILLO);
  M5.Speaker.setVolume(96);

  cv.setColorDepth(4);
  cv.createSprite(W, H);
  for (int i = 0; i < 16; ++i)
    cv.setPaletteColor(i, pal::kColores[i].r, pal::kColores[i].g,
                       pal::kColores[i].b);
  cv.setFont(&fonts::Font0);

  prefs.begin("stickrpg", false);
  record = prefs.getUInt("record", 0);
  soundOn = prefs.getBool("sound", true);

  rng.reseed((static_cast<uint64_t>(esp_random()) << 32) | esp_random());
  setState(St::START);
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  uint32_t dt = now - last;
  last = now;

  M5.update();
  handlePower();
  if (!paused) {
    gt += dt;
    update();
  }
  render();

  uint32_t spent = millis() - now;
  if (spent < FRAME_MS) delay(FRAME_MS - spent);
}
