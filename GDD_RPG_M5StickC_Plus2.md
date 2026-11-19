# STICK RPG

**Game Design Document (GDD) — RPG de combate por turnos con scroll lateral automático**

M5StickC Plus2 · Pantalla horizontal 240 × 135 px · 16 colores máximo

> **IDEA CENTRAL**
>
> El guerrero avanza solo entre encuentros. El jugador decide cómo atacar y, tras cada victoria, si acepta la recompensa o conserva su equipo. El objetivo es sobrevivir al mayor número posible de enemigos.

| **DOCUMENTO**                         | **ESTADO**                                                 |
|---------------------------------------|------------------------------------------------------------|
| Versión 1.2 · 19 de noviembre de 2026 | Segunda revisión; prototipo de firmware y simulador        |
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

## 4.5. Golpe fuerte anunciado

En su turno, el enemigo puede **prepararse** en lugar de atacar. Ese turno no hace daño, pero aparece un «!» naranja parpadeando sobre él y el texto «¡SE PREPARA!». Su siguiente acción es un **golpe fuerte** (`GOLPE_FUERTE`, ×2,5), que también puede fallar o ser crítico.

- El aviso no desaparece con los turnos extra del héroe: el golpe fuerte llega en el siguiente turno real del enemigo.
- Así hay algo que decidir sin añadir botones: arriesgar una carga perfecta para matarlo antes, o asumir el golpe.
- Cada tipo de enemigo se prepara con una frecuencia distinta (§7.1).

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

Probabilidad inicial propuesta de poción: 10 % (por balancear). Los jefes nunca dejan poción.

### Rarezas

| **Rareza** | **Color**  | **Peso base** | **Potencia** | **2.ª estadística** |
|------------|------------|---------------|--------------|---------------------|
| Común      | Gris claro | 70 %          | ×1,0         | 40 %                |
| Rara       | Azul       | 25 %          | ×1,25        | 80 %                |
| Épica      | Morado     | 5 %           | ×1,55        | Siempre             |

- **LUCK**: cada punto pasa 1 punto de peso de Común a Rara (70 %) y Épica (30 %). Común nunca baja del 10 %. El nombre y el marco del objeto se pintan del color de su rareza, así el efecto de la suerte se ve.
- **Jefes**: siempre dejan un objeto Raro o Épico.

### Generación de objetos

- **Estadística principal**: ATK en la espada y DEF en el resto. Su valor = base × factor de ranura × crecimiento por encuentro × rareza × **variación aleatoria ±25 %**.
- **Estadística secundaria** (según la rareza): CRIT, CRIT DMG, EXTRA o LUCK. Su valor es pequeño y casi fijo; crece poco con el nivel para no romper los topes.
- **Material** (nombre): cambia cada 6 encuentros (Madera, Hierro, Acero, Plata, Mitril, Rúnico, Astral). Por ejemplo, «Espada de Acero».
- **¿Puede salir peor?** Sí. Por la variación ±25 % y porque la pieza nueva puede traer otra estadística secundaria (o ninguna), un objeto de más nivel puede ser peor que el actual. La comparación lo enseña con claridad.

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

Las estadísticas de los enemigos crecen de forma exponencial con el número de encuentro (HP ×1,07, ATK ×1,05 y DEF ×1,065 por encuentro, valores iniciales). Como el héroe solo sube su vida máxima con pociones, la dificultad acaba superándolo: la partida siempre termina.

## 7.1. Tipos de enemigo

| **Tipo**  | **Perfil**                    | **HP** | **ATK** | **DEF** | **CRIT** | **Fallo** | **Se prepara** |
|-----------|-------------------------------|--------|---------|---------|----------|-----------|----------------|
| Bandido   | Equilibrado                   | ×1,0   | ×1,0    | ×1,0    | 5 %      | 10 %      | 15 %           |
| Golem     | Tanque                        | ×1,4   | ×0,8    | ×1,7    | 3 %      | 12 %      | 10 %           |
| Lobo      | Ágil: crítico alto, DEF baja  | ×0,8   | ×0,95   | ×0,6    | 20 %     | 5 %       | 10 %           |
| Ogro      | Bruto: pega fuerte y avisa    | ×1,15  | ×1,25   | ×0,85   | 5 %      | 14 %      | 35 %           |
| **Jefe**  | Cada 10 encuentros            | ×1,9   | ×1,1    | ×1,15   | 10 %     | 8 %       | 25 %           |

- El encuentro 1 siempre es un Bandido. A partir del 2, el tipo es aleatorio entre los cuatro normales.
- El jefe aparece en los encuentros 10, 20, 30… El número de encuentro se pinta en morado y su botín es como mínimo Raro.

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

## 8.2. Paleta

Paleta fija de 16 colores. Cada color tiene un uso reservado para que la información se reconozca siempre igual.

| **#** | **Nombre**   | **Hex**   | **Uso principal**                         |
|-------|--------------|-----------|-------------------------------------------|
| 0     | Negro        | `#0B0B14` | Contornos, paneles, sombras de texto      |
| 1     | Noche        | `#1E2140` | Cielo, fondo de pantallas                 |
| 2     | Pizarra      | `#33406B` | Montañas lejanas, separadores             |
| 3     | Gris         | `#5B6B8C` | Golem, texto desactivado                  |
| 4     | Gris claro   | `#A6AEBF` | Texto secundario, rareza Común            |
| 5     | Blanco       | `#F2F2EB` | Texto principal, hoja de la espada        |
| 6     | Marrón osc.  | `#4A2E22` | Tierra                                    |
| 7     | Marrón       | `#8C5A3C` | Madera, lobo, zona perfecta (fondo)       |
| 8     | Piel         | `#E8B58A` | Piel                                      |
| 9     | Rojo         | `#C23B3B` | Daño recibido, ▼, vida enemiga, bandido   |
| 10    | Naranja      | `#F08A3C` | Aviso de golpe fuerte, estadística topada |
| 11    | Amarillo     | `#F7D046` | Crítico, zona perfecta, selección, título |
| 12    | Verde        | `#4CB050` | Vida del héroe, curación, ▲               |
| 13    | Verde osc.   | `#2A6B4F` | Hierba, ogro                              |
| 14    | Azul         | `#4AA3F0` | Rareza Rara, turno extra, barra de carga  |
| 15    | Morado       | `#9A5BD6` | Rareza Épica, jefe                        |

## 8.3. Sonido

Pitidos cortos con el zumbador para: golpe, fallo, crítico, turno extra, entrada en la zona perfecta de la carga, aviso de golpe fuerte, victoria, botín y Game Over. Se puede silenciar desde Start (B) y el ajuste se guarda.

## 8.4. Ritmo de los textos de combate

| **Evento**                     | **Duración** |
|--------------------------------|--------------|
| Golpe / fallo                  | 450 ms       |
| Crítico / golpe fuerte         | 650 ms       |
| Turno extra                    | 500 ms       |
| Textos dentro de una cadena    | 300 ms       |
| Pausa antes del turno enemigo  | 450 ms       |
| «¡SE PREPARA!»                 | 650 ms       |

Los textos de un mismo golpe se juntan en un solo mensaje (por ejemplo, «PERFECTO CRITICO +TURNO») en lugar de mostrarse uno detrás de otro. El daño aparece como un número que sube sobre el personaje golpeado.

## 8.5. Persistencia

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
- Cuatro tipos de enemigo + jefe cada 10, con golpe fuerte anunciado.
- Rarezas visibles por color.
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
| Golpe fuerte   | Multiplicador (×2,5) y frecuencia por tipo.                                                                   |

Todos los valores viven en `src/core/balance.h`. El simulador (`make sim`) juega miles de partidas con un jugador automático para ajustarlos.

**Primera pasada del simulador** (5000 partidas, `skill` = probabilidad de soltar en la zona perfecta):

| **Jugador**                  | **Media** | **p10** | **Mediana** | **p90** |
|------------------------------|-----------|---------|-------------|---------|
| Solo ataque normal           | 13,9      | 9       | 14          | 19      |
| Carga sin timing (skill 0)   | 14,9      | 9       | 15          | 19      |
| Carga con skill 0,5          | 20,8      | 16      | 20          | 27      |
| Carga con skill 0,9          | 24,9      | 19      | 25          | 29      |

Se cumple lo buscado: cargar sin timing apenas mejora el ataque normal, y la habilidad sí se nota. Las muertes ante un jefe rondan el 32 %; habrá que revisarlo.

# 11. Sprites y gráficos

El prototipo dibuja personajes y fondo con primitivas (rectángulos, líneas, círculos) en un lienzo de 4 bpp con la paleta de §8.2. Los sprites definitivos sustituirán esas funciones de dibujo.

**Requisitos de los sprites:**

| **Elemento**  | **Tamaño aprox.** | **Animaciones**                          |
|---------------|-------------------|------------------------------------------|
| Héroe         | 24–40 px de alto  | Correr, reposo, ataque, recibir golpe    |
| Enemigos (×5) | 24–56 px de alto  | Reposo, ataque, recibir golpe, prepararse |
| Fondo         | Tiles de 16–24 px | Scroll horizontal con parallax           |

- **Vista lateral**, mirando a la derecha (héroe) y a la izquierda (enemigos).
- Solo los 16 colores de la paleta (§8.2). El índice 1 (Noche) es el transparente: los personajes no usan ese color.

## 11.1. Flujo de trabajo (Aseprite)

1. La fuente de cada sprite es `assets/sprites/<nombre>.aseprite`: modo indexado, paleta del juego, una **capa por pieza** y una **etiqueta por animación** (`idle`, `run`, `attack`, `hit`…).
2. `make sprites` exporta cada archivo a `src/gfx/<nombre>_gfx.h` (4 bpp, 2 píxeles por byte, en flash). Los nombres de capa y de etiqueta pasan a ser constantes en C++.
3. El firmware dibuja capa a capa con una tabla de recoloreado.

## 11.2. Héroe

Lienzo de 32 × 36 px, con los pies en (13, 35). Diez frames: `idle` (2 × 400 ms), `run` (4 × 100 ms), `attack` (3 × 90 ms: preparación, tajo, remate) y `hit` (1).

| **Capa** (de abajo arriba) | **Se dibuja si…**              | **Recoloreado**              |
|----------------------------|--------------------------------|------------------------------|
| escudo                     | hay escudo equipado            | Rareza del escudo            |
| cuerpo                     | siempre                        | —                            |
| botas                      | hay botas                      | Rareza de las botas          |
| pechera                    | hay pechera                    | Rareza de la pechera         |
| casco                      | hay casco                      | Rareza del casco             |
| brazo (brazo + espada)     | siempre                        | Rareza de la espada          |

Las piezas de equipo se dibujan en Gris claro y Gris. El juego cambia esos dos colores según la rareza: Común sin cambio, Rara en Azul y Pizarra, Épica en Morado y Pizarra. El Blanco (brillo) y el resto de colores no cambian. Al recibir un golpe, el sprite parpadea en blanco.

La primera versión del héroe la generó `tools/sprites/gen_heroe.lua` (`make gen-heroe`, **sobrescribe** el archivo). A partir de ahí se edita a mano en Aseprite.

## 11.3. Pendiente

- Enemigos (Bandido, Golem, Lobo, Ogro, Jefe) con `idle`, `attack`, `hit` y `prepare`, siguiendo el mismo flujo. Una sola capa, salvo que se quiera variar su color.
- Fondo en tiles con parallax.
- SpriteCast queda descartado por ahora: pensado para sprites grandes con cámara 3D y servicios de pago. Se puede revisar si hacen falta ilustraciones grandes (por ejemplo, para la pantalla de título).

# 12. Estructura técnica

| **Ruta**                | **Contenido**                                                      |
|-------------------------|--------------------------------------------------------------------|
| `src/core/`             | Reglas del juego en C++ puro, sin Arduino (combate, botín, enemigos) |
| `src/core/balance.h`    | Todos los parámetros de balance                                     |
| `src/main.cpp`          | Firmware: estados, entrada, render 4 bpp, sonido, NVS              |
| `src/palette.h`         | Paleta de 16 colores                                                |
| `src/gfx/`              | Dibujo de sprites (`sprite.h`) y cabeceras exportadas (`*_gfx.h`)   |
| `assets/sprites/`       | Fuentes `.aseprite`                                                 |
| `tools/sprites/`        | Exportador a C y generador del héroe                                |
| `tools/sim/`            | Simulador de balance para PC                                        |
| `tools/test/`           | Pruebas de la lógica                                                |

- Firmware: `pio run -t upload` (PlatformIO + M5Unified).
- PC: `make test` y `make sim ARGS="--skill 0.7"`.
- Sprites: `make sprites` (necesita Aseprite; ruta configurable con `ASEPRITE=`).

> **PRINCIPIO DE DISEÑO**
>
> Mantener los sistemas simples y las decisiones significativas: cuándo soltar el ataque cargado, cómo aprovechar los turnos extra, qué equipamiento conservar y si curarse ahora o crecer a largo plazo. Cualquier nueva mecánica debe justificar su coste en pantalla, controles y claridad.
