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
};

// Dibuja una capa de un frame. (x, y) = esquina superior izquierda.
template <class Canvas>
void drawCel(Canvas& cv, const Sheet& s, int layer, int frame, int x, int y,
             const Remap& remap, bool flipX = false) {
  const uint8_t* p = s.cel(layer, frame);
  for (int j = 0; j < s.h; ++j) {
    int py = y + j;
    for (int i = 0; i < s.w; i += 2) {
      uint8_t b = *p++;
      uint8_t c0 = b >> 4, c1 = b & 0x0F;
      int x0 = flipX ? x + s.w - 1 - i : x + i;
      int x1 = flipX ? x0 - 1 : x0 + 1;
      if (c0 != s.key) cv.drawPixel(x0, py, remap.to[c0]);
      if (c1 != s.key) cv.drawPixel(x1, py, remap.to[c1]);
    }
  }
}

}  // namespace gfx
