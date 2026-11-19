#include "clouds.h"

#include <cstring>

#include "palette.h"

namespace clouds {
namespace {

constexpr int W = 240;
constexpr int H = 44;
constexpr uint8_t KEY = 15;

struct LayerDef {
  int y;          // posición vertical en pantalla
  int cellX;      // tamaño de celda del ruido (divisor de 240)
  int cellY;
  float thresh;   // umbral de densidad
  uint8_t body, light, shadow;
  uint32_t msPerPx;  // velocidad
};

const LayerDef kLayers[2] = {
    {4, 40, 10, 0.63f, pal::OSCURO, pal::MEDIO, pal::NEGRO, 380},   // lejanas
    {12, 80, 12, 0.68f, pal::MEDIO, pal::CLARO, pal::OSCURO, 150},  // cercanas
};

uint8_t buf[2][W * H / 2];
float dens[H][W];

uint32_t hash(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

// Ruido de valor periódico en x (periodo = W).
float lattice(uint32_t seed, int ix, int iy, int periodX) {
  ix %= periodX;
  if (ix < 0) ix += periodX;
  return (hash(seed ^ (ix * 73856093u) ^ (iy * 19349663u)) & 0xFFFF) / 65535.0f;
}

float smooth(float t) { return t * t * (3 - 2 * t); }

float noise(uint32_t seed, float x, float y, int cellX, int cellY) {
  int periodX = W / cellX;
  float fx = x / cellX, fy = y / cellY;
  int ix = static_cast<int>(fx), iy = static_cast<int>(fy);
  float tx = smooth(fx - ix), ty = smooth(fy - iy);
  float a = lattice(seed, ix, iy, periodX), b = lattice(seed, ix + 1, iy, periodX);
  float c = lattice(seed, ix, iy + 1, periodX), d = lattice(seed, ix + 1, iy + 1, periodX);
  return (a + (b - a) * tx) + ((c + (d - c) * tx) - (a + (b - a) * tx)) * ty;
}

void put(uint8_t* b, int x, int y, uint8_t c) {
  uint8_t& p = b[y * (W / 2) + (x >> 1)];
  if ((x & 1) == 0) p = static_cast<uint8_t>((p & 0x0F) | (c << 4));
  else p = static_cast<uint8_t>((p & 0xF0) | c);
}

void build(int li, uint32_t seed) {
  const LayerDef& L = kLayers[li];
  for (int y = 0; y < H; ++y) {
    // Más densas en el centro de la franja, nada en los bordes.
    float v = 1.0f - (2.0f * y / (H - 1) - 1.0f) * (2.0f * y / (H - 1) - 1.0f);
    for (int x = 0; x < W; ++x) {
      float n = noise(seed, x, y, L.cellX, L.cellY) * 0.65f +
                noise(seed * 31 + 7, x, y, L.cellX / 2, L.cellY / 2) * 0.35f;
      dens[y][x] = n * (0.15f + 0.85f * v);
    }
  }
  uint8_t* b = buf[li];
  std::memset(b, (KEY << 4) | KEY, sizeof(buf[li]));
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      float d = dens[y][x];
      if (d >= L.thresh) {
        bool top = y < 2 || dens[y - 2][x] < L.thresh;
        bool bottom = y > H - 3 || dens[y + 2][x] < L.thresh;
        put(b, x, y, top ? L.light : (bottom ? L.shadow : L.body));
      } else if (d >= L.thresh - 0.05f && gfx::bayer(x, y, 7)) {
        put(b, x, y, L.body);  // borde deshilachado
      }
    }
}

}  // namespace

void generate(uint32_t seed) {
  build(0, seed * 2654435761u + 1);
  build(1, seed * 2246822519u + 2);
}

void draw(gfx::Canvas4& c, uint32_t timeMs) {
  for (int li = 0; li < 2; ++li) {
    const LayerDef& L = kLayers[li];
    gfx::Sheet s{W, H, 1, 1, KEY, buf[li]};
    // Se dibuja desplazando el origen vertical con un lienzo virtual.
    int off = static_cast<int>(timeMs / L.msPerPx);
    c.blitWrapAt(s, 0, off, L.y);
  }
}

}  // namespace clouds
