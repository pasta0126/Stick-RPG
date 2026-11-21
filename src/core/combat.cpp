#include "combat.h"

#include <cmath>

#include "balance.h"

float chargeMultiplier(float f, float zona) {
  if (f < 0) f = 0;
  if (f > 1) f = 1;
  if (f >= zona) return bal::CARGA_PERFECTA;
  return bal::CARGA_MIN + (bal::CARGA_MAX - bal::CARGA_MIN) * f;
}

bool chargeIsPerfect(float f, float zona) { return f >= zona; }

float chargeFill(uint32_t heldMs) {
  if (heldMs <= bal::T_UMBRAL_MS) return 0.0f;
  uint32_t t = (heldMs - bal::T_UMBRAL_MS) % bal::CICLO_CARGA_MS;
  return static_cast<float>(t) / static_cast<float>(bal::CICLO_CARGA_MS - 1);
}

float baseDamage(int atk, int def) {
  if (atk < 1) atk = 1;
  if (def < 0) def = 0;
  return static_cast<float>(atk) * atk / static_cast<float>(atk + def);
}

AttackResult resolveAttack(Rng& rng, const Combatant& a, const Combatant& d,
                           float mult) {
  AttackResult r;
  r.hit = !rng.chance(a.fallo);
  if (!r.hit) return r;
  float dmg = baseDamage(a.atk, d.def) * mult;
  r.crit = rng.chance(a.crit);
  if (r.crit) dmg *= 1.0f + a.critDmg / 100.0f;
  r.damage = static_cast<int>(std::lround(dmg));
  if (r.damage < 1) r.damage = 1;
  return r;
}

float Battle::extraChance() const {
  return hero_.eff(Stat::EXTRA) / static_cast<float>(1 << chain_);
}

HeroTurnResult Battle::attack(float mult, bool special) {
  HeroTurnResult r;
  if (special && !hero_.especialListo()) {
    special = false;
    mult = 1.0f;
  }
  // El contador se mantiene entre combates; los fallos también cuentan.
  if (special)
    hero_.golpesEspecial = 0;
  else if (hero_.golpesEspecial < hero_.info().especialCada)
    ++hero_.golpesEspecial;
  r.atk = resolveAttack(rng_, hero_.combatant(), enemy_.c, mult);
  enemy_.hp -= r.atk.damage;
  // Vampirismo (§5): recupera un % del daño infligido, mínimo 1 si hay VAMP.
  int vamp = hero_.eff(Stat::VAMP);
  if (vamp > 0 && r.atk.damage > 0) {
    int heal = r.atk.damage * vamp / 100;
    if (heal < 1) heal = 1;
    int before = hero_.hp;
    hero_.hp = hero_.hp + heal > hero_.hpMax ? hero_.hpMax : hero_.hp + heal;
    r.healed = hero_.hp - before;
  }
  // Regeneración propia del héroe (Druida): un % de la vida máx. por ataque,
  // con tope por combate para que no se eternice contra enemigos que apenas
  // le hacen daño.
  int regen = hero_.info().regen;
  int tope = hero_.hpMax * bal::TOPE_REGEN / 100 - regenado_;
  if (regen > 0 && hero_.hp < hero_.hpMax && tope > 0) {
    int heal = hero_.hpMax * regen / 100;
    if (heal < 1) heal = 1;
    if (heal > tope) heal = tope;
    regenado_ += heal;
    int before = hero_.hp;
    hero_.hp = hero_.hp + heal > hero_.hpMax ? hero_.hpMax : hero_.hp + heal;
    r.healed += hero_.hp - before;
  }
  if (enemy_.hp <= 0) {
    enemy_.hp = 0;
    r.enemyDead = true;
    return r;
  }
  // La comprobación ocurre siempre, también tras un fallo (§4.4).
  r.extraTurn = rng_.unit() * 100.0f < extraChance();
  if (r.extraTurn)
    ++chain_;
  else
    chain_ = 0;
  return r;
}

EnemyTurnResult Battle::enemyTurn() {
  EnemyTurnResult r;
  chain_ = 0;
  if (enemy_.preparado) {
    r.action = EnemyAction::GOLPE_FUERTE;
    enemy_.preparado = false;
    r.atk = resolveAttack(rng_, enemy_.c, hero_.combatant(), bal::GOLPE_FUERTE);
  } else if (rng_.chance(enemy_.probPreparar)) {
    r.action = EnemyAction::PREPARAR;
    enemy_.preparado = true;
    return r;
  } else {
    r.action = EnemyAction::ATAQUE;
    r.atk = resolveAttack(rng_, enemy_.c, hero_.combatant(), 1.0f);
  }
  hero_.hp -= r.atk.damage;
  if (hero_.hp <= 0) {
    hero_.hp = 0;
    r.heroDead = true;
  }
  return r;
}
