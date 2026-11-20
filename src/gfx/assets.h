#pragma once
// Generado por tools/art/build.py. No editar.

#include "sprite.h"
#include "heroe_gfx.h"
#include "vikinga_gfx.h"
#include "esqueleto_gfx.h"
#include "titulo_gfx.h"
#include "titulo_fondo_gfx.h"
#include "salteador_gfx.h"
#include "golem_gfx.h"
#include "huargo_gfx.h"
#include "troll_gfx.h"
#include "rey_ciervo_gfx.h"
#include "dama_espectral_gfx.h"
#include "caballero_negro_gfx.h"
#include "tejedora_gfx.h"
#include "rey_liche_gfx.h"
#include "bg_bosque_gfx.h"
#include "bg_cementerio_gfx.h"
#include "bg_castillo_gfx.h"

namespace gfx {

constexpr int NUM_BIOMAS = 3;
// Paletas de 8 colores por bioma (índices: ver src/palette.h).
static const uint8_t kBiomePalette[NUM_BIOMAS][8][3] = {
    {{0x0A, 0x0E, 0x0D}, {0x1B, 0x29, 0x25}, {0x3B, 0x52, 0x47}, {0x7A, 0x93, 0x80}, {0xD4, 0xDE, 0xC8}, {0x9C, 0x2E, 0x30}, {0xD6, 0xA2, 0x48}, {0x9F, 0xE0, 0x5A}},  // bosque
    {{0x0B, 0x0A, 0x12}, {0x22, 0x1D, 0x33}, {0x43, 0x3B, 0x5C}, {0x82, 0x7A, 0x9A}, {0xDA, 0xD4, 0xE6}, {0xA0, 0x2C, 0x3E}, {0xD8, 0xB4, 0x62}, {0x72, 0xE4, 0xD4}},  // cementerio
    {{0x0E, 0x0A, 0x0A}, {0x2A, 0x1E, 0x1D}, {0x52, 0x40, 0x3A}, {0x8E, 0x7E, 0x72}, {0xE4, 0xDA, 0xCA}, {0xB8, 0x30, 0x2A}, {0xEC, 0xA4, 0x3A}, {0xA4, 0x72, 0xE0}},  // castillo
};

static const Sheet* const kBiomeBg[NUM_BIOMAS] = {
    &bg_bosque::sheet, &bg_cementerio::sheet, &bg_castillo::sheet};
// Desplazamiento por encuentro de cada capa del fondo (parallax).
static const int16_t kParallax[6] = {0, 17, 29, 53, 89, 131};
constexpr int BG_LAYER_PERSONAJES = 5;  // se dibujan antes de esta capa

struct HeroArt {
  const Sheet* sheet;  // capas: las de heroe::Layer
  uint8_t ax, ay;      // pies
};

// Mismo orden que HeroClass (src/core/game_types.h).
static const HeroArt kHeroArt[] = {
    {&heroe::sheet, 26, 89},  // heroe
    {&vikinga::sheet, 32, 91},  // vikinga
};

// Héroe maldito (talismán). Si aún no hay arte propio es una copia del
// caballero y el juego lo tiñe.
static const HeroArt kUndeadArt = {&esqueleto::sheet, 26, 89};
constexpr bool kUndeadOwnArt = false;

struct EnemyArt {
  const Sheet* sheet;
  uint8_t ax, ay;  // pies
  uint8_t reach;   // píxeles visibles a la izquierda de los pies al caminar
  Anim walk, idle, attack, hit, prepare;
};

// Mismo orden que EnemyType (src/core/game_types.h).
static const EnemyArt kEnemyArt[] = {
    {&salteador::sheet, 56, 71, 41, salteador::WALK, salteador::IDLE, salteador::ATTACK, salteador::HIT, salteador::PREPARE},  // salteador
    {&golem::sheet, 68, 77, 56, golem::WALK, golem::IDLE, golem::ATTACK, golem::HIT, golem::PREPARE},  // golem
    {&huargo::sheet, 66, 64, 60, huargo::WALK, huargo::IDLE, huargo::ATTACK, huargo::HIT, huargo::PREPARE},  // huargo
    {&troll::sheet, 95, 91, 84, troll::WALK, troll::IDLE, troll::ATTACK, troll::HIT, troll::PREPARE},  // troll
    {&rey_ciervo::sheet, 106, 107, 64, rey_ciervo::WALK, rey_ciervo::IDLE, rey_ciervo::ATTACK, rey_ciervo::HIT, rey_ciervo::PREPARE},  // rey_ciervo
    {&dama_espectral::sheet, 118, 101, 73, dama_espectral::WALK, dama_espectral::IDLE, dama_espectral::ATTACK, dama_espectral::HIT, dama_espectral::PREPARE},  // dama_espectral
    {&caballero_negro::sheet, 119, 100, 104, caballero_negro::WALK, caballero_negro::IDLE, caballero_negro::ATTACK, caballero_negro::HIT, caballero_negro::PREPARE},  // caballero_negro
    {&tejedora::sheet, 114, 91, 87, tejedora::WALK, tejedora::IDLE, tejedora::ATTACK, tejedora::HIT, tejedora::PREPARE},  // tejedora
    {&rey_liche::sheet, 91, 107, 56, rey_liche::WALK, rey_liche::IDLE, rey_liche::ATTACK, rey_liche::HIT, rey_liche::PREPARE},  // rey_liche
};

}  // namespace gfx
