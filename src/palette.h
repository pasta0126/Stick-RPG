#pragma once
// Paleta global de 16 colores (GDD §8.2). Los índices se usan directamente
// como color al dibujar en el lienzo de 4 bpp.

#include <cstdint>

namespace pal {

constexpr uint8_t NEGRO = 0;      // contornos, fondo de paneles
constexpr uint8_t NOCHE = 1;      // cielo / fondo HUD
constexpr uint8_t PIZARRA = 2;    // montañas lejanas
constexpr uint8_t GRIS = 3;       // rocas, golem, texto desactivado
constexpr uint8_t GRIS_CLARO = 4; // texto secundario, rareza común
constexpr uint8_t BLANCO = 5;     // texto principal
constexpr uint8_t MARRON_OSC = 6; // tierra
constexpr uint8_t MARRON = 7;     // madera, lobo
constexpr uint8_t PIEL = 8;       // piel del héroe
constexpr uint8_t ROJO = 9;       // daño, ▼, vida enemiga
constexpr uint8_t NARANJA = 10;   // aviso de golpe fuerte
constexpr uint8_t AMARILLO = 11;  // crítico, zona perfecta, selección
constexpr uint8_t VERDE = 12;     // curación, ▲, vida del héroe
constexpr uint8_t VERDE_OSC = 13; // vegetación, ogro
constexpr uint8_t AZUL = 14;      // rareza rara, turno extra
constexpr uint8_t MORADO = 15;    // rareza épica, jefe

struct Rgb { uint8_t r, g, b; };

constexpr Rgb kColores[16] = {
    {0x0B, 0x0B, 0x14}, {0x1E, 0x21, 0x40}, {0x33, 0x40, 0x6B},
    {0x5B, 0x6B, 0x8C}, {0xA6, 0xAE, 0xBF}, {0xF2, 0xF2, 0xEB},
    {0x4A, 0x2E, 0x22}, {0x8C, 0x5A, 0x3C}, {0xE8, 0xB5, 0x8A},
    {0xC2, 0x3B, 0x3B}, {0xF0, 0x8A, 0x3C}, {0xF7, 0xD0, 0x46},
    {0x4C, 0xB0, 0x50}, {0x2A, 0x6B, 0x4F}, {0x4A, 0xA3, 0xF0},
    {0x9A, 0x5B, 0xD6},
};

}  // namespace pal
