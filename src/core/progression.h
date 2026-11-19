#pragma once
// Héroe inicial, enemigos, botín y curación (GDD §5–§7).

#include "game_types.h"
#include "rng.h"

Hero makeHero();

bool isBossLevel(int nivel);

// Biomas (GDD §7.2): cambian tras cada jefe y se repiten en ciclo.
enum class Bioma : uint8_t { BOSQUE, CEMENTERIO, CASTILLO, COUNT };
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
int potionRecover(Hero& h);    // §6.4 RECUPERAR
int potionMaxGain(const Hero& h);
int potionMaxHp(Hero& h);      // §6.4 VIDA MÁX.

// Equipa el objeto y devuelve el anterior.
Item equipItem(Hero& h, const Item& it);
