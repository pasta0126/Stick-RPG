#pragma once
// Reglas de combate (GDD §4).

#include "game_types.h"
#include "rng.h"

// Multiplicador del ataque cargado según el llenado f ∈ [0, 1] (§4.3).
float chargeMultiplier(float f);
bool chargeIsPerfect(float f);

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

  // chargeMult = 1 para el ataque normal.
  HeroTurnResult heroAttack(float chargeMult);
  EnemyTurnResult enemyTurn();

  // Probabilidad (%) de turno extra en este momento (con encadenamiento).
  float extraChance() const;
  int chain() const { return chain_; }
  void reset() { chain_ = 0; }

 private:
  Hero& hero_;
  Enemy& enemy_;
  Rng& rng_;
  int chain_ = 0;  // turnos extra consecutivos
};
