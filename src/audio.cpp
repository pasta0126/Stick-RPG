#include "audio.h"

#include <M5Unified.h>

namespace audio {
namespace {

bool on = true;
const Note* seq = nullptr;
int seqLen = 0, seqPos = 0;
uint32_t nextAt = 0;

// Notas (Hz).
constexpr uint16_t E3 = 165, A3 = 220, AS3 = 233, B3 = 247, C4 = 262,
                   D4 = 294, E4 = 330, F4 = 349, G4 = 392, A4 = 440,
                   B4 = 494, C5 = 523, D5 = 587, E5 = 659, F5 = 698,
                   G5 = 784, A5 = 880, C6 = 1047;

// Fanfarria corta en La menor (inicio).
const Note kInicio[] = {{A4, 110}, {C5, 110}, {E5, 110}, {A5, 260}, {0, 40},
                        {G5, 110}, {E5, 110}, {F5, 140}, {E5, 140}, {D5, 140},
                        {E5, 380}};
// Tritono amenazante (jefe).
const Note kJefe[] = {{E4, 160}, {AS3, 160}, {E4, 160}, {AS3, 420}};
const Note kVictoria[] = {{C5, 80}, {E5, 80}, {G5, 80}, {C6, 220}};
const Note kBioma[] = {{A3, 140}, {E4, 140}, {A4, 140}, {B4, 140}, {C5, 360}};
const Note kDerrota[] = {{A4, 220}, {F4, 220}, {D4, 220}, {A3, 220}, {E3, 600}};

}  // namespace

void begin(bool enabled) {
  on = enabled;
  M5.Speaker.setVolume(110);
}

void setEnabled(bool v) {
  on = v;
  if (!on) {
    M5.Speaker.stop();
    seq = nullptr;
  }
}

bool enabled() { return on; }

void play(const Note* notes, int count) {
  if (!on) return;
  seq = notes;
  seqLen = count;
  seqPos = 0;
  nextAt = 0;
}

void sfx(uint16_t freq, uint16_t ms, bool force) {
  if (!on) return;
  if (seq && !force) return;
  if (force) seq = nullptr;
  M5.Speaker.tone(freq, ms);
}

void update() {
  if (!seq) return;
  uint32_t now = millis();
  if (now < nextAt) return;
  if (seqPos >= seqLen) {
    seq = nullptr;
    return;
  }
  const Note& n = seq[seqPos++];
  if (n.freq) M5.Speaker.tone(n.freq, n.ms > 15 ? n.ms - 10 : n.ms);
  nextAt = now + n.ms;
}

void jingleInicio() { play(kInicio, sizeof(kInicio) / sizeof(Note)); }
void jingleJefe() { play(kJefe, sizeof(kJefe) / sizeof(Note)); }
void jingleVictoria() { play(kVictoria, sizeof(kVictoria) / sizeof(Note)); }
void jingleBioma() { play(kBioma, sizeof(kBioma) / sizeof(Note)); }
void jingleDerrota() { play(kDerrota, sizeof(kDerrota) / sizeof(Note)); }

}  // namespace audio
