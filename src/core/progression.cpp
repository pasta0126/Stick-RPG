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
    case EnemyType::JABALI: return {0.95f, 1.10f, 0.80f, 8, 10, 30};
    case EnemyType::ARQUERO: return {0.75f, 1.10f, 0.60f, 18, 6, 15};
    case EnemyType::HOMBRE_SETA: return {1.20f, 0.85f, 1.00f, 3, 14, 20};
    case EnemyType::TRASGO: return {0.70f, 0.90f, 0.70f, 15, 6, 10};
    case EnemyType::OSO: return {1.40f, 1.20f, 0.90f, 6, 14, 35};
    case EnemyType::SERPIENTE: return {0.85f, 1.05f, 0.70f, 15, 6, 20};
    case EnemyType::ESQUELETO: return {0.90f, 1.00f, 1.00f, 8, 10, 15};
    case EnemyType::FANTASMA: return {0.80f, 1.00f, 0.50f, 15, 4, 20};
    case EnemyType::BANSHEE: return {0.80f, 1.15f, 0.60f, 20, 6, 25};
    case EnemyType::GARGOLA: return {1.10f, 0.95f, 1.60f, 5, 12, 15};
    case EnemyType::CUERVO: return {0.60f, 0.85f, 0.50f, 20, 5, 10};
    case EnemyType::GUARDIA_REAL: return {1.10f, 0.95f, 1.40f, 5, 10, 20};
    case EnemyType::ARMADURA: return {1.20f, 1.00f, 1.50f, 3, 10, 25};
    case EnemyType::VERDUGO: return {1.25f, 1.35f, 0.80f, 10, 14, 40};
    case EnemyType::MAGO: return {0.85f, 1.25f, 0.70f, 12, 8, 30};
    case EnemyType::DRAGON_JOVEN: return {1.30f, 1.20f, 1.10f, 8, 10, 35};
    // Jefes (§7.1): cada uno con su estilo.
    case EnemyType::REY_CIERVO:      return {2.00f, 1.10f, 1.20f, 8, 10, 20};
    case EnemyType::DAMA_ESPECTRAL:  return {1.50f, 1.15f, 0.90f, 25, 4, 20};
    case EnemyType::CABALLERO_NEGRO: return {1.60f, 1.10f, 1.20f, 10, 8, 25};
    case EnemyType::TEJEDORA:        return {1.70f, 1.20f, 1.00f, 15, 6, 25};
    case EnemyType::REY_LICHE:       return {2.20f, 1.25f, 1.25f, 12, 6, 35};
    case EnemyType::BRUJA_PANTANO:   return {1.70f, 1.15f, 0.90f, 12, 8, 30};
    case EnemyType::BANDIDO:
    default:               return {1.00f, 1.00f, 1.00f, 5, 10, 15};
  }
}

}  // namespace

Hero makeHero(HeroClass c) {
  Hero h;
  h.clase = c;
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
  if (c == HeroClass::VIKINGA) {
    h.base[static_cast<int>(Stat::ATK)] = bal::VIKINGA_ATK;
    h.base[static_cast<int>(Stat::DEF)] = bal::VIKINGA_DEF;
    h.base[static_cast<int>(Stat::CRIT_DMG)] = bal::VIKINGA_CRITDMG;
    h.fallo = bal::VIKINGA_FALLO;
  }
  return h;
}

bool isBossLevel(int nivel) { return nivel > 0 && nivel % bal::JEFE_CADA == 0; }

#ifdef PRUEBA_ENEMIGOS
// Modo de prueba (entorno `prueba` de platformio.ini): mismo ritmo que el
// juego (normales y jefe cada JEFE_CADA), pero los normales salen en orden y
// sin repetir hasta haberlos visto todos.
static EnemyType normalPrueba(int nivel) {
  int visto = (nivel - 1) - (nivel - 1) / bal::JEFE_CADA;  // normales previos
  return static_cast<EnemyType>(visto % NUM_NORMALES);
}
#endif

// El viaje: cada capítulo, un bioma y su jefe. Tras el último, vuelta a
// empezar (más fuertes por el nivel).
struct Capitulo {
  Bioma bioma;
  EnemyType jefe;
};
static const Capitulo kCapitulos[] = {
    {Bioma::BOSQUE, EnemyType::REY_CIERVO},
    {Bioma::PANTANO, EnemyType::BRUJA_PANTANO},
    {Bioma::CEMENTERIO, EnemyType::DAMA_ESPECTRAL},
    {Bioma::CASTILLO, EnemyType::CABALLERO_NEGRO},
    {Bioma::BOSQUE, EnemyType::TEJEDORA},
    {Bioma::CASTILLO, EnemyType::REY_LICHE},
};
constexpr int NUM_CAPITULOS = sizeof(kCapitulos) / sizeof(kCapitulos[0]);

static const Capitulo& capituloDe(int nivel) {
  return kCapitulos[((nivel - 1) / bal::JEFE_CADA) % NUM_CAPITULOS];
}

Bioma biomaDe(int nivel) { return capituloDe(nivel).bioma; }

const char* biomaName(Bioma b) {
  switch (b) {
    case Bioma::CEMENTERIO: return "El Cementerio";
    case Bioma::CASTILLO: return "El Castillo";
    case Bioma::PANTANO: return "El Pantano";
    default: return "El Bosque";
  }
}

Enemy makeEnemy(Rng& rng, int nivel) {
  Enemy e;
  e.nivel = nivel;
  if (isBossLevel(nivel)) {
    e.type = capituloDe(nivel).jefe;
  } else {
#ifdef PRUEBA_ENEMIGOS
    e.type = normalPrueba(nivel);
#else
    e.type = static_cast<EnemyType>(rng.range(0, NUM_NORMALES - 1));
#endif
  }
#ifdef PRUEBA_ENEMIGOS
  nivel = 1;  // todos con fuerza de nivel 1: se trata de verlos
#endif

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
  // Común: % de tener 1 secundaria. Rara: % de tener 2 (si no, 1).
  static const int kSecundaria[] = {50, 40, 100};

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

  // Secundarias: Común 0-1, Rara 1-2, Épica 2 (máx. 3 estadísticas).
  int nSec = 0;
  if (rareza == Rareza::COMUN) nSec = rng.chance(kSecundaria[0]) ? 1 : 0;
  else if (rareza == Rareza::RARA) nSec = rng.chance(kSecundaria[1]) ? 2 : 1;
  else nSec = 2;

  static const Stat kArma[] = {Stat::CRIT, Stat::CRIT_DMG, Stat::EXTRA,
                               Stat::LUCK, Stat::VAMP};
  static const Stat kArmadura[] = {Stat::CRIT, Stat::EXTRA, Stat::LUCK,
                                   Stat::VAMP};
  const Stat* pool = slot == Slot::ESPADA ? kArma : kArmadura;
  int poolN = slot == Slot::ESPADA ? 5 : 4;
  for (int k = 0; k < nSec; ++k) {
    Stat s;
    do {
      s = pool[rng.range(0, poolN - 1)];
    } while (it.get(s) != 0);  // sin repetir estadística
    int base;
    switch (s) {
      case Stat::CRIT: base = rng.range(1, 4); break;
      case Stat::CRIT_DMG: base = rng.range(5, 15); break;
      case Stat::EXTRA: base = rng.range(1, 4); break;
      case Stat::VAMP: base = rng.range(2, 5); break;
      default: base = rng.range(2, 6); break;  // LUCK
    }
    int sv = roundi((base + nivel / 15) * kRareza[r]);
    it.mods[it.nMods++] = {s, static_cast<int16_t>(sv < 1 ? 1 : sv)};
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

bool rollTalisman(Rng& rng, const Hero& h) {
  return !h.talisman && !h.maldito && rng.chance(bal::PROB_TALISMAN);
}

bool reviveUndead(Hero& h) {
  if (!h.talisman || h.maldito) return false;
  h.talisman = false;
  h.maldito = true;
  int hpMax = roundi(h.hpMax * bal::NO_MUERTO_HPMAX / 100.0f);
  h.hpMax = hpMax < 1 ? 1 : hpMax;
  h.hp = h.hpMax;
  h.fallo += bal::NO_MUERTO_FALLO;
  h.base[static_cast<int>(Stat::DEF)] -= bal::NO_MUERTO_DEF;
  h.golpesEspecial = 0;
  return true;
}

int healVictory(Hero& h) {
  int amount = roundi(h.hpMax * bal::CURA_VICTORIA / 100.0f);
  int before = h.hp;
  h.hp = h.hp + amount > h.hpMax ? h.hpMax : h.hp + amount;
  return h.hp - before;
}

int healBeforeBoss(Hero& h) {
  int g = roundi((h.hpMax - h.hp) * bal::CURA_JEFE / 100.0f);
  h.hp += g;
  return g;
}

int potionRecoverGain(const Hero& h) {
  int falta = h.hpMax - h.hp;
  if (falta <= 0) return 0;
  int g = roundi(falta * bal::POCION_RECUPERA / 100.0f);
  return g < 1 ? 1 : g;
}

int potionRecover(Hero& h) {
  int g = potionRecoverGain(h);
  h.hp += g;
  return g;
}

int potionMaxGain(const Hero& h) {
  int g = roundi(h.hpMax * bal::POCION_HPMAX / 100.0f);
  return g < 1 ? 1 : g;
}

int potionMaxHp(Hero& h) {
  int g = potionMaxGain(h);
  h.hpMax += g;  // solo sube el máximo; la vida actual no cambia
  return g;
}

Item equipItem(Hero& h, const Item& it) {
  Item& slot = h.equip[static_cast<int>(it.slot)];
  Item old = slot;
  slot = it;
  return old;
}
