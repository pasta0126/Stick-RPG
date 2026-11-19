#include "combat.h"

#include <cmath>

#include "balance.h"

float chargeMultiplier(float f) {
  if (f < 0) f = 0;
  if (f > 1) f = 1;
  if (f >= bal::ZONA_PERFECTA) return bal::CARGA_PERFECTA;
  return bal::CARGA_MIN + (bal::CARGA_MAX - bal::CARGA_MIN) * f;
}

bool chargeIsPerfect(float f) { return f >= bal::ZONA_PERFECTA; }

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

HeroTurnResult Battle::heroAttack(float chargeMult) {
  HeroTurnResult r;
  r.atk = resolveAttack(rng_, hero_.combatant(), enemy_.c, chargeMult);
  enemy_.hp -= r.atk.damage;
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
