# STICK RPG

**Game Design Document (GDD) — RPG de combate por turnos con scroll lateral automático**

M5StickC Plus2 · Pantalla horizontal 240 × 135 px · 8 colores por bioma · Fantasía medieval oscura

> **IDEA CENTRAL**
>
> El guerrero avanza solo entre encuentros. El jugador decide cómo atacar y, tras cada victoria, si acepta la recompensa o conserva su equipo. El objetivo es sobrevivir al mayor número posible de enemigos.

| **DOCUMENTO**                         | **ESTADO**                                                 |
|---------------------------------------|------------------------------------------------------------|
| Versión 1.3 · 19 de noviembre de 2026 | Rediseño visual (8 colores, biomas), especial, jefes, VAMP |
| Alcance                               | Decisiones acordadas y parámetros pendientes de equilibrar |

Nombre de trabajo: «Stick RPG» (coincide con el repositorio). El título definitivo todavía no está decidido.

# 1. Visión general

Juego de pantalla horizontal, pixel art de **fantasía medieval oscura** y 8 colores por bioma (bosque siniestro, cementerio, castillo). El caballero permanece en su puesto; los enemigos entran caminando desde la derecha ante un fondo de varias capas. La interacción se concentra en el botón A durante el combate y en la decisión de recompensas con A y B.

## 1.1. Bucle principal
1. Start: pulsar A para comenzar.
2. Entrada: aparece el escenario del bioma actual con el caballero en guardia a la izquierda.
3. Encuentro: un enemigo entra caminando desde la derecha. Cada encuentro desplaza las capas del fondo para que el paisaje cambie; al cambiar de bioma hay un fundido con su nombre.
4. Combate: enfrentamiento por turnos. El héroe siempre comienza.
5. Victoria: el héroe recupera un porcentaje de su vida máxima y el enemigo deja exactamente una recompensa; el jugador decide qué hacer con ella.
6. Continuación: entra el siguiente enemigo.
7. Derrota: pantalla Game Over con estadísticas de la partida, récord y posibilidad de reiniciar.

> **ESTRUCTURA DE LA PARTIDA**
>
> El juego se repite en un bucle de encuentros cada vez más difíciles. El scroll es una transición visual; no hay plataformas, saltos ni esquiva manual. La vida del héroe **se conserva entre combates**: solo se recupera con la curación por victoria y con pociones.

# 2. Dispositivo y restricciones

| **Aspecto**        | **Regla**                                                                 |
|--------------------|---------------------------------------------------------------------------|
| Hardware objetivo  | M5StickC Plus2 (ESP32-PICO-V3-02, pantalla ST7789V2, zumbador, batería)   |
| Orientación        | Horizontal (240 × 135 píxeles)                                            |
| Color              | 8 colores simultáneos; la paleta cambia con el bioma (§8.2)               |
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

## 4.3. Ataque normal y ataque especial (cargado)

**El ataque cargado es especial, no constante:** solo se puede usar **a partir del 3.er golpe y cada 3 golpes** (`ESPECIAL_CADA` = 3).

- Cada ataque normal suma una runa (3 runas en el HUD); los fallos también cuentan.
- Las runas **no se reinician entre combates**.
- Con las 3 runas encendidas, mantener A carga el especial. Al soltarlo se gastan las runas.
- Sin runas suficientes, A ataca **al pulsar**, sin esperar al umbral.
- Con el especial listo, un toque corto sigue siendo un ataque normal y no gasta las runas.
- **Las runas solo se gastan al lanzar el especial**, así se puede reservar. Protecciones: umbral de 350 ms; si se suelta A en los primeros 150 ms de carga cuenta como ataque normal. Mientras se mantiene A, B no hace nada.


La pulsación se interpreta con **un único umbral de tiempo** (`T_umbral`, 350 ms):

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

Valores iniciales: `CARGA_MIN` = ×0,6, `CARGA_MAX` = ×2,4, `ZONA_PERFECTA` = 92 %, `CARGA_PERFECTA` = ×3,2. Son más altos que en la v1.2 porque el especial ahora sale cada 3 golpes.

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
| Vampirismo (VAMP %)          | Cura un % del daño infligido (mínimo 1 si hay VAMP y el golpe acierta). Se ve como «+N» verde sobre el héroe. | 25 % |
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

**Cada objeto tiene como máximo 3 estadísticas:** 1 principal y hasta 2 secundarias, sin repetir. Las armas aportan sobre todo estadísticas ofensivas; las armaduras y el escudo, defensa y bonificadores secundarios (crítico, suerte, turno extra o vampirismo). No hay mochila: en cada recompensa se conserva la pieza actual o se sustituye por la nueva.

## 5.2. Panel de estadísticas

Durante el turno del héroe, B abre una placa de **Atributos**: una fila por atributo con nombre completo (Vida, Ataque, Defensa, Crítico, Poder crítico, Turno extra, Suerte, Vampirismo) y su valor alineado a la derecha; en oro con «máx» si ha llegado al tope. El combate queda en pausa; B vuelve.

# 6. Recompensas y curación

## 6.1. Curación por victoria

Al ganar cada combate, el héroe recupera automáticamente un **porcentaje de su vida máxima** (`CURA_VICTORIA`, valor inicial propuesto: 20 %), sin superar el máximo. Se muestra como un número verde flotante antes de la pantalla de botín.

## 6.2. Obtención de botín

Cada enemigo derrotado deja exactamente 1 recompensa: una pieza de equipo (espada, casco, pechera, botas o escudo) o, con baja probabilidad, una poción. No se acumula una cola de botín. El nivel del enemigo marca la escala general del objeto; la suerte aumenta la probabilidad de obtener mejores estadísticas, pero no la cantidad de objetos.

Probabilidad inicial propuesta de poción: 10 % (por balancear). Los jefes nunca dejan poción.

### Rarezas

| **Rareza** | **Color**           | **Peso base** | **Potencia** | **Secundarias**   |
|------------|---------------------|---------------|--------------|-------------------|
| Común      | Claro (acero)       | 70 %          | ×1,0         | 0 o 1 (50 %)      |
| Rara       | Espectral (bioma)   | 25 %          | ×1,25        | 1 o 2 (40 % dos)  |
| Épica      | Oro                 | 5 %           | ×1,55        | Siempre 2         |

- **LUCK**: cada punto pasa 1 punto de peso de Común a Rara (70 %) y Épica (30 %). Común nunca baja del 10 %. El nombre y el marco del objeto se pintan del color de su rareza, así el efecto de la suerte se ve.
- **Jefes**: siempre dejan un objeto Raro o Épico.

### Generación de objetos

- **Estadística principal**: ATK en la espada y DEF en el resto. Su valor = base × factor de ranura × crecimiento por encuentro × rareza × **variación aleatoria ±25 %**.
- **Estadísticas secundarias** (según la rareza): CRIT, CRIT DMG, EXTRA, LUCK o VAMP (CRIT DMG solo en armas). Su valor es pequeño y casi fijo; crece poco con el nivel para no romper los topes.
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

## 7.1. Enemigos normales

Aparecen **al azar** entre los cuatro tipos en todos los encuentros que no son de jefe.

| **Tipo**  | **Perfil**                    | **HP** | **ATK** | **DEF** | **CRIT** | **Fallo** | **Se prepara** |
|-----------|-------------------------------|--------|---------|---------|----------|-----------|----------------|
| Salteador | Equilibrado                   | ×1,0   | ×1,0    | ×1,0    | 5 %      | 10 %      | 15 %           |
| Golem     | Tanque                        | ×1,4   | ×0,8    | ×1,7    | 3 %      | 12 %      | 10 %           |
| Huargo    | Ágil: crítico alto, DEF baja  | ×0,8   | ×0,95   | ×0,6    | 20 %     | 5 %       | 10 %           |
| Troll     | Bruto: pega fuerte y avisa    | ×1,15  | ×1,25   | ×0,85   | 5 %      | 14 %      | 35 %           |

## 7.2. Jefes

Un jefe **siempre** cada 10 encuentros. Son 5 jefes únicos, con sprite propio y más grande (hasta unos 96 px), que salen en orden. Tras el quinto se repite la secuencia con las estadísticas del nivel. Su botín es como mínimo Raro.

| **Encuentro** | **Jefe**         | **Concepto visual**                                   | **HP** | **ATK** | **DEF** | **CRIT** | **Fallo** | **Se prepara** |
|---------------|------------------|-------------------------------------------------------|--------|---------|---------|----------|-----------|----------------|
| 10            | Rey Ciervo       | Gigante del bosque con cornamenta y corona de raíces | ×2,0   | ×1,1    | ×1,2    | 8 %      | 10 %      | 20 %           |
| 20            | Dama Espectral   | Aparición flotante con velo y cadenas                 | ×1,5   | ×1,15   | ×0,9    | 25 %     | 4 %       | 20 %           |
| 30            | Caballero Negro  | Reflejo oscuro del héroe, gran espada y capa raída    | ×1,9   | ×1,15   | ×1,45   | 10 %     | 8 %       | 30 %           |
| 40            | La Tejedora      | Araña colosal con rostro pálido                       | ×1,7   | ×1,2    | ×1,0    | 15 %     | 6 %       | 25 %           |
| 50            | Rey Liche        | Esqueleto coronado con báculo y fuego espectral       | ×2,2   | ×1,25   | ×1,25   | 12 %     | 6 %       | 35 %           |

## 7.3. Biomas

El bioma cambia **después de cada jefe**: encuentros 1–10 Bosque, 11–20 Cementerio, 21–30 Castillo, y vuelta a empezar. Cada bioma tiene su paleta de 8 colores (§8.2) y su fondo por capas. Héroe, enemigos e interfaz toman la paleta del bioma.

# 8. Pantallas y presentación

| **Pantalla**             | **Información imprescindible**                                                                                                      |
|--------------------------|-------------------------------------------------------------------------------------------------------------------------------------|
| Start                    | Logotipo dibujado («STICK» dorado en relieve, espada como subrayado, «R P G» entre rombos) que cae y aterriza con temblor y destello; reflejo que recorre las letras, destellos y ascuas; «Una leyenda oscura», «Pulsa A», récord y sonido (B). |
| Entrada / desplazamiento | Guerrero corriendo, escenario con scroll y entrada del enemigo por la derecha.                                                      |
| Combate                  | Héroe a la izquierda, enemigo a la derecha, HP de ambos, número de encuentro, efectos de golpe/fallo/crítico/turno extra y barra de carga con zona perfecta. |
| Panel de estadísticas    | Estadísticas completas y equipo actual (ver §5.2).                                                                                  |
| Botín: equipo            | Dos columnas; actual + CONSERVAR a la izquierda; nuevo + EQUIPAR a la derecha; diferencias marcadas.                                |
| Botín: poción            | Vida actual, resultado de RECUPERAR y de VIDA MÁX.                                                                                  |
| Pausa                    | «PAUSA» y cómo reanudar (PWR).                                                                                                      |
| Game Over                | Enemigos derrotados, récord (y aviso de «¡Nuevo récord!»), golpe más alto de la partida y «A: reiniciar».                           |

## 8.1. Lenguaje visual

- 8 colores por bioma, con el mismo papel en todos. Mantener el contraste del texto y el HUD.
- Texto: títulos y pantallas en FreeSerif 9–18 pt; HUD en fuente pixel 5×7 en negrita. La placa del enemigo muestra su vida restante (número y barra).
- Botín: dos placas lado a lado (actual a la izquierda, nueva a la derecha). Cada una muestra el tipo de objeto (serif 9 negrita) y su rareza debajo; la lista de estadísticas va en fuente pixel 5×7 con interlineado amplio. La equipada muestra solo sus estadísticas, tal cual; la nueva muestra las suyas y, al final, las que se perderían («--»), y es la única con comparación: la diferencia con flecha y número en serif negrita (verde ▲ mejora, roja ▼ empeora). Las placas son los propios botones: B cambia, A confirma; la elegida se enmarca en oro y la otra se atenúa.
- Poción: mismo formato, dos placas seleccionables (Recuperar | Vida máx.) con la vida resultante en grande y la actual debajo.
- HUD: placas espejo. Héroe: corazón + «CABALLERO», vida y barra. Enemigo: nombre + calavera/corona, barra y vida. Centro: número de encuentro en romanos y runas del especial.
- Interfaz tematizada, abajo para dejar el cielo libre a los jefes: placas de hierro con remaches dorados, barras segmentadas, número de encuentro en **números romanos**, runas del especial, títulos en serif (FreeSerifBold) y textos en una fuente pixel propia de 5×7.
- Sprites y animaciones pequeños pero reconocibles a 240 × 135 píxeles.
- La información crítica (vidas, selección, carga, crítico, fallo y turno extra) debe leerse en segundos.
- En combate no hay desplazamiento ni selección compleja: el juego espera la pulsación o la liberación de A.
- Durante las transiciones no hace falta pulsar nada y el héroe corre solo.

## 8.2. Paletas (8 colores por bioma)

Cada índice tiene un papel fijo; el bioma solo cambia el tono. Los sprites y fondos se dibujan una vez y el juego cambia la paleta al entrar en otro bioma.

| **#** | **Papel**  | **Uso**                                        | **Bosque** | **Cementerio** | **Castillo** |
|-------|------------|------------------------------------------------|------------|----------------|--------------|
| 0     | Negro      | Contornos, paneles, siluetas cercanas          | `#0A0E0D`  | `#0B0A12`      | `#0E0A0A`    |
| 1     | Oscuro     | Cielo, sombras, fondo de barras                | `#1B2925`  | `#221D33`      | `#2A1E1D`    |
| 2     | Medio      | Bruma, malla, sombra del acero                 | `#3B5247`  | `#433B5C`      | `#52403A`    |
| 3     | Claro      | Acero, piedra, rareza Común                    | `#7A9380`  | `#827A9A`      | `#8E7E72`    |
| 4     | Brillo     | Texto, luna, brillos                           | `#D4DEC8`  | `#DAD4E6`      | `#E4DACA`    |
| 5     | Sangre     | Vida, daño, capa y penacho                     | `#9C2E30`  | `#A02C3E`      | `#B8302A`    |
| 6     | Oro        | Crítico, zona perfecta, selección, rareza Épica | `#D6A248`  | `#D8B462`      | `#ECA43A`    |
| 7     | Espectral  | Magia del bioma, runas, carga, rareza Rara     | `#9FE05A`  | `#72E4D4`      | `#A472E0`    |

Los índices 8–15 no se usan; el 15 es la transparencia de los sprites.

## 8.3. Sonido y música

Zumbador integrado, con secuenciador de notas que no bloquea el juego.

| **Momento**        | **Sonido**                                                    |
|--------------------|---------------------------------------------------------------|
| Pantalla de inicio | Jingle corto: fanfarria en La menor (~1,6 s)                  |
| Aparece un jefe    | Micro jingle: tritono amenazante (~0,9 s) y temblor           |
| Nuevo bioma        | Arpegio ascendente con el título del capítulo                 |
| Victoria           | Arpegio breve mientras el enemigo se desvanece                |
| Derrota            | Melodía descendente                                           |
| Combate            | Golpe, fallo, crítico, turno extra, carga, zona perfecta, pasos del enemigo, aviso de golpe fuerte |

Se puede silenciar desde el inicio (B); el ajuste se guarda.

## 8.6. Efectos y animación

- **Respiración**: héroe y enemigos alternan dos frames en reposo; los fantasmas (Dama Espectral, Rey Liche) flotan.
- **Entrada**: cada enemigo entra caminando desde la derecha y frena al llegar.
- **Golpes**: el héroe tarda ~180 ms en descargar el tajo; el golpeado retrocede y parpadea en blanco.
- **Temblor de pantalla**: fuerte al recibir un crítico, medio con golpe fuerte, crítico propio o carga perfecta; leve en golpes normales.
- **Destello**: la carga perfecta aclara toda la paleta un instante.
- **Muerte**: el enemigo se disuelve con trama.
- **Fundidos** de paleta entre encuentros; al cambiar de bioma, cartela «CAPÍTULO II · El Cementerio».
- **Jefes**: banda negra con su nombre en oro.
- **Carga del especial**: la espada se alza con chispas espectrales y la barra aparece arriba, a lo ancho de toda la pantalla.

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
- Curación por victoria y una recompensa por combate: comparación de equipo (máx. 3 estadísticas) o poción RECUPERAR / VIDA MÁX.
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

## 11.1. Dirección de arte

- Fantasía medieval oscura: bosque siniestro de árboles muertos bajo una luna pálida, cementerio y castillo.
- Personajes de **~64 px** (jefes hasta ~96 px), vista lateral, contorno negro, luz desde arriba a la izquierda con 3 tonos por material.
- El héroe no camina: está en guardia a la izquierda. Los enemigos **entran caminando** desde la derecha.

## 11.2. Fondo por capas (parallax)

| **Capa**  | **Contenido (bosque)**                          | **Movimiento**                      |
|-----------|-------------------------------------------------|-------------------------------------|
| Cielo     | Degradado tramado, luna, estrellas, bruma        | Fijo                                |
| Lejos     | Línea de pinos                                   | Desplazamiento pequeño por encuentro |
| Nubes     | Procedurales (ruido periódico), 2 capas, nuevas en cada bioma; no en interiores | Deriva lenta continua (2 velocidades) |
| Niebla    | Jirones tramados                                 | Estática                            |
| Medio     | Árboles muertos retorcidos con luz de luna       | Desplazamiento medio                |
| Suelo     | Tierra, piedras, matas                           | Desplazamiento grande               |
| *Personajes* |                                               |                                     |
| Frente    | Helechos negros en los bordes                    | El mayor desplazamiento             |

Todas las capas miden 240 px de ancho y se repiten en horizontal.

## 11.3. Héroe

Caballero de 80 × 72 px (pies en (30, 71)). Animaciones: `idle` (2), `attack` (3: preparación, tajo, remate), `charge` (espada en alto mientras se carga el especial) y `hit` (1).

| **Capa** (de abajo arriba) | **Se dibuja si…**   | **Recoloreado**      |
|----------------------------|---------------------|----------------------|
| cuerpo (capa, malla)       | siempre             | —                    |
| botas (grebas)             | hay botas           | Rareza de las botas  |
| pechera                    | hay pechera         | Rareza de la pechera |
| casco                      | hay casco           | Rareza del casco     |
| escudo                     | hay escudo          | Rareza del escudo    |
| brazo (brazo + espada)     | siempre             | Rareza de la espada  |

El equipo usa los tonos Medio, Claro y Brillo. Según la rareza, el Claro pasa a Espectral (Rara) u Oro (Épica).

## 11.4. Flujo de trabajo

1. `tools/art/*.py` generan los sprites y fondos (rasterizador propio con sombreado y contornos) y la captura simulada `tools/art/mock.py`.
2. El resultado se vuelca a `assets/sprites/*.aseprite` (fuente editable a mano) y `make sprites` lo exporta a `src/gfx/*_gfx.h` (4 bpp).
3. El firmware dibuja escribiendo directamente en el lienzo de 4 bpp, para mover varias capas de 240 px a 30 fps.

## 11.5. Estado

- Hecho: héroe, 4 enemigos, 5 jefes, fondos de los 3 biomas, interfaz completa (combate, botín, poción, estadísticas, inicio, capítulo, jefe, pausa, derrota) e integración en el firmware.
- Siguiente: probar en el dispositivo y pulir.

# 12. Estructura técnica

| **Ruta**                | **Contenido**                                                      |
|-------------------------|--------------------------------------------------------------------|
| `src/core/`             | Reglas del juego en C++ puro, sin Arduino (combate, botín, enemigos) |
| `src/core/balance.h`    | Todos los parámetros de balance                                     |
| `src/main.cpp`          | Firmware: estados, entrada, render 4 bpp, sonido, NVS              |
| `src/palette.h`         | Paletas de 8 colores por bioma                                      |
| `tools/art/`            | Generadores de sprites, fondos, fuente y captura simulada           |
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
