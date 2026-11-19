#pragma once
// PCG32: generador pequeño, rápido y reproducible a partir de una semilla.

#include <cstdint>

class Rng {
 public:
  explicit Rng(uint64_t seed = 0x853c49e6748fea9bULL) { reseed(seed); }

  void reseed(uint64_t seed) {
    state_ = 0;
    next();
    state_ += seed;
    next();
  }

  uint32_t next() {
    uint64_t old = state_;
    state_ = old * 6364136223846793005ULL + 1442695040888963407ULL;
    uint32_t xorshifted = static_cast<uint32_t>(((old >> 18u) ^ old) >> 27u);
    uint32_t rot = static_cast<uint32_t>(old >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((32u - rot) & 31u));
  }

  // Entero en [lo, hi] (ambos incluidos).
  int range(int lo, int hi) {
    if (hi <= lo) return lo;
    uint32_t span = static_cast<uint32_t>(hi - lo + 1);
    return lo + static_cast<int>(next() % span);
  }

  // Real en [0, 1).
  float unit() { return (next() >> 8) * (1.0f / 16777216.0f); }

  float uniform(float lo, float hi) { return lo + (hi - lo) * unit(); }

  // true con probabilidad pct/100.
  bool chance(int pct) { return range(0, 99) < pct; }
  bool chancef(float p) { return unit() < p; }

 private:
  uint64_t state_;
};
