#pragma once
// Dibujo rápido sobre el búfer de un lienzo de 4 bpp (2 píxeles por byte).
// Evita las llamadas por píxel de M5GFX para poder mover varias capas de
// 240 px a 30 fps.

#include <cstdint>
#include <cstring>

#include "font5x7_gfx.h"
#include "sprite.h"

namespace gfx {

// Bayer 4x4 para tramas y disoluciones.
static const uint8_t kBayer4[4][4] = {
    {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

inline bool bayer(int x, int y, int level16) {
  return kBayer4[y & 3][x & 3] < level16;
}

class Canvas4 {
 public:
  // hiFirst: el píxel izquierdo de cada byte va en el nibble alto.
  void attach(uint8_t* buf, int w, int h, bool hiFirst) {
    buf_ = buf;
    w_ = w;
    h_ = h;
    stride_ = (w + 1) / 2;
    hiFirst_ = hiFirst;
  }
  int width() const { return w_; }
  int height() const { return h_; }

  inline void pset(int x, int y, uint8_t c) {
    if (static_cast<unsigned>(x) >= static_cast<unsigned>(w_) ||
        static_cast<unsigned>(y) >= static_cast<unsigned>(h_))
      return;
    uint8_t* p = buf_ + y * stride_ + (x >> 1);
    if (((x & 1) == 0) == hiFirst_)
      *p = static_cast<uint8_t>((*p & 0x0F) | (c << 4));
    else
      *p = static_cast<uint8_t>((*p & 0xF0) | (c & 0x0F));
  }

  void fill(uint8_t c) {
    std::memset(buf_, (c << 4) | c, stride_ * h_);
  }

  void rect(int x0, int y0, int w, int h, uint8_t c) {
    for (int y = y0; y < y0 + h; ++y)
      for (int x = x0; x < x0 + w; ++x) pset(x, y, c);
  }

  void frame(int x0, int y0, int w, int h, uint8_t c) {
    for (int x = x0; x < x0 + w; ++x) {
      pset(x, y0, c);
      pset(x, y0 + h - 1, c);
    }
    for (int y = y0; y < y0 + h; ++y) {
      pset(x0, y, c);
      pset(x0 + w - 1, y, c);
    }
  }

  // Oscurece una zona tramando con `c` (nivel 0..16).
  void shade(int x0, int y0, int w, int h, uint8_t c, int level16) {
    for (int y = y0; y < y0 + h; ++y)
      for (int x = x0; x < x0 + w; ++x)
        if (bayer(x, y, level16)) pset(x, y, c);
  }

  // Dibuja una capa de un frame. (x, y) = esquina superior izquierda.
  // dissolve 0..16: oculta píxeles con trama (desaparición).
  void blit(const Sheet& s, int layer, int frame, int x, int y,
            const Remap& remap, int dissolve = 0) {
    const uint8_t* p = s.cel(layer, frame);
    const uint8_t key2 = static_cast<uint8_t>((s.key << 4) | s.key);
    for (int j = 0; j < s.h; ++j) {
      int py = y + j;
      if (py < 0 || py >= h_) {
        p += s.w / 2;
        continue;
      }
      for (int i = 0; i < s.w; i += 2) {
        uint8_t b = *p++;
        if (b == key2) continue;
        uint8_t c0 = b >> 4, c1 = b & 0x0F;
        int px = x + i;
        if (c0 != s.key && !(dissolve && bayer(px, py, dissolve)))
          pset(px, py, remap.to[c0]);
        if (c1 != s.key && !(dissolve && bayer(px + 1, py, dissolve)))
          pset(px + 1, py, remap.to[c1]);
      }
    }
  }

  // Capa de fondo de pantalla completa repetida en horizontal.
  void blitWrap(const Sheet& s, int layer, int xoff) {
    const uint8_t* p = s.cel(layer, 0);
    const uint8_t key2 = static_cast<uint8_t>((s.key << 4) | s.key);
    int sw = s.w;
    xoff %= sw;
    if (xoff < 0) xoff += sw;
    for (int y = 0; y < s.h && y < h_; ++y) {
      const uint8_t* row = p + y * (sw / 2);
      for (int x = 0; x < w_; ++x) {
        int sx = (x + xoff) % sw;
        uint8_t b = row[sx >> 1];
        if (b == key2) {
          if ((sx & 1) == 0) ++x;  // salta también el compañero
          continue;
        }
        uint8_t c = (sx & 1) ? (b & 0x0F) : (b >> 4);
        if (c != s.key) pset(x, y, c);
      }
    }
  }

  // --------------------------------------------------------- texto 5x7
  static const Glyph5& glyph(char ch) {
    unsigned char c = static_cast<unsigned char>(ch);
    if (c >= 'a' && c <= 'z') c = static_cast<unsigned char>(c - 32);
    if (c < 32 || c > 95) c = '?';
    return kFont5[c - 32];
  }

  static int textWidth(const char* s, int scale = 1) {
    int w = 0;
    for (; *s; ++s) w += (glyph(*s).w + 1) * scale;
    return w > 0 ? w - scale : 0;
  }

  void text(const char* s, int x, int y, uint8_t color, int scale = 1) {
    for (; *s; ++s) {
      const Glyph5& g = glyph(*s);
      for (int j = 0; j < 7; ++j)
        for (int i = 0; i < g.w; ++i)
          if (g.rows[j] & (0x80 >> i))
            for (int sy = 0; sy < scale; ++sy)
              for (int sx = 0; sx < scale; ++sx)
                pset(x + i * scale + sx, y + j * scale + sy, color);
      x += (g.w + 1) * scale;
    }
  }

  // align: 0 izquierda, 1 centro, 2 derecha. outline: contorno de 1 px.
  void label(const char* s, int x, int y, uint8_t color, uint8_t outline,
             int align = 0, int scale = 1, bool fullOutline = false) {
    int w = textWidth(s, scale);
    if (align == 1) x -= w / 2;
    else if (align == 2) x -= w;
    if (fullOutline) {
      for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
          if (dx || dy) text(s, x + dx, y + dy, outline, scale);
    } else {
      text(s, x + 1, y + 1, outline, scale);
    }
    text(s, x, y, color, scale);
  }

 private:
  uint8_t* buf_ = nullptr;
  int w_ = 0, h_ = 0, stride_ = 0;
  bool hiFirst_ = true;
};

}  // namespace gfx
