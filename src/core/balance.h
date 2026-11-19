#pragma once
// Parámetros de balance (GDD §10). Todos los valores son iniciales y se
// ajustan con el simulador (tools/sim).

#include <cstdint>

namespace bal {

// --- Entrada (§4.3) ---------------------------------------------------------
constexpr uint32_t T_UMBRAL_MS      = 200;   // toque vs. mantener
constexpr uint32_t CICLO_CARGA_MS   = 1100;  // 0 → 100 % de la barra

// --- Ataque cargado (§4.3) --------------------------------------------------
constexpr float CARGA_MIN       = 0.5f;
constexpr float CARGA_MAX       = 1.8f;
constexpr float ZONA_PERFECTA   = 0.92f;
constexpr float CARGA_PERFECTA  = 2.2f;

// --- Topes (§5) ---------------------------------------------------------------
constexpr int TOPE_CRIT      = 50;   // %
constexpr int TOPE_CRIT_DMG  = 150;  // % extra
constexpr int TOPE_EXTRA     = 35;   // %
constexpr int TOPE_FALLO_MIN = 2;    // % (nunca se acierta el 100 %)

// --- Héroe inicial (§5) -------------------------------------------------------
constexpr int HEROE_HP      = 60;
constexpr int HEROE_ATK     = 8;
constexpr int HEROE_DEF     = 3;
constexpr int HEROE_CRIT    = 5;
constexpr int HEROE_CRITDMG = 50;
constexpr int HEROE_EXTRA   = 5;
constexpr int HEROE_LUCK    = 0;
constexpr int HEROE_FALLO   = 8;

// --- Curación (§6) ------------------------------------------------------------
constexpr int CURA_VICTORIA  = 20;  // % de vida máx.
constexpr int POCION_HPMAX   = 10;  // % de vida máx.
constexpr int PROB_POCION    = 10;  // % del botín

// --- Enemigos (§7) ------------------------------------------------------------
constexpr int   ENEMIGO_HP      = 22;
constexpr int   ENEMIGO_ATK     = 7;
constexpr int   ENEMIGO_DEF     = 3;
constexpr int   ENEMIGO_CRIT    = 5;
constexpr int   ENEMIGO_CRITDMG = 50;
constexpr int   ENEMIGO_FALLO   = 10;
constexpr float CRECE_HP        = 1.070f;  // por encuentro (exponencial)
constexpr float CRECE_ATK       = 1.050f;
constexpr float CRECE_DEF       = 1.065f;
constexpr int   JEFE_CADA       = 10;      // un jefe cada N encuentros
constexpr float GOLPE_FUERTE    = 2.5f;    // multiplicador tras "Preparar"

// --- Botín (§6.2) -------------------------------------------------------------
constexpr float BOTIN_BASE      = 3.0f;    // presupuesto de la estad. principal
constexpr float CRECE_BOTIN     = 1.065f;  // por encuentro
constexpr int   RAREZA_COMUN    = 70;      // pesos base (%)
constexpr int   RAREZA_RARA     = 25;
constexpr int   RAREZA_EPICA    = 5;

// --- Ritmo de presentación (§8.4), en ms --------------------------------------
constexpr uint32_t T_TEXTO_GOLPE   = 450;
constexpr uint32_t T_TEXTO_CRITICO = 650;
constexpr uint32_t T_TEXTO_EXTRA   = 500;
constexpr uint32_t T_TEXTO_CADENA  = 300;  // textos dentro de una cadena
constexpr uint32_t T_TURNO_ENEMIGO = 450;  // pausa antes del ataque enemigo
constexpr uint32_t T_CARRERA       = 1200;
constexpr uint32_t T_SCROLL        = 1400;

}  // namespace bal
