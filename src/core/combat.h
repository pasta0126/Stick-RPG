#pragma once
// Reglas de combate (GDD §4).

#include "balance.h"
#include "game_types.h"
#include "rng.h"

// Multiplicador del ataque cargado según el llenado f ∈ [0, 1] (§4.3).
// zona: inicio de la zona perfecta (el Samurái la tiene más ancha).
float chargeMultiplier(float f, float zona = bal::ZONA_PERFECTA);
bool chargeIsPerfect(float f, float zona = bal::ZONA_PERFECTA);

// Llenado de la barra tras mantener A durante heldMs (barra cíclica).
float chargeFill(uint32_t heldMs);

// daño_base = ATK² / (ATK + DEF) (§4.2).
float baseDamage(int atk, int def);

struct AttackResult {
  bool hit = false;
  bool crit = false;
  int damage = 0;
};

AttackResult resolveAttack(Rng& rng, const Combatant& a, const Combatant& d,
                           float mult);

struct HeroTurnResult {
  AttackResult atk;
  int healed = 0;  // vida robada por VAMP + regeneración del héroe
  bool extraTurn = false;
  bool enemyDead = false;
};

enum class EnemyAction : uint8_t { ATAQUE, PREPARAR, GOLPE_FUERTE };

struct EnemyTurnResult {
  EnemyAction action = EnemyAction::ATAQUE;
  AttackResult atk;
  bool heroDead = false;
};

class Battle {
 public:
  Battle(Hero& hero, Enemy& enemy, Rng& rng)
      : hero_(hero), enemy_(enemy), rng_(rng) {}

  // Ataque normal: suma 1 golpe al contador del especial.
  HeroTurnResult heroAttack() { return attack(1.0f, false); }
  // Ataque cargado: solo si hero.especialListo(); vacía el contador.
  HeroTurnResult heroSpecial(float chargeMult) { return attack(chargeMult, true); }
  EnemyTurnResult enemyTurn();

  // Probabilidad (%) de turno extra en este momento (con encadenamiento).
  float extraChance() const;
  int chain() const { return chain_; }
  void reset() { chain_ = 0; regenado_ = 0; }

 private:
  HeroTurnResult attack(float mult, bool special);

  Hero& hero_;
  Enemy& enemy_;
  Rng& rng_;
  int chain_ = 0;  // turnos extra consecutivos
  int regenado_ = 0;  // vida regenerada en este combate (tiene tope)
};
