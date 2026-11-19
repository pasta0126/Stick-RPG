#include "progression.h"

#include <cmath>

#include "balance.h"

namespace {

Item basic(Slot slot, Stat s, int v) {
  Item it;
  it.present = true;
  it.slot = slot;
  it.rareza = Rareza::COMUN;
  it.tier = 0;
  it.nMods = 1;
  it.mods[0] = {s, static_cast<int16_t>(v)};
  return it;
}

int roundi(float v) { return static_cast<int>(std::lround(v)); }

struct EnemyProfile {
  float hp, atk, def;
  int crit, fallo, preparar;
};

// Multiplicadores por tipo (§7.1).
EnemyProfile profile(EnemyType t) {
  switch (t) {
    case EnemyType::GOLEM: return {1.40f, 0.80f, 1.70f, 3, 12, 10};
    case EnemyType::LOBO:  return {0.80f, 0.95f, 0.60f, 20, 5, 10};
    case EnemyType::OGRO:  return {1.15f, 1.25f, 0.85f, 5, 14, 35};
    case EnemyType::JEFE:  return {1.90f, 1.10f, 1.15f, 10, 8, 25};
    case EnemyType::BANDIDO:
    default:               return {1.00f, 1.00f, 1.00f, 5, 10, 15};
  }
}

}  // namespace

Hero makeHero() {
  Hero h;
  h.hpMax = h.hp = bal::HEROE_HP;
  h.base[static_cast<int>(Stat::ATK)] = bal::HEROE_ATK;
  h.base[static_cast<int>(Stat::DEF)] = bal::HEROE_DEF;
  h.base[static_cast<int>(Stat::CRIT)] = bal::HEROE_CRIT;
  h.base[static_cast<int>(Stat::CRIT_DMG)] = bal::HEROE_CRITDMG;
  h.base[static_cast<int>(Stat::EXTRA)] = bal::HEROE_EXTRA;
  h.base[static_cast<int>(Stat::LUCK)] = bal::HEROE_LUCK;
  h.fallo = bal::HEROE_FALLO;
  h.equip[static_cast<int>(Slot::CASCO)] = basic(Slot::CASCO, Stat::DEF, 1);
  h.equip[static_cast<int>(Slot::PECHERA)] = basic(Slot::PECHERA, Stat::DEF, 2);
  h.equip[static_cast<int>(Slot::BOTAS)] = basic(Slot::BOTAS, Stat::DEF, 1);
  h.equip[static_cast<int>(Slot::ESPADA)] = basic(Slot::ESPADA, Stat::ATK, 2);
  // Escudo vacío (§5.1).
  return h;
}

bool isBossLevel(int nivel) { return nivel > 0 && nivel % bal::JEFE_CADA == 0; }

Enemy makeEnemy(Rng& rng, int nivel) {
  Enemy e;
  e.nivel = nivel;
  if (isBossLevel(nivel))
    e.type = EnemyType::JEFE;
  else if (nivel <= 1)
    e.type = EnemyType::BANDIDO;
  else
    e.type = static_cast<EnemyType>(rng.range(0, 3));

  EnemyProfile p = profile(e.type);
  int n = nivel - 1;
  e.hpMax = e.hp = roundi(bal::ENEMIGO_HP * p.hp * std::pow(bal::CRECE_HP, n));
  e.c.atk = roundi(bal::ENEMIGO_ATK * p.atk * std::pow(bal::CRECE_ATK, n));
  e.c.def = roundi(bal::ENEMIGO_DEF * p.def * std::pow(bal::CRECE_DEF, n));
  e.c.crit = p.crit;
  e.c.critDmg = bal::ENEMIGO_CRITDMG;
  e.c.fallo = p.fallo;
  e.probPreparar = p.preparar;
  if (e.c.atk < 1) e.c.atk = 1;
  return e;
}

Rareza rollRareza(Rng& rng, int luck, bool jefe) {
  // Cada punto de LUCK pasa 1 punto de peso de Común a Rara (0,7) y Épica (0,3).
  float comun = bal::RAREZA_COMUN - static_cast<float>(luck);
  if (comun < 10) comun = 10;
  float moved = bal::RAREZA_COMUN - comun;
  float rara = bal::RAREZA_RARA + moved * 0.7f;
  float epica = bal::RAREZA_EPICA + moved * 0.3f;
  if (jefe) comun = 0;  // un jefe deja como mínimo un objeto raro
  float r = rng.unit() * (comun + rara + epica);
  if (r < comun) return Rareza::COMUN;
  if (r < comun + rara) return Rareza::RARA;
  return Rareza::EPICA;
}

Item makeItem(Rng& rng, Slot slot, int nivel, Rareza rareza) {
  static const float kSlotFactor[SLOT_COUNT] = {0.6f, 1.0f, 0.5f, 1.6f, 0.8f};
  static const float kRareza[] = {1.0f, 1.25f, 1.55f};
  static const int kSecundaria[] = {40, 80, 100};  // % de tener 2.ª estadística

  Item it;
  it.present = true;
  it.slot = slot;
  it.rareza = rareza;
  int t = (nivel - 1) / 6;
  it.tier = static_cast<uint8_t>(t > 6 ? 6 : t);

  int r = static_cast<int>(rareza);
  Stat primary = slot == Slot::ESPADA ? Stat::ATK : Stat::DEF;
  float v = bal::BOTIN_BASE * kSlotFactor[static_cast<int>(slot)] *
            std::pow(bal::CRECE_BOTIN, nivel - 1) * kRareza[r] *
            rng.uniform(0.75f, 1.25f);
  int pv = roundi(v);
  it.mods[0] = {primary, static_cast<int16_t>(pv < 1 ? 1 : pv)};
  it.nMods = 1;

  if (rng.chance(kSecundaria[r])) {
    static const Stat kArma[] = {Stat::CRIT, Stat::CRIT_DMG, Stat::EXTRA,
                                 Stat::LUCK};
    static const Stat kArmadura[] = {Stat::CRIT, Stat::EXTRA, Stat::LUCK};
    Stat s = slot == Slot::ESPADA ? kArma[rng.range(0, 3)]
                                  : kArmadura[rng.range(0, 2)];
    int base;
    switch (s) {
      case Stat::CRIT: base = rng.range(1, 4); break;
      case Stat::CRIT_DMG: base = rng.range(5, 15); break;
      case Stat::EXTRA: base = rng.range(1, 4); break;
      default: base = rng.range(2, 6); break;  // LUCK
    }
    int sv = roundi((base + nivel / 15) * kRareza[r]);
    it.mods[1] = {s, static_cast<int16_t>(sv < 1 ? 1 : sv)};
    it.nMods = 2;
  }
  return it;
}

Loot rollLoot(Rng& rng, int nivel, int luck, bool jefe) {
  Loot l;
  if (!jefe && rng.chance(bal::PROB_POCION)) {
    l.pocion = true;
    return l;
  }
  Slot s = static_cast<Slot>(rng.range(0, SLOT_COUNT - 1));
  l.item = makeItem(rng, s, nivel, rollRareza(rng, luck, jefe));
  return l;
}

int healVictory(Hero& h) {
  int amount = roundi(h.hpMax * bal::CURA_VICTORIA / 100.0f);
  int before = h.hp;
  h.hp = h.hp + amount > h.hpMax ? h.hpMax : h.hp + amount;
  return h.hp - before;
}

int potionRecover(Hero& h) {
  int before = h.hp;
  h.hp = h.hpMax;
  return h.hp - before;
}

int potionMaxGain(const Hero& h) {
  int g = roundi(h.hpMax * bal::POCION_HPMAX / 100.0f);
  return g < 1 ? 1 : g;
}

int potionMaxHp(Hero& h) {
  int g = potionMaxGain(h);
  h.hpMax += g;
  h.hp += g;
  return g;
}

Item equipItem(Hero& h, const Item& it) {
  Item& slot = h.equip[static_cast<int>(it.slot)];
  Item old = slot;
  slot = it;
  return old;
}
