# STICK RPG

**Game Design Document (GDD) — RPG de combate por turnos con scroll lateral automático**

M5StickC Plus2 · Pantalla horizontal 240 × 135 px · 16 colores máximo

> **IDEA CENTRAL**
>
> El guerrero avanza solo entre encuentros. El jugador decide cómo atacar y, tras cada victoria, si acepta la recompensa o conserva su equipo. El objetivo es sobrevivir al mayor número posible de enemigos.

| **DOCUMENTO**                         | **ESTADO**                                                 |
|---------------------------------------|------------------------------------------------------------|
| Versión 1.1 · 19 de noviembre de 2026 | Diseño conceptual consolidado, primera revisión            |
| Alcance                               | Decisiones acordadas y parámetros pendientes de equilibrar |

Nombre de trabajo: «Stick RPG» (coincide con el repositorio). El título definitivo todavía no está decidido.

# 1. Visión general

Juego de pantalla horizontal, estética pixel art y paleta global limitada a 16 colores. No hay salto ni control manual del desplazamiento: el guerrero corre solo y el scroll introduce cada nuevo combate. La interacción se concentra en el botón A durante el combate y en la decisión de recompensas con A y B.

## 1.1. Bucle principal
1. Start: pulsar A para comenzar.
2. Entrada: se carga el escenario; el guerrero aparece y corre hasta el extremo derecho de la pantalla.
3. Transición: el fondo se desplaza horizontalmente hasta situar visualmente de nuevo al héroe a la izquierda; entra un enemigo desde la derecha.
4. Combate: enfrentamiento por turnos. El héroe siempre comienza.
5. Victoria: el héroe recupera un porcentaje de su vida máxima y el enemigo deja exactamente una recompensa; el jugador decide qué hacer con ella.
6. Continuación: el guerrero vuelve a correr hacia la derecha y el escenario presenta al siguiente enemigo.
7. Derrota: pantalla Game Over con estadísticas de la partida, récord y posibilidad de reiniciar.

> **ESTRUCTURA DE LA PARTIDA**
>
> El juego se repite en un bucle de encuentros cada vez más difíciles. El scroll es una transición visual; no hay plataformas, saltos ni esquiva manual. La vida del héroe **se conserva entre combates**: solo se recupera con la curación por victoria y con pociones.

# 2. Dispositivo y restricciones

| **Aspecto**        | **Regla**                                                                 |
|--------------------|---------------------------------------------------------------------------|
| Hardware objetivo  | M5StickC Plus2 (ESP32-PICO-V3-02, pantalla ST7789V2, zumbador, batería)   |
| Orientación        | Horizontal (240 × 135 píxeles)                                            |
| Color              | Máximo 16 colores simultáneos en la paleta del juego                      |
| Estética           | Pixel art, sprites y tipografía de alta legibilidad                       |
| Movimiento         | Avance del personaje y scroll automáticos; sin salto                      |
| Entrada de usuario | Botón A, botón B y botón de encendido (PWR); sin controles externos       |
| Sonido             | Efectos simples mediante el zumbador integrado                            |
| Persistencia       | Récord y ajustes guardados en memoria no volátil (NVS)                    |
| Estructura         | Un enemigo por encuentro, combates por turnos, recompensa al ganar        |

# 3. Estados y controles

| **Estado**             | **Botón A**                                                                 | **Botón B**                         |
|------------------------|-----------------------------------------------------------------------------|-------------------------------------|
| Start                  | Comenzar                                                                    | Activar / silenciar sonido          |
| Carrera / scroll       | Sin acción                                                                  | Sin acción                          |
| Turno del héroe        | Toque: ataque normal. Mantener: carga; soltar: ataque cargado (ver §4.3).   | Abrir panel de estadísticas         |
| Panel de estadísticas  | Sin acción                                                                  | Cerrar panel                        |
| Turno del enemigo      | Sin acción                                                                  | Sin acción                          |
| Recompensa de equipo   | Confirmar la opción resaltada                                               | Alternar CONSERVAR / EQUIPAR        |
| Recompensa de poción   | Confirmar la opción resaltada                                               | Alternar RECUPERAR / VIDA MÁX.      |
| Game Over              | Regresar a Start / reiniciar                                                | Sin acción                          |

El turno extra es un turno del héroe normal: se controla igual.

**Botón PWR** (cualquier estado): pulsación corta = pausa / reanudar. Pulsación larga = apagado del dispositivo (comportamiento del hardware). En pausa, la pantalla baja el brillo tras unos segundos sin actividad.

# 4. Reglas del combate

## 4.1. Secuencia de un enfrentamiento

- El héroe abre siempre el combate.
- El jugador ataca con A (normal o cargado). El golpe puede acertar o fallar.
- Si acierta, se calcula el daño; puede activarse un crítico y aumentar el daño.
- Tras resolver la acción del héroe, se comprueba el turno extra.
- Si hay turno extra, se salta el turno del enemigo y el héroe vuelve a atacar. Si no, ataca el enemigo.
- El turno enemigo se resuelve automáticamente con las mismas reglas: **el enemigo también puede fallar y hacer críticos**. Si ambos siguen vivos, el control vuelve al héroe.
- El combate termina en cuanto la vida de uno de los dos llega a cero.

## 4.2. Daño, defensa, fallo y crítico

Fórmula proporcional (estable en una partida sin fin, porque el resultado depende de la proporción entre ataque y defensa y no de su diferencia):

```
daño_base = ATK² / (ATK + DEF)
daño      = redondear(daño_base × multiplicador_carga × multiplicador_crítico)
daño      = máx(daño, 1)          // solo en impactos acertados
```

- Con ATK = DEF, el golpe hace la mitad del ATK; si la DEF es muy alta, el daño baja de forma gradual, sin llegar a anularse.
- Los golpes fallidos hacen 0 de daño.
- Multiplicador crítico = 1 + CRIT DMG (por ejemplo, +50 % → ×1,5).
- Orden de resolución: ¿acierta? → daño base → × carga → × crítico → redondeo → mínimo 1.

## 4.3. Ataque normal y ataque cargado

La pulsación se interpreta con **un único umbral de tiempo** (`T_umbral`, unos 200 ms):

| **Acción**                              | **Comportamiento**                                                                         |
|-----------------------------------------|--------------------------------------------------------------------------------------------|
| Pulsar y soltar antes de `T_umbral`     | Ataque normal (multiplicador ×1). Sale al soltar.                                          |
| Mantener más de `T_umbral`              | Aparece la barra de carga y empieza a llenarse.                                            |
| Barra llena sin soltar                  | La barra **vuelve a cero y empieza a llenarse otra vez** (ciclo continuo).                 |
| Soltar A                                | Ataque cargado; su multiplicador depende del llenado de la barra en el momento de soltar.  |
| Coste                                   | Cualquier ataque, normal o cargado, ocupa una única acción del héroe.                      |

Multiplicador del ataque cargado según el llenado `f` (0–100 %):

```
multiplicador_carga = CARGA_MIN + (CARGA_MAX − CARGA_MIN) × f
si f ≥ ZONA_PERFECTA: multiplicador_carga = CARGA_PERFECTA
```

Valores iniciales propuestos: `CARGA_MIN` = ×0,5, `CARGA_MAX` = ×1,8, `ZONA_PERFECTA` = 92 %, `CARGA_PERFECTA` = ×2,2.

Así, cargar es una apuesta de timing: si se suelta en el momento justo, el golpe es mucho más fuerte que uno normal; si se suelta tarde (la barra ya se ha reiniciado), puede hacer menos daño que un ataque normal. La zona perfecta se marca en la barra y tiene un destello y un sonido propios.

## 4.4. Turno extra

La probabilidad de turno extra (EXTRA %) sale de las estadísticas del héroe y sus objetos. Al activarse, el enemigo pierde su turno y el héroe ataca de nuevo, sin pulsar nada especial. Es automático.

- Los turnos extra se pueden encadenar, pero **cada turno extra consecutivo reduce a la mitad la probabilidad del siguiente** dentro de la misma secuencia.
- La probabilidad base está limitada por un tope (ver §5).
- Un ataque fallido también puede activar turno extra (regla simple: la comprobación ocurre siempre). Puede cambiarse durante el balance.

# 5. Estadísticas y equipamiento

El guerrero tiene estadísticas base y bonificadores de las piezas equipadas. Al sustituir un objeto, se retiran sus modificadores y se aplican los del nuevo; las estadísticas visibles se actualizan al instante.

| **Estadística**              | **Función**                                                       | **Tope**   |
|------------------------------|-------------------------------------------------------------------|------------|
| Vida (HP actual / máxima)    | Supervivencia. Los objetos no la modifican (ver §6.3).            | —          |
| Ataque (ATK)                 | Potencia ofensiva.                                                | —          |
| Defensa (DEF)                | Reduce el daño entrante (fórmula proporcional).                   | —          |
| Prob. crítica (CRIT %)       | Posibilidad de que un golpe acertado sea crítico.                 | 50 %       |
| Daño crítico (CRIT DMG)      | Daño extra del crítico.                                           | +150 %     |
| Turno extra (EXTRA %)        | Probabilidad de saltar el turno enemigo y volver a atacar.        | 35 %       |
| Suerte (LUCK)                | Mejora la calidad esperable del botín.                            | —          |
| Precisión / fallo            | Probabilidad base de fallo del héroe y del enemigo.               | Por definir |

Los topes son valores iniciales y se ajustarán en el balance. Las estadísticas que superen el tope no tienen efecto adicional; el panel lo indica.

## 5.1. Ranuras

| **Ranura**      | **Estado inicial**                                            |
|-----------------|---------------------------------------------------------------|
| Casco           | Pieza básica equipada                                         |
| Pechera         | Pieza básica equipada                                         |
| Botas / zapatos | Pieza básica equipada                                         |
| Espada          | Arma básica equipada                                          |
| Escudo          | Sin equipar al inicio; ranura disponible para futuros objetos |

**Cada objeto tiene como máximo 2 estadísticas.** Las armas aportan sobre todo estadísticas ofensivas; las armaduras y el escudo, defensa y bonificadores secundarios (crítico, suerte o turno extra). No hay mochila: en cada recompensa se conserva la pieza actual o se sustituye por la nueva.

## 5.2. Panel de estadísticas

Durante el turno del héroe, B abre un panel superpuesto con: HP actual / máxima, ATK, DEF, CRIT %, CRIT DMG, EXTRA %, LUCK y la lista de las cinco ranuras con el nombre del objeto equipado. El combate queda en pausa mientras el panel está abierto; B lo cierra.

# 6. Recompensas y curación

## 6.1. Curación por victoria

Al ganar cada combate, el héroe recupera automáticamente un **porcentaje de su vida máxima** (`CURA_VICTORIA`, valor inicial propuesto: 20 %), sin superar el máximo. Se muestra como un número verde flotante antes de la pantalla de botín.

## 6.2. Obtención de botín

Cada enemigo derrotado deja exactamente 1 recompensa: una pieza de equipo (espada, casco, pechera, botas o escudo) o, con baja probabilidad, una poción. No se acumula una cola de botín. El nivel del enemigo marca la escala general del objeto; la suerte aumenta la probabilidad de obtener mejores estadísticas, pero no la cantidad de objetos.

Probabilidad inicial propuesta de poción: 10 % (por balancear).

## 6.3. Equipo: comparar y decidir

La pantalla de botín enseña lado a lado el objeto equipado y el encontrado (dos columnas de unos 120 px). Con un máximo de 2 estadísticas por objeto, siempre se muestran todos los valores y la diferencia (▲ verde / ▼ roja) en el objeto nuevo.

| **OBJETO ACTUAL**    | **OBJETO NUEVO**         |
|----------------------|--------------------------|
| Espada de hierro     | Espada de plata          |
| ATK +5               | ATK +7 ▲2                |
| CRIT +3%             | EXTRA +4% (nuevo)        |
| \[ CONSERVAR \]      | \[ EQUIPAR \]            |

Si el objeto nuevo no tiene una estadística que sí tiene el actual, se muestra la pérdida (por ejemplo, `CRIT −3% ▼`). La selección se cambia con B y se confirma con A. Al equipar se descarta el anterior; al conservar se pierde el encontrado.

## 6.4. Pociones: recuperar o vida máxima

Las pociones son raras y nunca se guardan para después. Al obtener una, el jugador elige:

| **Opción**   | **Efecto**                                                                         |
|--------------|------------------------------------------------------------------------------------|
| RECUPERAR    | Restaura la vida **por completo**.                                                 |
| VIDA MÁX.    | Aumenta la vida máxima de forma permanente (`POCION_HPMAX`, valor inicial +10 %). La vida actual sube en la misma cantidad. |

Se alterna con B y se confirma con A. La pantalla muestra la vida actual y el resultado de cada opción.

> **EJEMPLO**
>
> Vida 23/50. RECUPERAR → 50/50. VIDA MÁX. → 28/55. Es una decisión entre sobrevivir ahora y ser más resistente después.

Como los objetos nunca cambian la vida máxima, la única forma de subirla son estas pociones. Así se evitan casos raros al cambiar de equipo.

# 7. Dificultad y progresión

Cada nuevo enemigo es un escalón de dificultad. A medida que avanza la partida aumentan su vida, ataque y defensa, y hace falta mejorar el equipo. Las recompensas también escalan con el avance, sin garantizar que cada pieza nueva sea mejor que la actual.

La progresión se basa en la secuencia de encuentros y el equipamiento. No hay experiencia, oro, tiendas ni niveles propios del héroe en esta versión.

# 8. Pantallas y presentación

| **Pantalla**             | **Información imprescindible**                                                                                                      |
|--------------------------|-------------------------------------------------------------------------------------------------------------------------------------|
| Start                    | Título del juego, «A: comenzar», récord guardado e indicador de sonido (B).                                                         |
| Entrada / desplazamiento | Guerrero corriendo, escenario con scroll y entrada del enemigo por la derecha.                                                      |
| Combate                  | Héroe a la izquierda, enemigo a la derecha, HP de ambos, número de encuentro, efectos de golpe/fallo/crítico/turno extra y barra de carga con zona perfecta. |
| Panel de estadísticas    | Estadísticas completas y equipo actual (ver §5.2).                                                                                  |
| Botín: equipo            | Dos columnas; actual + CONSERVAR a la izquierda; nuevo + EQUIPAR a la derecha; diferencias marcadas.                                |
| Botín: poción            | Vida actual, resultado de RECUPERAR y de VIDA MÁX.                                                                                  |
| Pausa                    | «PAUSA» y cómo reanudar (PWR).                                                                                                      |
| Game Over                | Enemigos derrotados, récord (y aviso de «¡Nuevo récord!»), golpe más alto de la partida y «A: reiniciar».                           |

## 8.1. Lenguaje visual

- Solo 16 colores en una paleta fija para toda la experiencia; mantener el contraste del texto y el HUD.
- Sprites y animaciones pequeños pero reconocibles a 240 × 135 píxeles.
- La información crítica (vidas, selección, carga, crítico, fallo y turno extra) debe leerse en segundos.
- En combate no hay desplazamiento ni selección compleja: el juego espera la pulsación o la liberación de A.
- Durante las transiciones no hace falta pulsar nada y el héroe corre solo.

## 8.2. Sonido

Pitidos cortos con el zumbador para: golpe, fallo, crítico, turno extra, entrada en la zona perfecta de la carga, victoria, botín y Game Over. Se puede silenciar desde Start (B) y el ajuste se guarda.

## 8.3. Persistencia

Se guardan en la memoria no volátil (NVS): el récord de enemigos derrotados y el ajuste de sonido. La partida en curso no se guarda; al apagar se pierde.

# 9. Primera versión jugable (MVP)

- Pantalla de Start con récord, entrada corriendo y transición de scroll hasta un enemigo.
- Un guerrero con las cinco ranuras, escudo inicialmente vacío y equipo básico.
- Un enemigo funcional y sistema de turnos: ataque normal, ataque cargado con barra cíclica y zona perfecta, fallos, críticos (también del enemigo) y turno extra con reducción por encadenamiento.
- Fórmula de daño proporcional, topes de estadísticas y recálculo por equipamiento.
- Curación por victoria y una recompensa por combate: comparación de equipo (máx. 2 estadísticas) o poción RECUPERAR / VIDA MÁX.
- Panel de estadísticas con B y pausa con PWR.
- Escalado inicial de dificultad y repetición infinita de encuentros.
- Game Over con enemigos derrotados, récord persistente y reinicio.
- Efectos de sonido básicos.

# 10. Parámetros pendientes de balance

| **Parámetro**  | **Pendiente de fijar**                                                                                        |
|----------------|---------------------------------------------------------------------------------------------------------------|
| Ataques        | `T_umbral`, duración del ciclo de carga, `CARGA_MIN`, `CARGA_MAX`, `ZONA_PERFECTA`, `CARGA_PERFECTA`.        |
| Precisión      | Probabilidad base de fallo (héroe y enemigo), si la carga la modifica y si algún objeto la mejora.            |
| Críticos       | CRIT % y CRIT DMG base; validar topes (50 % / +150 %).                                                        |
| Turno extra    | Probabilidad base; validar tope (35 %) y la reducción a la mitad por encadenamiento.                          |
| Enemigos       | Estadísticas iniciales, curva de crecimiento, probabilidad de fallo/crítico, variedad y tipos visuales.       |
| Botín          | Probabilidades por categoría, probabilidad de poción, fórmula de calidad, efecto exacto de LUCK.             |
| Curación       | `CURA_VICTORIA` y `POCION_HPMAX`.                                                                             |
| Personaje      | Estadísticas iniciales y propiedades del equipo básico.                                                       |
| Transiciones   | Duración de la carrera y del scroll.                                                                          |

> **PRINCIPIO DE DISEÑO**
>
> Mantener los sistemas simples y las decisiones significativas: cuándo soltar el ataque cargado, cómo aprovechar los turnos extra, qué equipamiento conservar y si curarse ahora o crecer a largo plazo. Cualquier nueva mecánica debe justificar su coste en pantalla, controles y claridad.
