// Pruebas de la lógica del juego (sin hardware). Ejecutar: make test

#include <cmath>
#include <cstdio>

#include "../../src/core/balance.h"
#include "../../src/core/combat.h"
#include "../../src/core/progression.h"

static int failures = 0;
#define CHECK(cond)                                                    \
  do {                                                                 \
    if (!(cond)) {                                                     \
      std::printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
      ++failures;                                                      \
    }                                                                  \
  } while (0)

static bool near(float a, float b) { return std::fabs(a - b) < 1e-3f; }

static void testDamage() {
  CHECK(near(baseDamage(10, 10), 5.0f));   // ATK = DEF → mitad
  CHECK(near(baseDamage(10, 0), 10.0f));
  CHECK(baseDamage(10, 1000) > 0.0f);      // nunca se anula

  Rng rng(7);
  Combatant a; a.atk = 1; a.fallo = 0;
  Combatant d; d.def = 500;
  for (int i = 0; i < 100; ++i) {
    AttackResult r = resolveAttack(rng, a, d, 1.0f);
    CHECK(r.hit && r.damage >= 1);         // mínimo 1 al acertar
  }
}

static void testCharge() {
  CHECK(near(chargeMultiplier(0.0f), bal::CARGA_MIN));
  CHECK(near(chargeMultiplier(0.5f), (bal::CARGA_MIN + bal::CARGA_MAX) / 2));
  CHECK(near(chargeMultiplier(0.95f), bal::CARGA_PERFECTA));
  CHECK(chargeFill(bal::T_UMBRAL_MS) == 0.0f);
  // La barra es cíclica: tras un ciclo completo vuelve a empezar.
  CHECK(chargeFill(bal::T_UMBRAL_MS + bal::CICLO_CARGA_MS) < 0.01f);
  CHECK(chargeFill(bal::T_UMBRAL_MS + bal::CICLO_CARGA_MS - 1) > 0.99f);
}

static void testCaps() {
  Hero h = makeHero();
  Item it; it.present = true; it.slot = Slot::ESCUDO; it.nMods = 2;
  it.mods[0] = {Stat::CRIT, 500};
  it.mods[1] = {Stat::EXTRA, 500};
  equipItem(h, it);
  CHECK(h.eff(Stat::CRIT) == bal::TOPE_CRIT);
  CHECK(h.eff(Stat::EXTRA) == bal::TOPE_EXTRA);
  CHECK(h.raw(Stat::CRIT) > bal::TOPE_CRIT);
}

static void testItems() {
  Rng rng(3);
  for (int i = 0; i < 2000; ++i) {
    Slot s = static_cast<Slot>(i % SLOT_COUNT);
    Item it = makeItem(rng, s, 1 + i % 80, static_cast<Rareza>(i % 3));
    CHECK(it.nMods >= 1 && it.nMods <= MAX_MODS);
    CHECK(it.mods[0].value >= 1);
  }
  for (int i = 0; i < 200; ++i)
    CHECK(rollRareza(rng, 0, true) != Rareza::COMUN);  // jefe ≥ rara
}

static void testHealing() {
  Hero h = makeHero();
  h.hp = 10;
  int healed = healVictory(h);
  CHECK(healed == h.hpMax * bal::CURA_VICTORIA / 100);
  h.hp = h.hpMax - 1;
  healVictory(h);
  CHECK(h.hp == h.hpMax);                  // sin superar el máximo

  h.hp = 23; h.hpMax = 50;
  potionMaxHp(h);
  CHECK(h.hp == 28 && h.hpMax == 55);      // ejemplo del GDD §6.4
  potionRecover(h);
  CHECK(h.hp == 55);
}

static void testExtraChain() {
  Hero h = makeHero();
  Item it; it.present = true; it.slot = Slot::ESCUDO; it.nMods = 1;
  it.mods[0] = {Stat::EXTRA, 100};
  equipItem(h, it);
  Rng rng(11);
  Enemy e = makeEnemy(rng, 1);
  e.hp = e.hpMax = 100000;
  Battle b(h, e, rng);
  int maxChain = 0;
  for (int i = 0; i < 5000; ++i) {
    HeroTurnResult r = b.heroAttack(1.0f);
    if (r.extraTurn) maxChain = b.chain() > maxChain ? b.chain() : maxChain;
    else b.enemyTurn(), h.hp = h.hpMax;
  }
  CHECK(maxChain >= 1);
  CHECK(maxChain < 10);                    // la cadena se agota
}

static void testEnemies() {
  Rng rng(5);
  CHECK(makeEnemy(rng, 1).type == EnemyType::BANDIDO);
  CHECK(makeEnemy(rng, 10).type == EnemyType::JEFE);
  CHECK(makeEnemy(rng, 30).hpMax > makeEnemy(rng, 1).hpMax);
}

int main() {
  testDamage();
  testCharge();
  testCaps();
  testItems();
  testHealing();
  testExtraChain();
  testEnemies();
  if (failures) {
    std::printf("%d fallos\n", failures);
    return 1;
  }
  std::printf("OK\n");
  return 0;
}
