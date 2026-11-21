#pragma once
// Tipos de datos del juego (GDD §5, §6, §7). Sin dependencias de Arduino.

#include <cstdint>

enum class Stat : uint8_t { ATK, DEF, CRIT, CRIT_DMG, EXTRA, LUCK, VAMP, COUNT };
constexpr int STAT_COUNT = static_cast<int>(Stat::COUNT);

enum class Slot : uint8_t { CASCO, PECHERA, BOTAS, ESPADA, ESCUDO, COUNT };

// Héroes jugables (se elige al empezar). Mismo orden que kHeroArt.
enum class HeroClass : uint8_t { CABALLERO, VIKINGA, SAMURAI, HECHICERA, PICARA,
                                 DRUIDA, COUNT };
constexpr int HERO_CLASS_COUNT = static_cast<int>(HeroClass::COUNT);

// Ficha de cada héroe: nombres, estadísticas iniciales y su mecánica propia.
struct HeroClassInfo {
  const char* name;     // "Caballero", ... (sin tildes: fuente ASCII)
  const char* weapon;   // nombre del arma en el botín ("Espada", "Katana", ...)
  const char* offhand;  // nombre de la pieza de la mano trasera ("Escudo", ...)
  int hp, atk, def, crit, critDmg, luck, fallo;
  int especialCada;     // golpes normales para habilitar la carga
  float zonaPerfecta;   // inicio de la zona perfecta de la barra (0..1)
  int regen;            // % de vida máx. recuperado en cada ataque propio
  int probPocion;       // % de que el botín sea una poción
};
const HeroClassInfo& heroInfo(HeroClass c);
const char* heroClassName(HeroClass c);
// El arma y la mano trasera se llaman distinto según el héroe de la partida.
void setSlotNames(HeroClass c);
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

// Máximo 3 estadísticas por objeto (GDD §5.1): 1 principal + 2 secundarias.
constexpr int MAX_MODS = 3;

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
  HeroClass clase = HeroClass::CABALLERO;
  int hp = 0;
  int hpMax = 0;
  int base[STAT_COUNT] = {};
  int fallo = 0;
  int golpesEspecial = 0;  // golpes acumulados para el ataque especial (§4.3)
  Item equip[SLOT_COUNT];

  bool especialListo() const;
  const HeroClassInfo& info() const { return heroInfo(clase); }

  // Suma base + equipo, sin topes.
  int raw(Stat s) const;
  // Valor efectivo con topes (GDD §5).
  int eff(Stat s) const;
  Combatant combatant() const;
};

// Enemigos normales (aleatorios) y jefes únicos: un capítulo = 5 normales +
// el jefe de su bioma (ver kCapitulos en progression.cpp).
enum class EnemyType : uint8_t {
  BANDIDO, GOLEM, LOBO, OGRO,
  // Lote 2 (arte de IA): salen al azar en cualquier bioma, como los cuatro
  // primeros.
  JABALI,
  ARQUERO,
  HOMBRE_SETA,
  TRASGO,
  OSO,
  SERPIENTE,
  ESQUELETO,
  FANTASMA,
  BANSHEE,
  GARGOLA,
  CUERVO,
  GUARDIA_REAL,
  ARMADURA,
  VERDUGO,
  MAGO,
  DRAGON_JOVEN,
  REY_CIERVO, DAMA_ESPECTRAL, CABALLERO_NEGRO, TEJEDORA,
  BRUJA_PANTANO, SENOR_TRASGOS, YETI, GUARDIAN_CRIPTA, DRAGON_ANCESTRAL,
  PRIMIGENIO,
  COUNT
};
constexpr int NUM_NORMALES = 20;
constexpr int NUM_JEFES = 10;
const char* enemyName(EnemyType t);
inline bool isBoss(EnemyType t) {
  return static_cast<int>(t) >= NUM_NORMALES && t != EnemyType::COUNT;
}

struct Enemy {
  EnemyType type = EnemyType::BANDIDO;
  int nivel = 1;
  int hp = 0;
  int hpMax = 0;
  Combatant c;
  int probPreparar = 0;   // % de anunciar golpe fuerte
  bool preparado = false; // el siguiente ataque es un golpe fuerte
};
