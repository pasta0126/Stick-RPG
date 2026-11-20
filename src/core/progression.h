#pragma once
// Héroe inicial, enemigos, botín y curación (GDD §5–§7).

#include "game_types.h"
#include "rng.h"

Hero makeHero(HeroClass c = HeroClass::CABALLERO);

bool isBossLevel(int nivel);

// Biomas: cada capítulo (JEFE_CADA encuentros) es un bioma con su jefe, en el
// orden fijo de kCapitulos (progression.cpp), y vuelta a empezar.
enum class Bioma : uint8_t { BOSQUE, CEMENTERIO, CASTILLO, PANTANO, COUNT };
Bioma biomaDe(int nivel);
const char* biomaName(Bioma b);
Enemy makeEnemy(Rng& rng, int nivel);

struct Loot {
  bool pocion = false;
  Item item;
};

Rareza rollRareza(Rng& rng, int luck, bool jefe);
Item makeItem(Rng& rng, Slot slot, int nivel, Rareza rareza);
Loot rollLoot(Rng& rng, int nivel, int luck, bool jefe);

// Devuelven los HP realmente recuperados / ganados.
int healVictory(Hero& h);      // §6.1
int healBeforeBoss(Hero& h);   // §6.1: al aparecer un jefe
int potionRecoverGain(const Hero& h);
int potionRecover(Hero& h);    // §6.4 RECUPERAR
int potionMaxGain(const Hero& h);
int potionMaxHp(Hero& h);      // §6.4 VIDA MÁX.

// Talismán de los no muertos: tirada al vencer a un jefe (solo si no lo
// tiene ni está ya maldito) y conversión en esqueleto al morir con él.
bool rollTalisman(Rng& rng, const Hero& h);
bool reviveUndead(Hero& h);  // false si no tenía talismán

// Equipa el objeto y devuelve el anterior.
Item equipItem(Hero& h, const Item& it);
