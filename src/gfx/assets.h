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
#include "jabali_gfx.h"
#include "arquero_gfx.h"
#include "hombre_seta_gfx.h"
#include "trasgo_gfx.h"
#include "oso_gfx.h"
#include "serpiente_gfx.h"
#include "esqueleto_guerrero_gfx.h"
#include "fantasma_gfx.h"
#include "banshee_gfx.h"
#include "gargola_gfx.h"
#include "cuervo_gfx.h"
#include "guardia_real_gfx.h"
#include "armadura_animada_gfx.h"
#include "verdugo_gfx.h"
#include "mago_torre_gfx.h"
#include "dragon_joven_gfx.h"
#include "rey_ciervo_gfx.h"
#include "dama_espectral_gfx.h"
#include "caballero_negro_gfx.h"
#include "tejedora_gfx.h"
#include "rey_liche_gfx.h"
#include "bruja_pantano_gfx.h"
#include "senor_trasgos_gfx.h"
#include "yeti_gfx.h"
#include "bg_bosque_gfx.h"
#include "bg_cementerio_gfx.h"
#include "bg_castillo_gfx.h"
#include "bg_pantano_gfx.h"
#include "bg_cueva_gfx.h"
#include "bg_montana_gfx.h"

namespace gfx {

constexpr int NUM_BIOMAS = 6;
// Paletas de 8 colores por bioma (índices: ver src/palette.h).
static const uint8_t kBiomePalette[NUM_BIOMAS][8][3] = {
    {{0x0A, 0x0E, 0x0D}, {0x1B, 0x29, 0x25}, {0x3B, 0x52, 0x47}, {0x7A, 0x93, 0x80}, {0xD4, 0xDE, 0xC8}, {0x9C, 0x2E, 0x30}, {0xD6, 0xA2, 0x48}, {0x9F, 0xE0, 0x5A}},  // bosque
    {{0x0B, 0x0A, 0x12}, {0x22, 0x1D, 0x33}, {0x43, 0x3B, 0x5C}, {0x82, 0x7A, 0x9A}, {0xDA, 0xD4, 0xE6}, {0xA0, 0x2C, 0x3E}, {0xD8, 0xB4, 0x62}, {0x72, 0xE4, 0xD4}},  // cementerio
    {{0x0E, 0x0A, 0x0A}, {0x2A, 0x1E, 0x1D}, {0x52, 0x40, 0x3A}, {0x8E, 0x7E, 0x72}, {0xE4, 0xDA, 0xCA}, {0xB8, 0x30, 0x2A}, {0xEC, 0xA4, 0x3A}, {0xA4, 0x72, 0xE0}},  // castillo
    {{0x0A, 0x0F, 0x08}, {0x1A, 0x2C, 0x14}, {0x36, 0x56, 0x2A}, {0x7A, 0xA0, 0x4C}, {0xDD, 0xED, 0xB0}, {0x96, 0x2E, 0x2A}, {0xD0, 0xA0, 0x40}, {0xC8, 0xF0, 0x40}},  // pantano
    {{0x0E, 0x0A, 0x06}, {0x2E, 0x20, 0x14}, {0x5E, 0x40, 0x24}, {0xA0, 0x78, 0x48}, {0xEE, 0xD8, 0xB0}, {0xA8, 0x34, 0x2C}, {0xE0, 0xA8, 0x3C}, {0x5E, 0xD0, 0xE8}},  // cueva
    {{0x0A, 0x0D, 0x14}, {0x1C, 0x26, 0x38}, {0x3E, 0x51, 0x70}, {0x8A, 0xA0, 0xBC}, {0xE8, 0xF0, 0xF8}, {0xA0, 0x30, 0x3A}, {0xD8, 0xB0, 0x60}, {0x7C, 0xF0, 0xC0}},  // montana
};

static const Sheet* const kBiomeBg[NUM_BIOMAS] = {
    &bg_bosque::sheet, &bg_cementerio::sheet, &bg_castillo::sheet, &bg_pantano::sheet, &bg_cueva::sheet, &bg_montana::sheet};
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
static const HeroArt kUndeadArt = {&esqueleto::sheet, 43, 96};
constexpr bool kUndeadOwnArt = true;

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
    {&jabali::sheet, 49, 59, 44, jabali::WALK, jabali::IDLE, jabali::ATTACK, jabali::HIT, jabali::PREPARE},  // jabali
    {&arquero::sheet, 38, 74, 29, arquero::WALK, arquero::IDLE, arquero::ATTACK, arquero::HIT, arquero::PREPARE},  // arquero
    {&hombre_seta::sheet, 31, 69, 29, hombre_seta::WALK, hombre_seta::IDLE, hombre_seta::ATTACK, hombre_seta::HIT, hombre_seta::PREPARE},  // hombre_seta
    {&trasgo::sheet, 41, 65, 29, trasgo::WALK, trasgo::IDLE, trasgo::ATTACK, trasgo::HIT, trasgo::PREPARE},  // trasgo
    {&oso::sheet, 47, 90, 36, oso::WALK, oso::IDLE, oso::ATTACK, oso::HIT, oso::PREPARE},  // oso
    {&serpiente::sheet, 31, 68, 23, serpiente::WALK, serpiente::IDLE, serpiente::ATTACK, serpiente::HIT, serpiente::PREPARE},  // serpiente
    {&esqueleto_guerrero::sheet, 56, 89, 35, esqueleto_guerrero::WALK, esqueleto_guerrero::IDLE, esqueleto_guerrero::ATTACK, esqueleto_guerrero::HIT, esqueleto_guerrero::PREPARE},  // esqueleto_guerrero
    {&fantasma::sheet, 42, 77, 29, fantasma::WALK, fantasma::IDLE, fantasma::ATTACK, fantasma::HIT, fantasma::PREPARE},  // fantasma
    {&banshee::sheet, 44, 87, 29, banshee::WALK, banshee::IDLE, banshee::ATTACK, banshee::HIT, banshee::PREPARE},  // banshee
    {&gargola::sheet, 42, 84, 36, gargola::WALK, gargola::IDLE, gargola::ATTACK, gargola::HIT, gargola::PREPARE},  // gargola
    {&cuervo::sheet, 41, 67, 27, cuervo::WALK, cuervo::IDLE, cuervo::ATTACK, cuervo::HIT, cuervo::PREPARE},  // cuervo
    {&guardia_real::sheet, 37, 76, 33, guardia_real::WALK, guardia_real::IDLE, guardia_real::ATTACK, guardia_real::HIT, guardia_real::PREPARE},  // guardia_real
    {&armadura_animada::sheet, 48, 89, 39, armadura_animada::WALK, armadura_animada::IDLE, armadura_animada::ATTACK, armadura_animada::HIT, armadura_animada::PREPARE},  // armadura_animada
    {&verdugo::sheet, 58, 102, 41, verdugo::WALK, verdugo::IDLE, verdugo::ATTACK, verdugo::HIT, verdugo::PREPARE},  // verdugo
    {&mago_torre::sheet, 63, 91, 31, mago_torre::WALK, mago_torre::IDLE, mago_torre::ATTACK, mago_torre::HIT, mago_torre::PREPARE},  // mago_torre
    {&dragon_joven::sheet, 47, 81, 42, dragon_joven::WALK, dragon_joven::IDLE, dragon_joven::ATTACK, dragon_joven::HIT, dragon_joven::PREPARE},  // dragon_joven
    {&rey_ciervo::sheet, 106, 107, 64, rey_ciervo::WALK, rey_ciervo::IDLE, rey_ciervo::ATTACK, rey_ciervo::HIT, rey_ciervo::PREPARE},  // rey_ciervo
    {&dama_espectral::sheet, 118, 101, 73, dama_espectral::WALK, dama_espectral::IDLE, dama_espectral::ATTACK, dama_espectral::HIT, dama_espectral::PREPARE},  // dama_espectral
    {&caballero_negro::sheet, 119, 100, 104, caballero_negro::WALK, caballero_negro::IDLE, caballero_negro::ATTACK, caballero_negro::HIT, caballero_negro::PREPARE},  // caballero_negro
    {&tejedora::sheet, 114, 91, 87, tejedora::WALK, tejedora::IDLE, tejedora::ATTACK, tejedora::HIT, tejedora::PREPARE},  // tejedora
    {&rey_liche::sheet, 91, 107, 56, rey_liche::WALK, rey_liche::IDLE, rey_liche::ATTACK, rey_liche::HIT, rey_liche::PREPARE},  // rey_liche
    {&bruja_pantano::sheet, 51, 102, 40, bruja_pantano::WALK, bruja_pantano::IDLE, bruja_pantano::ATTACK, bruja_pantano::HIT, bruja_pantano::PREPARE},  // bruja_pantano
    {&senor_trasgos::sheet, 71, 121, 53, senor_trasgos::WALK, senor_trasgos::IDLE, senor_trasgos::ATTACK, senor_trasgos::HIT, senor_trasgos::PREPARE},  // senor_trasgos
    {&yeti::sheet, 82, 121, 60, yeti::WALK, yeti::IDLE, yeti::ATTACK, yeti::HIT, yeti::PREPARE},  // yeti
};

}  // namespace gfx
