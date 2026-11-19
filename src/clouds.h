#pragma once
// Nubes procedurales: dos capas generadas con ruido periódico (se repiten
// sin costura cada 240 px) que se desplazan despacio a distinta velocidad.

#include <cstdint>

#include "gfx/canvas4.h"

namespace clouds {

// Genera nubes nuevas para el bioma (semilla distinta en cada llamada).
void generate(uint32_t seed);
// Dibuja las dos capas según el tiempo de juego (ms).
void draw(gfx::Canvas4& c, uint32_t timeMs);

}  // namespace clouds
