#pragma once
// Tipos de datos del juego (GDD §5, §6, §7). Sin dependencias de Arduino.

#include <cstdint>

enum class Stat : uint8_t { ATK, DEF, CRIT, CRIT_DMG, EXTRA, LUCK, COUNT };
constexpr int STAT_COUNT = static_cast<int>(Stat::COUNT);

enum class Slot : uint8_t { CASCO, PECHERA, BOTAS, ESPADA, ESCUDO, COUNT };
constexpr int SLOT_COUNT = static_cast<int>(Slot::COUNT);

enum class Rareza : uint8_t { COMUN, RARA, EPICA, COUNT };

const char* statName(Stat s);      // "ATK", "DEF", ...
bool statIsPercent(Stat s);        // CRIT, CRIT_DMG, EXTRA → "%"
const char* slotName(Slot s);      // "Casco", ...
const char* rarezaName(Rareza r);  // "Comun", ...

struct StatMod {
  Stat stat = Stat::ATK;
  int16_t value = 0;
};

// Máximo 2 estadísticas por objeto (GDD §5.1).
constexpr int MAX_MODS = 2;

struct Item {
  bool present = false;
  Slot slot = Slot::ESPADA;
  Rareza rareza = Rareza::COMUN;
  uint8_t tier = 0;  // material: Madera, Hierro, Acero...
  uint8_t nMods = 0;
  StatMod mods[MAX_MODS];

  int get(Stat s) const {
    int v = 0;
    for (int i = 0; i < nMods; ++i)
      if (mods[i].stat == s) v += mods[i].value;
    return v;
  }
};

// Nombre visible, p. ej. "Espada de Hierro". Escribe en buf (>= 24 bytes).
void itemName(const Item& it, char* buf, int len);

// Estadísticas de combate ya calculadas (con topes aplicados).
struct Combatant {
  int atk = 1;
  int def = 0;
  int crit = 0;     // %
  int critDmg = 0;  // % extra
  int fallo = 0;    // %
};

struct Hero {
  int hp = 0;
  int hpMax = 0;
  int base[STAT_COUNT] = {};
  int fallo = 0;
  Item equip[SLOT_COUNT];

  // Suma base + equipo, sin topes.
  int raw(Stat s) const;
  // Valor efectivo con topes (GDD §5).
  int eff(Stat s) const;
  Combatant combatant() const;
};

enum class EnemyType : uint8_t { BANDIDO, GOLEM, LOBO, OGRO, JEFE, COUNT };
const char* enemyName(EnemyType t);

struct Enemy {
  EnemyType type = EnemyType::BANDIDO;
  int nivel = 1;
  int hp = 0;
  int hpMax = 0;
  Combatant c;
  int probPreparar = 0;   // % de anunciar golpe fuerte
  bool preparado = false; // el siguiente ataque es un golpe fuerte
};
