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
  CHECK(h.hp == 23 && h.hpMax == 60);      // ejemplo del GDD §6.4
  potionRecover(h);
  CHECK(h.hp == 42);                       // +50 % de los 37 que faltan
  h.hp = h.hpMax;
  CHECK(potionRecover(h) == 0);

  h.hp = 20; h.hpMax = 60;
  CHECK(healBeforeBoss(h) == 20 && h.hp == 40);  // 50 % de lo que falta
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
    HeroTurnResult r = b.heroAttack();
    if (r.extraTurn) maxChain = b.chain() > maxChain ? b.chain() : maxChain;
    else b.enemyTurn(), h.hp = h.hpMax;
  }
  CHECK(maxChain >= 1);
  CHECK(maxChain < 10);                    // la cadena se agota
}

static void testEnemies() {
  Rng rng(5);
  for (int n = 1; n < 200; ++n)
    CHECK(isBoss(makeEnemy(rng, n).type) == isBossLevel(n));
  CHECK(makeEnemy(rng, bal::JEFE_CADA).type == EnemyType::REY_CIERVO);
  CHECK(makeEnemy(rng, 5 * bal::JEFE_CADA).type == EnemyType::REY_LICHE);
  CHECK(makeEnemy(rng, 6 * bal::JEFE_CADA).type == EnemyType::REY_CIERVO);
  CHECK(makeEnemy(rng, 30).hpMax > makeEnemy(rng, 1).hpMax);
}

static void testVamp() {
  Hero h = makeHero();
  Item it; it.present = true; it.slot = Slot::ESCUDO; it.nMods = 1;
  it.mods[0] = {Stat::VAMP, 500};
  equipItem(h, it);
  CHECK(h.eff(Stat::VAMP) == bal::TOPE_VAMP);
  Rng rng(4);
  Enemy e = makeEnemy(rng, 1);
  e.hp = e.hpMax = 100000;
  h.hp = 10;
  Battle b(h, e, rng);
  int healedTotal = 0;
  for (int i = 0; i < 50; ++i) {
    HeroTurnResult r = b.heroAttack();
    healedTotal += r.healed;
    if (r.atk.damage > 0) CHECK(r.healed >= 0);
    CHECK(h.hp <= h.hpMax);
  }
  CHECK(healedTotal > 0);
}

static void testSpecial() {
  Hero h = makeHero();
  Rng rng(9);
  Enemy e = makeEnemy(rng, 1);
  e.hp = e.hpMax = 100000;
  Battle b(h, e, rng);
  CHECK(!h.especialListo());
  b.heroAttack(); b.heroAttack();
  CHECK(!h.especialListo());
  b.heroAttack();
  CHECK(h.especialListo());                 // a partir del 3.er golpe
  b.heroSpecial(2.0f);
  CHECK(h.golpesEspecial == 0);             // se vacía al usarlo
  b.heroSpecial(2.0f);                      // sin cargar: cuenta como normal
  CHECK(h.golpesEspecial == 1);
}

static void testTalisman() {
  Hero h = makeHero();
  CHECK(!reviveUndead(h));                  // sin talismán no se levanta
  Rng rng(7);
  int drops = 0;
  for (int i = 0; i < 1000; ++i) drops += rollTalisman(rng, h);
  CHECK(drops > 250 && drops < 450);        // ~PROB_TALISMAN
  h.talisman = true;
  CHECK(!rollTalisman(rng, h));             // no se acumulan
  int def = h.raw(Stat::DEF), fallo = h.fallo, hpMax = h.hpMax;
  h.hp = 0;
  CHECK(reviveUndead(h));
  CHECK(h.maldito && !h.talisman);
  CHECK(h.hpMax < hpMax && h.hp == h.hpMax);
  CHECK(h.fallo > fallo && h.raw(Stat::DEF) < def);
  CHECK(h.equip[static_cast<int>(Slot::ESPADA)].present);  // conserva el equipo
  CHECK(!rollTalisman(rng, h));             // maldito: ya no cae
  h.talisman = true;
  CHECK(!reviveUndead(h));                  // solo una vez
}

int main() {
  testTalisman();
  testVamp();
  testSpecial();
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
