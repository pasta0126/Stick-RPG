#pragma once
// Papeles de los 8 colores de cada bioma (GDD §8.2). Los tonos reales están
// en src/gfx/assets.h (kBiomePalette), generados desde tools/art/palettes.py.

#include <cstdint>

namespace pal {

constexpr uint8_t NEGRO = 0;      // contornos, paneles
constexpr uint8_t OSCURO = 1;     // cielo, sombras, fondo de barras
constexpr uint8_t MEDIO = 2;      // bruma, malla, marcos
constexpr uint8_t CLARO = 3;      // acero, piedra, rareza Común
constexpr uint8_t BRILLO = 4;     // texto, luna, brillos
constexpr uint8_t SANGRE = 5;     // vida, daño
constexpr uint8_t ORO = 6;        // crítico, selección, rareza Épica
constexpr uint8_t ESPECTRAL = 7;  // magia del bioma, runas, rareza Rara

}  // namespace pal
