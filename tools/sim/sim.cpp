// Simulador de balance: juega miles de partidas con un jugador automático y
// muestra cuántos enemigos se derrotan. Uso:
//   ./build/sim [--games N] [--seed S] [--skill P] [--policy charge|normal]
//               [--verbose]
//   skill = probabilidad de soltar la carga en la zona perfecta (0..1).

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "../../src/core/balance.h"
#include "../../src/core/combat.h"
#include "../../src/core/progression.h"

namespace {

struct Options {
  int games = 5000;
  uint64_t seed = 1;
  float skill = 0.5f;
  bool charge = true;
  bool verbose = false;
};

float itemScore(const Item& it) {
  if (!it.present) return 0;
  return it.get(Stat::ATK) * 1.2f + it.get(Stat::DEF) * 1.0f +
         it.get(Stat::CRIT) * 0.4f + it.get(Stat::CRIT_DMG) * 0.1f +
         it.get(Stat::EXTRA) * 0.6f + it.get(Stat::LUCK) * 0.15f +
         it.get(Stat::VAMP) * 0.5f;
}

float playerCharge(Rng& rng, const Options& o) {
  if (!o.charge) return 1.0f;
  if (rng.chancef(o.skill)) return chargeMultiplier(rng.uniform(bal::ZONA_PERFECTA, 1.0f));
  return chargeMultiplier(rng.uniform(0.0f, bal::ZONA_PERFECTA));
}

struct RunStats {
  int defeated = 0;
  int turns = 0;
  bool diedAtBoss = false;
  int maxHit = 0;
  int hpMaxEnd = 0;
};

RunStats playRun(Rng& rng, const Options& o) {
  RunStats st;
  Hero hero = makeHero();
  for (int nivel = 1;; ++nivel) {
    Enemy enemy = makeEnemy(rng, nivel);
    if (isBossLevel(nivel)) healBeforeBoss(hero);
    Battle b(hero, enemy, rng);
    bool heroDead = false;
    while (true) {
      HeroTurnResult hr = (o.charge && hero.especialListo())
                              ? b.heroSpecial(playerCharge(rng, o))
                              : b.heroAttack();
      ++st.turns;
      st.maxHit = std::max(st.maxHit, hr.atk.damage);
      if (hr.enemyDead) break;
      if (hr.extraTurn) continue;
      EnemyTurnResult er = b.enemyTurn();
      if (er.heroDead) { heroDead = true; break; }
    }
    if (heroDead) {
      st.diedAtBoss = isBossLevel(nivel);
      st.hpMaxEnd = hero.hpMax;
      if (o.verbose)
        std::printf("  muere en %d (%s) ATK %d DEF %d HPmax %d\n", nivel,
                    enemyName(enemy.type), hero.eff(Stat::ATK),
                    hero.eff(Stat::DEF), hero.hpMax);
      return st;
    }
    ++st.defeated;
    healVictory(hero);
    Loot loot = rollLoot(rng, nivel, hero.eff(Stat::LUCK), isBossLevel(nivel));
    if (loot.pocion) {
      if (hero.hp * 2 < hero.hpMax) potionRecover(hero);
      else potionMaxHp(hero);
    } else {
      const Item& cur = hero.equip[static_cast<int>(loot.item.slot)];
      if (itemScore(loot.item) > itemScore(cur)) equipItem(hero, loot.item);
    }
    if (nivel > 2000) return st;  // seguridad
  }
}

int pct(const std::vector<int>& v, float p) {
  return v[static_cast<size_t>(p * (v.size() - 1))];
}

}  // namespace

int main(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    auto arg = [&](const char* n) { return std::strcmp(argv[i], n) == 0 && i + 1 < argc; };
    if (arg("--games")) o.games = std::atoi(argv[++i]);
    else if (arg("--seed")) o.seed = std::strtoull(argv[++i], nullptr, 10);
    else if (arg("--skill")) o.skill = static_cast<float>(std::atof(argv[++i]));
    else if (arg("--policy")) o.charge = std::strcmp(argv[++i], "normal") != 0;
    else if (std::strcmp(argv[i], "--verbose") == 0) o.verbose = true;
    else { std::fprintf(stderr, "argumento desconocido: %s\n", argv[i]); return 1; }
  }

  Rng rng(o.seed);
  std::vector<int> defeated;
  long turns = 0, combats = 0, bossDeaths = 0, maxHit = 0, hpMax = 0;
  for (int g = 0; g < o.games; ++g) {
    RunStats st = playRun(rng, o);
    defeated.push_back(st.defeated);
    turns += st.turns;
    combats += st.defeated + 1;
    bossDeaths += st.diedAtBoss;
    maxHit += st.maxHit;
    hpMax += st.hpMaxEnd;
  }
  std::sort(defeated.begin(), defeated.end());
  double mean = 0;
  for (int d : defeated) mean += d;
  mean /= defeated.size();

  std::printf("Partidas: %d  politica: %s  skill: %.2f  semilla: %llu\n",
              o.games, o.charge ? "carga" : "normal", o.skill,
              static_cast<unsigned long long>(o.seed));
  std::printf("Enemigos derrotados  media %.1f | p10 %d | p50 %d | p90 %d | max %d\n",
              mean, pct(defeated, 0.1f), pct(defeated, 0.5f),
              pct(defeated, 0.9f), defeated.back());
  std::printf("Turnos del heroe por combate: %.2f\n",
              static_cast<double>(turns) / combats);
  std::printf("Muertes ante jefe: %.1f %%\n", 100.0 * bossDeaths / o.games);
  std::printf("Golpe maximo medio: %.1f   Vida max. final media: %.1f\n",
              static_cast<double>(maxHit) / o.games,
              static_cast<double>(hpMax) / o.games);

  std::printf("Histograma (bloques de 5):\n");
  int top = defeated.back();
  for (int lo = 0; lo <= top; lo += 5) {
    int n = 0;
    for (int d : defeated) n += (d >= lo && d < lo + 5);
    if (!n) continue;
    int bar = static_cast<int>(60.0 * n / o.games + 0.5);
    std::printf("  %3d-%3d %5.1f%% %s\n", lo, lo + 4, 100.0 * n / o.games,
                std::string(bar, '#').c_str());
  }
  return 0;
}
