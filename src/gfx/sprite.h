#pragma once
// Hojas de sprites de 4 bpp exportadas desde Aseprite (tools/sprites).

#include <cstdint>

namespace gfx {

struct Anim {
  uint8_t from;   // primer frame
  uint8_t count;  // número de frames
  uint16_t ms;    // duración de cada frame

  // Frame de la animación tras `elapsed` ms; `loop` = repetir.
  int frameAt(uint32_t elapsed, bool loop = true) const {
    uint32_t i = elapsed / ms;
    if (loop) i %= count;
    else if (i >= count) i = count - 1;
    return from + static_cast<int>(i);
  }
  uint32_t duration() const { return static_cast<uint32_t>(count) * ms; }
};

struct Sheet {
  uint8_t w, h, frames, layers;
  uint8_t key;  // índice transparente
  const uint8_t* data;

  const uint8_t* cel(int layer, int frame) const {
    return data + (static_cast<int>(layer) * frames + frame) * (w * h / 2);
  }
};

// Tabla de recoloreado: índice del sprite → índice de la paleta.
struct Remap {
  uint8_t to[16];
  static Remap identity() {
    Remap r;
    for (int i = 0; i < 16; ++i) r.to[i] = static_cast<uint8_t>(i);
    return r;
  }
  static Remap solid(uint8_t c) {
    Remap r;
    for (int i = 0; i < 16; ++i) r.to[i] = c;
    return r;
  }
};

}  // namespace gfx
