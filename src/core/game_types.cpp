#include "game_types.h"

#include <cstdio>

#include "balance.h"

// Los textos van sin tildes: la fuente del dispositivo es ASCII.

const char* statName(Stat s) {
  switch (s) {
    case Stat::ATK: return "ATK";
    case Stat::DEF: return "DEF";
    case Stat::CRIT: return "CRIT";
    case Stat::CRIT_DMG: return "C.DMG";
    case Stat::EXTRA: return "EXTRA";
    case Stat::LUCK: return "LUCK";
    case Stat::VAMP: return "VAMP";
    default: return "?";
  }
}

bool statIsPercent(Stat s) {
  return s == Stat::CRIT || s == Stat::CRIT_DMG || s == Stat::EXTRA ||
         s == Stat::VAMP;
}

namespace {
const char* gWeaponName = "Espada";
}

void setWeaponName(const char* name) { gWeaponName = name; }

const char* heroClassName(HeroClass c) {
  return c == HeroClass::VIKINGA ? "Vikinga" : "Caballero";
}

const char* slotName(Slot s) {
  switch (s) {
    case Slot::CASCO: return "Casco";
    case Slot::PECHERA: return "Pechera";
    case Slot::BOTAS: return "Botas";
    case Slot::ESPADA: return gWeaponName;
    case Slot::ESCUDO: return "Escudo";
    default: return "?";
  }
}

const char* rarezaName(Rareza r) {
  switch (r) {
    case Rareza::COMUN: return "Comun";
    case Rareza::RARA: return "Rara";
    case Rareza::EPICA: return "Epica";
    default: return "?";
  }
}

static const char* const kMateriales[] = {"Madera", "Hierro", "Acero",
                                          "Plata",  "Mitril", "Runico",
                                          "Astral"};
constexpr int kNumMateriales = sizeof(kMateriales) / sizeof(kMateriales[0]);

void itemName(const Item& it, char* buf, int len) {
  if (!it.present) {
    std::snprintf(buf, len, "(vacio)");
    return;
  }
  int t = it.tier < kNumMateriales ? it.tier : kNumMateriales - 1;
  std::snprintf(buf, len, "%s de %s", slotName(it.slot), kMateriales[t]);
}

const char* enemyName(EnemyType t) {
  switch (t) {
    case EnemyType::BANDIDO: return "Salteador";
    case EnemyType::GOLEM: return "Golem";
    case EnemyType::LOBO: return "Huargo";
    case EnemyType::OGRO: return "Troll";
    case EnemyType::JABALI: return "Jabali";
    case EnemyType::ARQUERO: return "Arquero";
    case EnemyType::HOMBRE_SETA: return "Hombre Seta";
    case EnemyType::AVISPA: return "Avispa Reina";
    case EnemyType::ENT: return "Ent del Roble";
    case EnemyType::TRASGO: return "Trasgo";
    case EnemyType::OSO: return "Oso Pardo";
    case EnemyType::DRUIDA: return "Druida Corrupto";
    case EnemyType::SERPIENTE: return "Serpiente";
    case EnemyType::ESQUELETO: return "Esqueleto";
    case EnemyType::ZOMBI: return "Zombi";
    case EnemyType::FANTASMA: return "Fantasma";
    case EnemyType::SEPULTURERO: return "Sepulturero";
    case EnemyType::BANSHEE: return "Banshee";
    case EnemyType::GARGOLA: return "Gargola";
    case EnemyType::MOMIA: return "Momia";
    case EnemyType::CUERVO: return "Cuervo";
    case EnemyType::GUARDIA_REAL: return "Guardia Real";
    case EnemyType::ARMADURA: return "Armadura Viva";
    case EnemyType::VERDUGO: return "Verdugo";
    case EnemyType::MAGO: return "Mago Oscuro";
    case EnemyType::DRAGON_JOVEN: return "Dragon Joven";
    case EnemyType::DEMONIO: return "Demonio";
    case EnemyType::SUCUBO: return "Sucubo";
    case EnemyType::REY_CIERVO: return "Rey Ciervo";
    case EnemyType::DAMA_ESPECTRAL: return "Dama Espectral";
    case EnemyType::CABALLERO_NEGRO: return "Caballero Negro";
    case EnemyType::TEJEDORA: return "La Tejedora";
    case EnemyType::REY_LICHE: return "Rey Liche";
    default: return "?";
  }
}

int Hero::raw(Stat s) const {
  int v = base[static_cast<int>(s)];
  for (const Item& it : equip)
    if (it.present) v += it.get(s);
  return v;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

int Hero::eff(Stat s) const {
  int v = raw(s);
  switch (s) {
    case Stat::ATK: return v < 1 ? 1 : v;
    case Stat::CRIT: return clampi(v, 0, bal::TOPE_CRIT);
    case Stat::CRIT_DMG: return clampi(v, 0, bal::TOPE_CRIT_DMG);
    case Stat::EXTRA: return clampi(v, 0, bal::TOPE_EXTRA);
    case Stat::VAMP: return clampi(v, 0, bal::TOPE_VAMP);
    default: return v < 0 ? 0 : v;
  }
}

bool Hero::especialListo() const { return golpesEspecial >= bal::ESPECIAL_CADA; }

Combatant Hero::combatant() const {
  Combatant c;
  c.atk = eff(Stat::ATK);
  c.def = eff(Stat::DEF);
  c.crit = eff(Stat::CRIT);
  c.critDmg = eff(Stat::CRIT_DMG);
  c.fallo = fallo < bal::TOPE_FALLO_MIN ? bal::TOPE_FALLO_MIN : fallo;
  return c;
}
