#pragma once
// Sonido con el zumbador: efectos cortos y jingles secuenciados sin bloquear.

#include <cstdint>

namespace audio {

struct Note {
  uint16_t freq;  // 0 = silencio
  uint16_t ms;
};

void begin(bool enabled);
void setEnabled(bool on);
bool enabled();
void update();  // llamar en cada frame

// Efecto corto; no interrumpe un jingle en curso salvo `force`.
void sfx(uint16_t freq, uint16_t ms, bool force = false);
void play(const Note* notes, int count);

// Jingles y efectos del juego.
void jingleInicio();
void jingleJefe();
void jingleVictoria();
void jingleBioma();
void jingleDerrota();

}  // namespace audio
