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
constexpr int ENEMY_START = 300;  // mínimo; ver enemyStart()
constexpr uint32_t FRAME_MS = 33;
constexpr uint32_t T_APAGAR_MS = 2000;
constexpr uint32_t T_ATENUAR_MS = 10000;
constexpr uint8_t BRILLO_PANTALLA = 120;
constexpr uint8_t BRILLO_PAUSA = 12;
// Inactividad: sin tocar ningún botón se pasa a reposo (pantalla apagada y
// CPU dormida; cualquier botón despierta en pausa) y, si sigue sin usarse,
// el dispositivo se apaga solo.
constexpr uint32_t T_REPOSO_MS = 2 * 60 * 1000;
constexpr uint32_t T_AUTOAPAGADO_MS = 10 * 60 * 1000;  // ya en reposo
// Batería: colores fijos (fuera de la paleta del bioma) y lectura periódica.
constexpr uint8_t COL_BAT_OK = 8;
constexpr uint8_t COL_BAT_BAJA = 9;
constexpr uint32_t T_BATERIA_MS = 3000;

constexpr uint32_t T_FADE = 320;
constexpr uint32_t T_BIOMA = 2200;
constexpr uint32_t T_ENTRADA = 1300;
constexpr uint32_t T_JEFE = 2000;
constexpr uint32_t T_GOLPE_HEROE = 180;    // hasta el frame del tajo
constexpr uint32_t T_GOLPE_ENEMIGO = 200;
constexpr uint32_t T_MUERTE = 800;
constexpr uint32_t T_VICTORIA = 700;
constexpr uint32_t T_NO_MUERTO = 1800;  // el héroe se levanta como esqueleto

M5Canvas cv(&M5.Display);
gfx::Canvas4 c4;
Preferences prefs;
Rng rng;
Hero hero;
Enemy enemy;
Battle battle(hero, enemy, rng);

enum class St : uint8_t {
  TITLE, SELECT, FADE, BIOME_INTRO, ENEMY_ENTER, BOSS_INTRO, HERO_TURN, HERO_STRIKE,
  SHOW, ENEMY_WAIT, ENEMY_STRIKE, ENEMY_DIE, VICTORY, LOOT_ITEM, LOOT_POTION,
  GAME_OVER, UNDEAD
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
bool paused = false;
uint32_t lastInputMs = 0;  // millis() de la última pulsación
float batLevel = -1;       // % suavizado
int batCells = 3;          // celdas encendidas (1..3)
uint32_t soundToastMs = 0;  // millis() del último cambio de sonido
uint32_t pausedAtMs = 0;
bool aArmed = false, charging = false, inPerfect = false;
uint32_t aPressMs = 0;
float chargeF = 0;
bool pendingSpecial = false, pendingPerfect = false;
float pendingMult = 1.0f;
constexpr uint32_t LEJOS = 0x40000000u;  // "hace mucho" para los temporizadores
uint32_t talismanT = 0;  // momento en que cayó el talismán
uint32_t heroAtkT = 0, heroHitT = 0, enemyAtkT = 0, enemyHitT = 0;
uint32_t shakeT = 0, shakeDur = 0;
int shakeAmp = 0;
int enemyX = ENEMY_START;
Loot loot;
int sel = 0;
HeroClass elegido = HeroClass::CABALLERO;  // héroe de la partida

// ---------------------------------------------------------------- utilidades

uint32_t since(uint32_t t) { return gt - t; }
float lerp(float a, float b, float t) { return a + (b - a) * t; }
float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

void resetTimers() {
  talismanT = gt - LEJOS;
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
// Entra desde justo fuera de la pantalla, sea cual sea el ancho del sprite.
int enemyStart() { return std::max(ENEMY_START, W + art().reach + 2); }
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
  enemyX = enemyStart();
  resetTimers();
  for (Floater& f : floaters) f.on = false;
}

void newGame() {
  hero = makeHero(elegido);
  setWeaponName(elegido == HeroClass::VIKINGA ? "Hacha" : "Espada");
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
    case St::SELECT:
      sel = static_cast<int>(elegido);
      break;
    case St::BIOME_INTRO:
      audio::jingleBioma();
      break;
    case St::BOSS_INTRO: {
      audio::jingleJefe();
      shake(2, 600);
      int healed = healBeforeBoss(hero);
      char buf[12];
      std::snprintf(buf, sizeof(buf), "+%d", healed);
      if (healed > 0) floatText(buf, pal::ESPECTRAL, HERO_FX, FEET_Y - 82);
      break;
    }
    case St::HERO_TURN:
      aArmed = charging = inPerfect = false;
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
      if (isBoss(enemy.type) && rollTalisman(rng, hero)) {
        hero.talisman = true;
        talismanT = gt;
        audio::sfx(1180, 120, true);
      }
      break;
    }
    case St::UNDEAD:
      reviveUndead(hero);
      flashAmt = 1.0f;
      shake(4, 500);
      audio::jingleJefe();
      break;
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
  St next = St::HERO_TURN;
  if (r.heroDead) next = hero.talisman ? St::UNDEAD : St::GAME_OVER;
  show(label, color, dur, next);
}

// ------------------------------------------------------------------- update

void updateTitle() {
  if (M5.BtnA.wasPressed() && since(stT) > 300) {
    audio::sfx(660, 60, true);
    setState(St::SELECT);
  }
}

// Elegir héroe: B cambia de personaje, A lo confirma.
void updateSelect() {
  if (since(stT) < 250) return;
  if (M5.BtnB.wasPressed()) {
    sel = (sel + 1) % HERO_CLASS_COUNT;
    audio::sfx(770, 30);
  }
  if (M5.BtnA.wasPressed()) {
    elegido = static_cast<HeroClass>(sel);
    audio::sfx(990, 60, true);
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
  // Sin el especial disponible (§4.3), A ataca al pulsar.
  if (M5.BtnA.wasPressed() && !hero.especialListo()) {
    startHeroAttack(false, 1.0f, false);
    return;
  }
  if (M5.BtnA.wasPressed()) {
    aArmed = true;
    aPressMs = millis();
    charging = inPerfect = false;
    return;
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
    return;
  }
  // A se ha soltado con el ataque preparado.
  // Solo se lanza el especial (y se gastan las runas) con una carga real;
  // una pulsación que apenas pasa del umbral es un ataque normal.
  bool charged = held > bal::T_UMBRAL_MS + bal::T_CARGA_MIN_MS;
  float f = chargeFill(held);
  aArmed = charging = false;
  startHeroAttack(charged, charged ? chargeMultiplier(f) : 1.0f,
                  charged && chargeIsPerfect(f));
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
    case St::SELECT: updateSelect(); break;
    case St::FADE: updateFade(); break;
    case St::BIOME_INTRO:
      if (since(stT) > T_BIOMA || (since(stT) > 600 && M5.BtnA.wasPressed()))
        setState(St::ENEMY_ENTER);
      break;
    case St::ENEMY_ENTER: {
      float u = clamp01(since(stT) / static_cast<float>(T_ENTRADA));
      float e = 1 - (1 - u) * (1 - u);  // frena al llegar
      enemyX = static_cast<int>(lerp(enemyStart(), ENEMY_FX, e));
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
    case St::UNDEAD:
      if (since(stT) >= T_NO_MUERTO) setState(St::HERO_TURN);
      break;
    case St::VICTORY:
      // Si ha caído el talismán, se deja ver el aviso un poco más.
      if (since(stT) >= (since(talismanT) <= since(stT) ? 1600u : T_VICTORIA)) {
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

// Borde de 1 px en tono claro alrededor de la silueta (separa a los
// personajes del fondo): la capa en color sólido, desplazada en 4 sentidos.
// Se dibuja antes que el sprite, que lo tapa por dentro.
void blitHalo(const gfx::Sheet& s, int layer, int frame, int x, int y,
              int dissolve = 0, uint8_t color = pal::CLARO) {
  const Remap halo = Remap::solid(color);
  c4.blit(s, layer, frame, x - 1, y, halo, dissolve);
  c4.blit(s, layer, frame, x + 1, y, halo, dissolve);
  c4.blit(s, layer, frame, x, y - 1, halo, dissolve);
  c4.blit(s, layer, frame, x, y + 1, halo, dissolve);
}

void drawHero() {
  namespace H = gfx::heroe;  // capas y animaciones (iguales en todos los héroes)
  const gfx::HeroArt& art = hero.maldito
                                ? gfx::kUndeadArt
                                : gfx::kHeroArt[static_cast<int>(hero.clase)];
  int x0 = HERO_FX - art.ax, y0 = FEET_Y - art.ay;
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
  auto visible = [&](int layer, int slot) {
    return slot < 0 || layer == H::BRAZO || hero.equip[slot].present;
  };
  // Maldición no muerta: borde espectral en lugar del claro y, si aún no
  // hay arte del esqueleto, colores apagados.
  Remap curse = Remap::identity();
  if (hero.maldito && !gfx::kUndeadOwnArt) {
    curse.to[pal::SANGRE] = pal::MEDIO;
    curse.to[pal::ORO] = pal::CLARO;
    curse.to[pal::OSCURO] = pal::NEGRO;
  }
  uint8_t haloCol = hero.maldito ? pal::ESPECTRAL : pal::CLARO;
  for (auto& c : capas)
    if (visible(c.layer, c.slot))
      blitHalo(*art.sheet, c.layer, frame, x0, y0, 0, haloCol);
  for (auto& c : capas) {
    if (!visible(c.layer, c.slot)) continue;
    Remap rar = c.slot >= 0 ? rarityRemap(hero.equip[c.slot]) : Remap::identity();
    Remap r;
    for (int i = 0; i < 16; ++i) r.to[i] = rar.to[curse.to[i]];
    c4.blit(*art.sheet, c.layer, frame, x0, y0, flash ? white : r);
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
  blitHalo(*a.sheet, 0, frame, x0, y0, dissolve);
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
    if (l == 0 && curBiome != static_cast<int>(Bioma::CASTILLO) &&
        curBiome != static_cast<int>(Bioma::CUEVA) &&
        curBiome != static_cast<int>(Bioma::CRIPTA))
      clouds::draw(c4, gt);
  }
}

const char* const RUNE5[] = {"..#..", ".#o#.", "#ooo#", ".#o#.", "..#.."};

void drawHud() {
  char buf[24];
  // Héroe (espejo de la placa del enemigo): nombre arriba; vida y barra debajo.
  panel(1, 112, 98, 134);
  icon(HEART, 6, 5, 116, '#', pal::NEGRO, 'r',
       hero.maldito ? pal::ESPECTRAL : pal::SANGRE);
  const char* heroName = hero.maldito ? "ESQUELETO"
                         : hero.clase == HeroClass::VIKINGA ? "VIKINGA"
                                                            : "CABALLERO";
  c4.label(heroName, 15, 116, hero.maldito ? pal::ESPECTRAL : pal::BRILLO,
           pal::NEGRO);
  // Talismán de los no muertos: calavera en la esquina de la placa.
  if (hero.talisman)
    icon(SKULL, 6, 89, 116, '#', pal::NEGRO, 'c', pal::ESPECTRAL);
  std::snprintf(buf, sizeof(buf), "%d/%d", hero.hp, hero.hpMax);
  int hw = gfx::Canvas4::textWidth(buf, 1, true);
  c4.label(buf, 4, 125, pal::BRILLO, pal::NEGRO);
  bar(hw + 7, 125, 95 - hw - 7, 6, hero.hp, hero.hpMax, pal::SANGRE);

  // Centro: capítulo en romanos y encuentro dentro de él ("II-4"), y runas
  // del especial.
  panel(100, 112, 135, 134);
  char rom[16];
  roman((nivel - 1) / bal::JEFE_CADA + 1, rom);
  std::snprintf(buf, sizeof(buf), "%s-%d", rom, (nivel - 1) % bal::JEFE_CADA + 1);
  bool boldLv = gfx::Canvas4::textWidth(buf, 1, true) <= 30;
  c4.label(buf, 118, 116, isBossLevel(nivel) ? pal::SANGRE : pal::ORO,
           pal::NEGRO, 1, 1, false, boldLv);
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

// Barra de carga del especial: a lo ancho de la pantalla, arriba.
void drawChargeBar() {
  const int x0 = 8, y0 = 6, w = W - 16, h = 9;
  panel(x0 - 4, y0 - 4, x0 + w + 3, y0 + h + 3);
  int zone = static_cast<int>(w * bal::ZONA_PERFECTA);
  c4.rect(x0, y0, w, h, pal::OSCURO);
  c4.shade(x0 + zone, y0, w - zone, h, pal::ORO, 8);
  int f = static_cast<int>(w * chargeF);
  uint8_t col = inPerfect ? pal::ORO : pal::ESPECTRAL;
  c4.rect(x0, y0 + 1, f, h - 2, col);
  for (int x = x0 + 1; x < x0 + f; x += 2) c4.pset(x, y0 + 1, pal::BRILLO);
  for (int y = y0 - 3; y < y0 + h + 3; ++y) c4.pset(x0 + zone, y, pal::ORO);
}

void drawFloaters() {
  for (const Floater& f : floaters) {
    if (!f.on) continue;
    int dy = static_cast<int>(since(f.t0) / 22);
    c4.label(f.text, f.x, f.y - dy, f.color, pal::NEGRO, 1, 2, true, false);
  }
}

// Nombre completo y claro de cada atributo (pantalla de estadísticas).
const char* statLabel(Stat s) {
  switch (s) {
    case Stat::ATK: return "Ataque";
    case Stat::DEF: return "Defensa";
    case Stat::CRIT: return "Critico";
    case Stat::CRIT_DMG: return "Poder critico";
    case Stat::EXTRA: return "Turno extra";
    case Stat::LUCK: return "Suerte";
    case Stat::VAMP: return "Vampirismo";
    default: return "?";
  }
}

[[maybe_unused]] void drawStatsPanel() {
  c4.shade(0, 0, W, H, pal::NEGRO, 11);
  char buf[28];
  // Una sola placa: nombre del atributo a la izquierda, valor a la derecha.
  const int x0 = 44, x1 = 195;
  panel(x0, 3, x1, 131);
  txt(B9, "Atributos", W / 2, 13, pal::ORO, middle_center, NONE);
  for (int x = x0 + 6; x < x1 - 5; ++x)
    if (x % 2) c4.pset(x, 23, pal::MEDIO);
  int y = 33;
  txt(S9, "Vida", x0 + 10, y, pal::CLARO, middle_left, NONE);
  std::snprintf(buf, sizeof(buf), "%d/%d", hero.hp, hero.hpMax);
  txt(B9, buf, x1 - 10, y, pal::SANGRE, middle_right, NONE);
  for (int i = 0; i < STAT_COUNT; ++i) {
    Stat s = static_cast<Stat>(i);
    y += 12;
    bool capped = hero.raw(s) > hero.eff(s);
    txt(S9, statLabel(s), x0 + 10, y, pal::CLARO, middle_left, NONE);
    const char* pre = s == Stat::CRIT_DMG ? "+" : "";
    std::snprintf(buf, sizeof(buf), "%s%d%s%s", pre, hero.eff(s),
                  statIsPercent(s) ? "%" : "", capped ? " max" : "");
    txt(B9, buf, x1 - 10, y, capped ? pal::ORO : pal::BRILLO, middle_right, NONE);
  }
  txt(I9, "B: volver", W / 2, 124, pal::MEDIO, middle_center, NONE);
}

void drawSceneNoEnemy() {
  drawBackground(0, gfx::BG_LAYER_PERSONAJES);
  drawHero();
  drawBackground(gfx::BG_LAYER_PERSONAJES, 6);
}


// Indicador de mejora/empeora: flecha de color y número en serif negrita.
void delta(int x, int y, int d) {
  bool up = d > 0;
  uint8_t col = up ? pal::ESPECTRAL : pal::SANGRE;
  for (int k = 0; k < 5; ++k)
    for (int j = -k; j <= k; ++j)
      c4.pset(x + 4 + j, up ? y - 2 + k : y + 2 - k, col);
  char num[12];
  std::snprintf(num, sizeof(num), "%d", up ? d : -d);
  c4.label(num, x + 11, y - 3, col, pal::NEGRO);
}

// Marco de una placa seleccionable: oro doble con esquinas que laten, o
// atenuada si no está elegida.
void choiceFrame(int x0, int y0, int x1, int y1, bool selected) {
  if (selected) {
    c4.frame(x0, y0, x1 - x0 + 1, y1 - y0 + 1, pal::ORO);
    c4.frame(x0 + 2, y0 + 2, x1 - x0 - 3, y1 - y0 - 3, pal::ORO);
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
    txt(B9, slotName(it.slot), cx, 18, rc, middle_center, NONE);
    txt(I9, rarezaName(it.rareza), cx, 33, rc, middle_center, NONE);
  } else {
    txt(B9, "Vacio", cx, 18, pal::MEDIO, middle_center, NONE);
    txt(I9, "sin objeto", cx, 33, pal::MEDIO, middle_center, NONE);
  }
  for (int x = x0 + 6; x < x1 - 5; ++x)
    if (x % 2) c4.pset(x, 43, pal::MEDIO);

  // Lista en fuente pixel (nítida a tamaño pequeño) con interlineado amplio.
  int step = n > 5 ? 12 : 14;
  for (int i = 0; i < n; ++i) {
    Stat s = list[i];
    int y = 50 + i * step;  // parte superior del texto (7 px de alto)
    int v = it.present ? it.get(s) : 0;
    int vx = isNew ? x1 - 34 : x1 - 9;
    c4.label(statName(s), x0 + 9, y, pal::CLARO, pal::NEGRO);
    if (v) {
      std::snprintf(buf, sizeof(buf), "%+d%s", v, statIsPercent(s) ? "%" : "");
      c4.label(buf, vx, y, pal::BRILLO, pal::NEGRO, 2);
    } else {
      c4.label("--", vx, y, pal::MEDIO, pal::NEGRO, 2);
    }
    if (isNew) {
      int a = other && other->present ? other->get(s) : 0;
      int d = v - a;
      if (d) delta(x1 - 29, y + 3, d);
    }
  }

  choiceFrame(x0, y0, x1, y1, selected);
}

void drawLootItem() {
  drawSceneNoEnemy();
  c4.shade(0, 0, W, H, pal::NEGRO, 11);
  const Item& cur = hero.equip[static_cast<int>(loot.item.slot)];
  const Item& nue = loot.item;

  // Equipado: tal cual, solo sus estadísticas.
  Stat curList[MAX_MODS];
  int nc = 0;
  for (int i = 0; cur.present && i < cur.nMods; ++i) curList[nc++] = cur.mods[i].stat;
  // Nuevo: sus estadísticas comparadas y, al final, las que se perderían.
  Stat newList[2 * MAX_MODS];
  int nn = 0;
  for (int i = 0; i < nue.nMods; ++i) newList[nn++] = nue.mods[i].stat;
  for (int i = 0; i < nc; ++i)
    if (nue.get(curList[i]) == 0) newList[nn++] = curList[i];

  lootColumn(cur, nullptr, 3, 117, curList, nc, sel == 0, false);
  lootColumn(nue, &cur, 122, 236, newList, nn, sel == 1, true);
}

void potionColumn(const char* title, const char* l1, const char* l2,
                  int hp, int hpMax, int x0, int x1, bool selected) {
  const int y0 = 3, y1 = 131;
  panel(x0, y0, x1, y1);
  int cx = (x0 + x1) / 2;
  txt(B9, title, cx, 18, pal::ORO, middle_center, NONE);
  for (int x = x0 + 6; x < x1 - 5; ++x)
    if (x % 2) c4.pset(x, 30, pal::MEDIO);
  txt(S9, l1, cx, 46, pal::CLARO, middle_center, NONE);
  txt(S9, l2, cx, 60, pal::CLARO, middle_center, NONE);
  char buf[24];
  std::snprintf(buf, sizeof(buf), "%d/%d", hp, hpMax);
  txt(B9, buf, cx, 86, pal::ESPECTRAL, middle_center, NONE);
  std::snprintf(buf, sizeof(buf), "ahora %d/%d", hero.hp, hero.hpMax);
  txt(I9, buf, cx, 106, pal::MEDIO, middle_center, NONE);
  choiceFrame(x0, y0, x1, y1, selected);
}

void drawLootPotion() {
  drawSceneNoEnemy();
  c4.shade(0, 0, W, H, pal::NEGRO, 11);
  int r = potionRecoverGain(hero);
  int g = potionMaxGain(hero);
  char l0[24], l1[24];
  std::snprintf(l0, sizeof(l0), "+%d de vida", r);
  std::snprintf(l1, sizeof(l1), "+%d de vida max.", g);
  potionColumn("Recuperar", l0, "50% de lo perdido", hero.hp + r, hero.hpMax,
               3, 117, sel == 0);
  potionColumn("Vida max.", l1, "para siempre", hero.hp, hero.hpMax + g,
               122, 236, sel == 1);
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
  // Ilustración del título (ya trae su viñeteado); solo se oscurece la franja
  // inferior para que se lean el récord y el sonido.
  c4.blit(gfx::titulo_fondo::sheet, 0, 0, 0, 0, Remap::identity());
  c4.shade(0, 118, W, H - 118, pal::NEGRO, 6);

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

// Placa de un héroe en la pantalla de selección: nombre, sprite respirando
// con todo el equipo y su estilo de juego.
void heroColumn(HeroClass c, int x0, int x1, bool selected) {
  namespace H = gfx::heroe;
  const int y0 = 3, y1 = 131;
  panel(x0, y0, x1, y1);
  int cx = (x0 + x1) / 2;
  txt(B9, heroClassName(c), cx, 16, pal::ORO, middle_center, NONE);
  const gfx::HeroArt& art = gfx::kHeroArt[static_cast<int>(c)];
  int frame = selected ? H::IDLE.frameAt(gt) : H::IDLE.from;
  int fx = cx - 6, fy = 100;  // pies
  for (int l = 0; l < H::LAYER_COUNT; ++l)
    blitHalo(*art.sheet, l, frame, fx - art.ax, fy - art.ay);
  for (int l = 0; l < H::LAYER_COUNT; ++l)
    c4.blit(*art.sheet, l, frame, fx - art.ax, fy - art.ay, Remap::identity());
  const char* l1 = c == HeroClass::VIKINGA ? "ATAQUE Y CRITICO" : "MAS DEFENSA";
  const char* l2 = c == HeroClass::VIKINGA ? "MENOS DEFENSA" : "EQUILIBRADO";
  c4.label(l1, cx, 108, pal::CLARO, pal::NEGRO, 1);
  c4.label(l2, cx, 118, pal::MEDIO, pal::NEGRO, 1);
  choiceFrame(x0, y0, x1, y1, selected);
}

void drawSelect() {
  c4.blit(gfx::titulo_fondo::sheet, 0, 0, 0, 0, Remap::identity());
  c4.shade(0, 0, W, H, pal::NEGRO, 9);
  heroColumn(HeroClass::CABALLERO, 3, 117, sel == 0);
  heroColumn(HeroClass::VIKINGA, 122, 236, sel == 1);
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

// Pila de 3 celdas arriba a la derecha: verde con 2-3 celdas, roja con 1.
void drawBattery() {
  const int x0 = W - 20, y0 = 3;
  c4.rect(x0 - 1, y0 - 1, 18, 9, pal::NEGRO);
  for (int x = x0; x <= x0 + 14; ++x) {
    c4.pset(x, y0, pal::CLARO);
    c4.pset(x, y0 + 6, pal::CLARO);
  }
  for (int y = y0; y <= y0 + 6; ++y) {
    c4.pset(x0, y, pal::CLARO);
    c4.pset(x0 + 14, y, pal::CLARO);
  }
  for (int y = y0 + 2; y <= y0 + 4; ++y) c4.pset(x0 + 15, y, pal::CLARO);
  uint8_t col = batCells >= 2 ? COL_BAT_OK : COL_BAT_BAJA;
  for (int i = 0; i < batCells; ++i)
    c4.rect(x0 + 2 + i * 4, y0 + 2, 3, 3, col);
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
  }
  if (view == St::BOSS_INTRO) drawBossBanner();
  if (view == St::VICTORY && since(talismanT) <= since(stT)) {
    icon(SKULL, 6, W / 2 - 3, 30, '#', pal::NEGRO, 'c', pal::ESPECTRAL);
    txt(B9, "Talisman no muerto", W / 2, 50, pal::ESPECTRAL);
  }
  if (view == St::UNDEAD) {
    txt(B12, "Maldicion no muerta", W / 2, 42, pal::ESPECTRAL);
    txt(S9, "Te levantas como esqueleto", W / 2, 62, pal::BRILLO);
  }
  drawFloaters();
}

void render() {
  applyPalette();
  St view = st;
  if (st == St::FADE) {
    // Durante el fundido se ve la escena de origen y después la de destino.
    if (!fadeDone)
      view = fadeAct == FadeAct::NEW_GAME   ? St::SELECT
             : fadeAct == FadeAct::TO_TITLE ? St::GAME_OVER
                                            : St::VICTORY;
    else
      view = fadeNext == St::ENEMY_ENTER ? St::VICTORY : fadeNext;
  }
  switch (view) {
    case St::TITLE: drawTitle(); break;
    case St::SELECT: drawSelect(); break;
    case St::BIOME_INTRO: drawBiomeIntro(); break;
    case St::LOOT_ITEM: drawLootItem(); break;
    case St::LOOT_POTION: drawLootPotion(); break;
    case St::GAME_OVER: drawGameOver(); break;
    default: drawCombat(view); break;
  }
  // Aviso breve al cambiar el sonido (en el título ya lo indica su rótulo).
  if (view != St::TITLE && soundToastMs && millis() - soundToastMs < 1000)
    c4.label(audio::enabled() ? "SONIDO SI" : "SONIDO NO", W / 2, 24,
             pal::BRILLO, pal::NEGRO, 1, 1, true);
  if (!(view == St::HERO_TURN && charging)) drawBattery();
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

// B activa/desactiva el sonido en cualquier momento, salvo en el botín (donde
// cambia de panel) y con A pulsado (cargando el especial).
void handleSound() {
  if (!M5.BtnB.wasPressed()) return;
  if (!paused && (st == St::LOOT_ITEM || st == St::LOOT_POTION ||
                  st == St::SELECT))
    return;
  if (aArmed) return;
  audio::setEnabled(!audio::enabled());
  prefs.putBool("sound", audio::enabled());
  audio::sfx(880, 60);
  soundToastMs = millis();
}

void readBattery() {
  int lv = M5.Power.getBatteryLevel();
  if (lv < 0 || lv > 100) return;
  batLevel = batLevel < 0 ? lv : batLevel * 0.7f + lv * 0.3f;
  // Umbrales 67 % / 34 %, con 3 puntos de margen para no parpadear.
  int t = batLevel >= 67 ? 3 : batLevel >= 34 ? 2 : 1;
  if (t > batCells && batLevel < (t == 3 ? 70 : 37)) t = batCells;
  if (t < batCells && batLevel > (batCells == 3 ? 64 : 31)) t = batCells;
  batCells = t;
}

void updateBattery() {
  static uint32_t lastRead = 0;
  if (lastRead && millis() - lastRead < T_BATERIA_MS) return;
  lastRead = millis();
  readBattery();
}

bool anyButton() {
  return M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnPWR.isPressed();
}

void powerOff() {
  M5.Speaker.stop();
  M5.Display.setBrightness(0);
  M5.Power.powerOff();
}

// Reposo: pantalla y CPU dormidas hasta pulsar un botón. Si se agota el
// tiempo sin tocar nada, se apaga del todo.
void standby() {
  M5.Speaker.stop();
  aArmed = charging = false;
  if (st != St::TITLE) paused = true;
  M5.Display.setBrightness(0);
  M5.Display.sleep();
  uint32_t t0 = millis();
  bool woke = false;
  while (!woke) {
    uint32_t slept = millis() - t0;
    if (slept >= T_AUTOAPAGADO_MS) powerOff();
    gpio_wakeup_enable(GPIO_NUM_37, GPIO_INTR_LOW_LEVEL);  // A
    gpio_wakeup_enable(GPIO_NUM_39, GPIO_INTR_LOW_LEVEL);  // B
    gpio_wakeup_enable(GPIO_NUM_35, GPIO_INTR_LOW_LEVEL);  // PWR
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(
        static_cast<uint64_t>(T_AUTOAPAGADO_MS - slept) * 1000);
    gpio_hold_en(GPIO_NUM_4);  // mantiene el equipo encendido con batería
    esp_light_sleep_start();
    gpio_hold_dis(GPIO_NUM_4);
    woke = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO;
  }
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  // La pulsación que despierta no cuenta como acción del juego.
  while (digitalRead(37) == LOW || digitalRead(39) == LOW ||
         digitalRead(35) == LOW)
    delay(10);
  delay(30);
  M5.update();
  M5.Display.wakeup();
  M5.Display.setBrightness(BRILLO_PANTALLA);
  pausedAtMs = lastInputMs = millis();
}

void handleIdle() {
  if (anyButton()) lastInputMs = millis();
  if (millis() - lastInputMs > T_REPOSO_MS) standby();
}

void handlePower() {
  if (M5.BtnPWR.pressedFor(T_APAGAR_MS)) powerOff();
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
  cv.setPaletteColor(COL_BAT_OK, 40, 200, 70);
  cv.setPaletteColor(COL_BAT_BAJA, 220, 40, 40);

  prefs.begin("stickrpg", false);
  record = prefs.getUInt("record", 0);
  audio::begin(prefs.getBool("sound", true));

  rng.reseed((static_cast<uint64_t>(esp_random()) << 32) | esp_random());
  hero = makeHero();
  clouds::generate(esp_random());
  resetTimers();
  setState(St::TITLE);
  readBattery();
  lastInputMs = millis();
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  uint32_t dt = now - last;
  last = now;

  M5.update();
  handleIdle();
  handlePower();
  updateBattery();
  handleSound();
  audio::update();
  if (!paused) {
    gt += dt;
    update();
  }
  render();

  uint32_t spent = millis() - now;
  if (spent < FRAME_MS) delay(FRAME_MS - spent);
}
