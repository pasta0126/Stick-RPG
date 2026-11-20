# Prompts de personajes para generar sprites

Pega primero el **bloque de estilo común** y después el bloque del personaje. Un personaje por conversación (o por mensaje) para que no se mezclen estilos.

---

## Bloque de estilo común (pegar siempre)

```
Genera una hoja de sprites de pixel art para un videojuego RPG de fantasía medieval oscura.

ESTILO
- Pixel art limpio, estilo 16 bits, vista lateral pura (de perfil), personaje de pie sobre una línea de suelo plana.
- Contorno negro de 1 píxel alrededor de toda la silueta.
- Luz desde arriba a la izquierda: borde superior/izquierdo más claro, sombra de 2 px abajo/derecha. Tres tonos por material.
- Sin antialiasing, sin degradados suaves, sin píxeles sueltos, sin texturas de ruido, sin desenfoque. Bloques de color planos.
- Siluetas grandes y legibles: se verá en una pantalla diminuta de 240×135.
- Cada píxel debe ser un cuadrado perfecto y todos del mismo tamaño (cuadrícula de píxeles visible y regular).

PALETA ESTRICTA (usa SOLO estos 8 colores, nada más):
- #0A0E0D negro (contornos, sombras más profundas)
- #1B2925 oscuro (sombras, cuero, tela oscura)
- #3B5247 medio (malla, sombra del acero, piedra)
- #7A9380 claro (acero, piedra, hueso)
- #D4DEC8 brillo (brillos, ojos, reflejos)
- #9C2E30 sangre (tela roja, capa, sangre)
- #D6A248 oro (ribetes dorados, coronas, hebillas) — con moderación
- #9FE05A espectral (brillo mágico, ojos, runas, fuego fantasmal) — con moderación

FONDO: magenta puro #FF00FF liso, sin sombra en el suelo, sin escenario.

FORMATO: una tira horizontal con los fotogramas en fila, todos del mismo tamaño, el personaje con los pies en la misma línea de suelo en todos, misma escala y mismo encuadre. Separación de magenta entre fotogramas. Sin texto, sin números, sin etiquetas.
```

> Consejo: si la imagen sale "pintada" en vez de pixel art, añade: *"Que parezca dibujado a mano en Aseprite a resolución nativa de ~80 px de alto y escalado ×8 con vecino más cercano."*

---

## Fotogramas para los enemigos (pegar después del personaje)

Todos los enemigos y jefes **miran a la IZQUIERDA** y usan exactamente estos 8 fotogramas, en este orden:

```
8 fotogramas en una fila, mirando a la IZQUIERDA:
1. Caminar A (paso con la pierna delantera adelantada)
2. Caminar B (paso contrario)
3. Reposo A (respirando, postura de guardia)
4. Reposo B (igual que el 3 pero el torso, la cabeza y los brazos 1 píxel más abajo)
5. Ataque: preparación (arma o garra echada hacia atrás)
6. Ataque: golpe (arma o garra lanzada hacia delante, a la izquierda)
7. Recibe golpe (retrocede hacia la derecha, encogido)
8. Preparando golpe fuerte (postura tensa, arma muy atrás o en alto, brillo espectral más intenso)
```

---

## Héroe — El Caballero

```
PERSONAJE: el héroe, un caballero oscuro de cuerpo entero, unos 64 px de alto en resolución nativa, mirando a la DERECHA.
- Yelmo cerrado de acero con visera de rendijas horizontales y una cresta; penacho rojo (#9C2E30) que cae hacia atrás.
- Peto de acero con una hombrera grande en el hombro delantero, cinturón oscuro con hebilla dorada.
- Debajo, cota de malla oscura y un faldón de tabardo.
- Capa roja (#9C2E30) que cae por la espalda.
- Grebas de acero ahusadas (más anchas en la rodilla) y escarpes.
- En el brazo trasero, un escudo de acero con una cruz/espada dorada como emblema.
- En el brazo delantero, una espada larga de acero con guarda y pomo dorados, guantelete de acero.
- Las piezas de armadura (yelmo, peto, grebas, escudo, espada) en tonos de acero #3B5247 / #7A9380 / #D4DEC8. Lo rojo y lo dorado NO deben usar el gris #7A9380.

6 fotogramas en una fila, mirando a la DERECHA (el héroe nunca camina):
1. Reposo A: en guardia, espada en alto inclinada hacia delante.
2. Reposo B: igual, pero el torso, la cabeza y los brazos 1 píxel más abajo (respiración).
3. Ataque – preparación: espada echada atrás por encima del hombro.
4. Ataque – tajo: espada horizontal hacia delante, cuerpo adelantado.
5. Ataque – remate: espada terminando el arco, apuntando abajo-delante.
6. Carga: espada levantada en vertical sobre la cabeza.
(Opcional 7. Recibe golpe: retrocede hacia atrás, encogido tras el escudo.)
```

> Nota: en el juego el héroe va separado en capas (cuerpo, botas, pechera, casco, escudo, brazo). ChatGPT no puede darlo así; pásame la tira entera y yo lo separo.

---

## Enemigos normales

### Salteador (bandido)

```
PERSONAJE: un salteador de caminos, unos 64 px de alto, mirando a la IZQUIERDA. Ágil y equilibrado.
- Capucha y capa corta de cuero marrón oscuro que le tapa la cabeza; la cara es una sombra negra con dos ojos pequeños brillantes en verde espectral (#9FE05A).
- Pañuelo rojo (#9C2E30) al cuello y una bufanda roja que ondea hacia atrás.
- Jubón de cuero, cinturón negro con hebilla dorada pequeña.
- Pantalones y botas de cuero oscuro.
- Una daga corta de acero en la mano delantera.
- Postura encorvada y sigilosa, de ladrón.
```

### Gólem

```
PERSONAJE: un gólem de piedra, unos 76 px de alto, mirando a la IZQUIERDA. Tanque lento y macizo.
- Cuerpo de bloques de piedra gris (#3B5247 / #7A9380) toscamente encajados, hombros enormes, cabeza pequeña hundida entre ellos.
- Grietas oscuras por todo el cuerpo.
- Una runa grabada en el pecho que brilla en verde espectral (#9FE05A); en el fotograma 8 brilla mucho más.
- Ojos de luz espectral.
- Brazos larguísimos con puños de roca enormes que casi tocan el suelo; piernas cortas y gruesas.
- Ataque: levanta un puño por encima de la cabeza y lo descarga hacia delante y abajo.
```

### Huargo (lobo)

```
PERSONAJE: un huargo, lobo gigante y salvaje, unos 60 px de alto y 90 px de largo, a cuatro patas, mirando a la IZQUIERDA. Rápido y feroz.
- Pelaje oscuro (#1B2925 / #3B5247) con una crin dentada y erizada a lo largo del lomo.
- Cola larga y espesa.
- Cabeza baja, hocico largo con colmillos blancos (#D4DEC8), ojos rojos (#9C2E30) o espectrales.
- Patas fuertes con garras.
- Caminar: trote al paso. Ataque: se agacha y luego se lanza con las fauces abiertas.
```

### Troll

```
PERSONAJE: un troll enorme y bruto, unos 88 px de alto, mirando a la IZQUIERDA. Pega muy fuerte y es lento.
- Cuerpo jorobado y musculoso, piel gruesa y rugosa en tonos piedra (#1B2925 / #3B5247 / #7A9380).
- Cabeza pequeña, baja y adelantada respecto al cuerpo, mandíbula prominente con colmillos inferiores.
- Brazos larguísimos; taparrabos de cuero.
- Garrote de madera enorme lleno de pinchos de hierro en la mano delantera.
- Ataque: levanta el garrote por encima de la cabeza y lo estrella delante.
```

---

## Jefes (más grandes: 90–108 px de alto)

### Rey Ciervo (jefe del Bosque)

```
PERSONAJE: el Rey Ciervo, gigante del bosque, unos 105 px de alto, mirando a la IZQUIERDA. Jefe solemne y antiguo.
- Cabeza de cráneo de ciervo (hueso #7A9380 / #D4DEC8) con cuencas que brillan en verde espectral.
- Una cornamenta enorme y ramificada que se abre por encima de la cabeza.
- Corona de raíces retorcidas entre los cuernos, con algún toque dorado.
- Túnica larga de musgo (verde oscuro) con el bajo raído y deshilachado.
- Hombreras de corteza de árbol.
- Báculo largo de madera retorcida en la mano delantera; en reposo lo apoya en el suelo.
- Ataque: alza el báculo y lo golpea hacia delante.
- Importante: la cara debe quedar por debajo del 15 % superior de la imagen (los cuernos sí pueden subir).
```

### Dama Espectral (jefa del Cementerio)

```
PERSONAJE: la Dama Espectral, una aparición fantasmal femenina, unos 100 px de alto, mirando a la IZQUIERDA. Flota sobre el suelo, sin pies.
- Vestido largo y pálido (#7A9380 / #D4DEC8) que termina en jirones deshilachados que se desvanecen abajo.
- Cabello largo y velo ondeando al viento hacia atrás.
- El rostro bajo el velo es un hueco oscuro con ojos de luz espectral.
- Un brazo extendido del que cuelgan cadenas.
- Halo y contorno interior de 1 px en verde espectral (#9FE05A) dentro del contorno negro.
- En vez de caminar, flota: los fotogramas de caminar y reposo suben y bajan unos píxeles.
- Ataque: lanza la cadena hacia delante.
```

### Caballero Negro (jefe del Castillo)

```
PERSONAJE: el Caballero Negro, un reflejo oscuro del héroe, unos 104 px de alto, mirando a la IZQUIERDA.
- Armadura completa negra (#0A0E0D / #1B2925) con solo un filo de luz gris (#7A9380) en los bordes superiores.
- Yelmo cerrado con cuernos curvados; ojos de la visera en verde espectral.
- Hombreras con pinchos.
- Capa raída y desgarrada, roja muy oscura, que cae por detrás.
- Faldón de placas.
- Mandoble enorme sujeto a dos manos.
- Ataque: mandoble alzado por encima de la cabeza y tajo descendente.
```

### La Tejedora (jefa)

```
PERSONAJE: La Tejedora, una araña colosal, unos 90 px de alto y 124 px de ancho, mirando a la IZQUIERDA.
- Cuerpo de araña negro y oscuro, abdomen grande y abultado detrás, con una marca de calavera pálida en el abdomen.
- Tórax delante y, en lugar de cabeza, una máscara pálida e inexpresiva de rostro humano (#D4DEC8) con ojos que brillan en verde espectral.
- Ocho patas largas y articuladas: cuatro del lado lejano (más oscuras) y cuatro del lado cercano.
- Caminar: patas alternándose. Ataque: alza la pata delantera y la clava hacia delante.
```

### Rey Liche (jefe final)

```
PERSONAJE: el Rey Liche, un esqueleto coronado, unos 105 px de alto, mirando a la IZQUIERDA. Jefe final.
- Cráneo con cuencas que arden en fuego verde espectral (#9FE05A).
- Corona dorada (#D6A248) de puntas.
- Túnica oscura con ribete dorado y el bajo en jirones; costillas a la vista por el pecho abierto.
- Cuello de capa alto.
- Brazo huesudo y delgado que sostiene un báculo largo coronado por una llama espectral verde.
- La otra mano libre; en el fotograma 8 (preparando) sostiene una bola de fuego espectral.
- Ataque: apunta el báculo hacia delante.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

---

## Qué hago yo con las imágenes

ChatGPT no clava ni el tamaño exacto ni la paleta, así que pásame las tiras tal cual. Yo:

1. Recorto cada fotograma y lo reduzco a la resolución real (vecino más cercano).
2. Lo cuantizo a los 8 índices de la paleta y pongo el magenta como transparencia.
3. Lo coloco con el lienzo y el anclaje de cada personaje (ver `docs/STYLE_GUIDE.md` §5) y lo meto en su `.aseprite`.

---

## Héroe v2 — versión con capas (usar esta)

Se hace en **dos mensajes dentro de la misma conversación**: primero la tira y después el mapa de piezas sobre esa misma imagen.

### Mensaje 1 — la tira

````
Genera una tira de sprites de pixel art del héroe de un RPG de fantasía medieval oscura.

PERSONAJE
Un caballero de cuerpo entero, de perfil, mirando a la DERECHA.
- Proporciones heroicas compactas: unas 4 cabezas de alto. Nada de estilo chibi ni cabeza enorme.
- Yelmo cerrado de acero con visera de rendijas horizontales y una cresta; penacho rojo que cae hacia atrás.
- Peto de acero con una hombrera grande en el hombro delantero y un cinturón oscuro con hebilla dorada pequeña.
- Debajo, cota de malla oscura y un faldón corto de tela oscura.
- Capa roja que cae por la espalda.
- Grebas de acero (más anchas en la rodilla) y escarpes de acero.
- Escudo de acero tipo heater con una cruz dorada, SIEMPRE sujeto con el brazo trasero (el izquierdo, el más alejado) y SIEMPRE visible delante del torso en TODOS los fotogramas.
- Espada larga de acero con guarda y pomo dorados en el brazo delantero (el derecho), con guantelete de acero.

FOTOGRAMAS: 7, en una fila y en este orden. El escudo aparece en los 7.
1. Reposo A: en guardia, escudo delante del pecho, espada en alto inclinada hacia delante.
2. Reposo B: idéntico al 1, pero el torso, la cabeza y los brazos 1 píxel más abajo. Las piernas no se mueven.
3. Ataque, preparación: espada echada atrás por encima del hombro y escudo delante.
4. Ataque, tajo: espada horizontal hacia delante y cuerpo algo adelantado; el escudo se queda pegado al costado.
5. Ataque, remate: espada terminando el arco, apuntando abajo y hacia delante.
6. Carga: espada levantada en vertical sobre la cabeza y escudo delante del pecho.
7. Recibe golpe: echado hacia atrás y encogido detrás del escudo.

Las piernas y los pies cambian lo mínimo entre fotogramas: el héroe nunca camina.

COMPOSICIÓN
- 7 celdas del mismo ancho, una junto a otra.
- En todas, los pies sobre la misma línea de suelo y en la misma posición horizontal dentro de la celda (en el tercio izquierdo, para que la espada tenga sitio a la derecha).
- Misma escala en todas: el caballero mide lo mismo en todos los fotogramas.
- Sin texto, sin números y sin sombra en el suelo.

ESTILO
- Pixel art limpio de 16 bits, como dibujado a mano en Aseprite a unos 64 px de alto y escalado con vecino más cercano. Todos los píxeles son cuadrados del mismo tamaño.
- Contorno negro de 1 píxel alrededor de toda la silueta.
- Luz desde arriba a la izquierda: borde claro arriba y a la izquierda, sombra abajo y a la derecha. Tres tonos por material.
- Sin antialiasing, sin degradados, sin píxeles sueltos, sin brillos, sin resplandores ni partículas.
- El borde entre el personaje y el fondo es duro: ningún píxel mezcla el magenta con el personaje.

PALETA ESTRICTA (solo estos colores):
- #0A0E0D negro: contornos
- #1B2925 oscuro: malla, cuero, tela oscura
- #3B5247 medio: sombra del acero
- #7A9380 claro: acero (color base de TODA la armadura, el escudo y la hoja de la espada)
- #D4DEC8 brillo: reflejos del acero
- #9C2E30 rojo: SOLO el penacho, la capa y un pañuelo al cuello
- #D6A248 oro: SOLO la cruz del escudo, la guarda y el pomo de la espada, y la hebilla
No uses verde.

FONDO: magenta puro #FF00FF, liso.
````

### Mensaje 2 — el mapa de piezas (sobre la imagen anterior)

````
Ahora, sobre la imagen exacta que acabas de generar, haz una versión de mapa de piezas. Debe tener las mismas 7 celdas, las mismas poses y la misma silueta píxel a píxel. No muevas, añadas ni quites nada; solo cambia los colores.

Pinta cada pieza con un único color plano, sin sombras ni contornos:
- CASCO (yelmo con su penacho): azul puro #0000FF
- PECHERA (peto, hombrera y cinturón con hebilla): amarillo puro #FFFF00
- BOTAS (grebas y escarpes, de la rodilla a los pies): cian puro #00FFFF
- ESCUDO (todo el escudo con su cruz): naranja puro #FF8000
- BRAZO (el brazo de la espada con el guantelete, más toda la espada): verde puro #00FF00
- CUERPO (todo lo demás: capa, cota de malla, faldón, muslos, brazo trasero): gris #808080

El fondo sigue en magenta #FF00FF. Sin contorno negro: cada píxel del caballero pasa a ser exactamente uno de esos 6 colores.
````

> Si el mapa no coincide con la tira (le cambia la pose o la silueta), pídele: *"No coincide con la imagen anterior. Repítelo calcando exactamente la misma silueta"*. Si sigue sin coincidir, pásamelo igual: lo uso solo como guía para recortar.

---

## Pantalla de título (fondo de la splash)

El logo "STICK RPG", los textos y las ascuas los dibuja el juego encima, así que la imagen es **solo el fondo, sin ningún texto**. La pantalla mide 240 × 135 (16:9).

````
Genera una ilustración de pixel art de 16 bits para el fondo de la pantalla de título de un RPG de fantasía medieval oscura. Formato horizontal 16:9 (1920×1080), pensado para reducirse a 240×135 píxeles: dibújalo como si cada píxel del juego fuera un bloque de 8×8, con bloques grandes y limpios.

ESCENA
Un caballero solitario de espaldas, visto de tres cuartos y pequeño en el encuadre, en el borde de una colina en la parte inferior izquierda. Lleva yelmo cerrado con penacho rojo, capa roja al viento, escudo con cruz dorada a la espalda y la espada apoyada en el suelo.
Mira hacia un valle brumoso. Al fondo, en el horizonte, a la derecha, la silueta de un castillo en ruinas con una sola ventana iluminada en dorado. Entre él y el castillo, un bosque de árboles muertos y retorcidos y, a media distancia, un cementerio con cruces y lápidas inclinadas que asoman entre la niebla.
Cielo nocturno con una luna llena enorme y pálida, algunas estrellas y jirones de nubes. Ascuas y chispas sueltas flotando en el aire.

COMPOSICIÓN (muy importante, encima irá texto)
- Mitad superior (franja central, del 5 % al 60 % de la altura): solo cielo oscuro, liso y con poco contraste. Ahí irá el logo; ni castillo, ni árboles, ni luna dentro de esa zona central. La luna puede ir arriba a la derecha, parcialmente fuera del centro.
- Centro, del 62 % al 85 % de la altura: zona oscura y tranquila (niebla, valle). Ahí irán dos líneas de texto centradas.
- Los elementos con detalle (caballero, árboles, castillo) van hacia los lados y la parte baja, dejando el centro despejado.
- Bordes algo más oscuros (viñeteado).
- Nada de texto, letras, títulos, logos ni firmas.

ESTILO
- Pixel art limpio de 16 bits, sin antialiasing, sin degradados suaves, sin desenfoque, sin texturas de ruido. Los degradados del cielo y la niebla se hacen con tramado (dithering) ordenado.
- Siluetas grandes y legibles con contorno oscuro. Luz de la luna desde arriba a la derecha.
- Ambiente: sombrío, silencioso y épico.

PALETA ESTRICTA (solo estos 8 colores):
#0A0E0D negro, #1B2925 oscuro, #3B5247 medio, #7A9380 claro, #D4DEC8 brillo (luna, reflejos), #9C2E30 rojo (capa, penacho), #D6A248 oro (ventana del castillo, cruz, ascuas), #9FE05A verde espectral (muy poco: fuegos fatuos en el cementerio).
````

---

## Logo del título

El juego lo anima: cae desde arriba, rebota con un temblor y un destello, y un reflejo blanco recorre las letras (solo sobre los píxeles que no son negros). Lienzo final: **208 × 72 px**, fondo transparente.

````
Diseña el logotipo de un videojuego en pixel art de 16 bits. El texto dice exactamente "STICK RPG". Revisa la ortografía letra a letra: S-T-I-C-K y debajo R-P-G. No añadas ninguna otra palabra.

FORMATO
- Imagen horizontal muy apaisada, proporción 208:72 (por ejemplo 1664×576), pensada para reducirse a 208×72 píxeles: dibújala como si cada píxel del juego fuera un bloque de 8×8, con bloques grandes y limpios.
- El logo ocupa casi todo el lienzo, con un margen pequeño y centrado.
- Fondo magenta puro #FF00FF, liso. Nada de escenario, cielo ni sombras sobre el fondo.

DISEÑO
- "STICK" en letras enormes, gruesas y anchas, de estilo medieval épico (serifas talladas en forma de cuña, como letras forjadas en metal), ocupando unos dos tercios de la altura.
- Las letras son de oro macizo con relieve biselado: cara superior iluminada, cara inferior en sombra y un borde superior brillante. Un par de muescas y arañazos de batalla.
- Contorno negro grueso alrededor de todas las letras, con una segunda línea oscura por dentro para dar volumen.
- Una espada larga de acero cruza horizontalmente por debajo de "STICK" como si fuera un subrayado: empuñadura y guarda doradas a la izquierda y punta afilada a la derecha, sobresaliendo un poco por los lados del texto.
- Debajo de la espada, "R P G" en letras más pequeñas, espaciadas, en acero pálido con contorno negro, flanqueadas por dos rombos dorados pequeños: ◆ R P G ◆.
- Detalle opcional: un goteo o una mancha de sangre roja en la punta de la espada, y unos pocos píxeles de brillo verde espectral en el filo.
- Debe leerse perfectamente a tamaño diminuto: formas grandes, contraste fuerte y nada de detalles finos de un píxel.

ESTILO
- Pixel art limpio, sin antialiasing, sin degradados suaves, sin desenfoque ni resplandores difusos, sin texturas de ruido.
- Luz desde arriba a la izquierda.

PALETA ESTRICTA (solo estos colores):
#0A0E0D negro (contornos), #1B2925 oscuro, #3B5247 medio, #7A9380 claro (acero), #D4DEC8 brillo (reflejos), #9C2E30 rojo (sangre), #D6A248 oro (letras, guarda, rombos), #9FE05A verde espectral (casi nada).
````

---

## Fondos de los biomas: formato usado (5 franjas en una imagen)

Al final se pidió **una imagen vertical por bioma con las 5 capas apiladas en franjas**, para que el estilo sea coherente: cielo (38 %), lejos (17 %), plano medio (26 %), suelo (8 %) y primer plano (11 %). Cada franja es una tira panorámica que se repite, con magenta en las franjas 2, 3 y 5. Se generaron en la conversación de ChatGPT del proyecto, el Bosque primero y los otros dos con "el mismo formato y estilo". Se guardan como `assets/ai/bg_bosque.png`, `bg_cementerio.png` y `bg_castillo.png`.

## Fondos de los biomas (por capas, método anterior)

Cada fondo tiene capas que se desplazan a distinta velocidad (parallax) y **se repiten en horizontal**. Se hace en **una conversación por bioma**: primero la escena completa como referencia y después cada capa por separado, calcando esa escena.

La niebla y las nubes las genera el juego, así que no hace falta pedirlas. Todo se dibuja con la paleta del Bosque: el juego cambia el tono en el Cementerio (violeta) y en el Castillo (cálido).

### Mensaje 1: escena completa (pegar el bloque común y el del bioma)

**Bloque común**

````
Genera un fondo de escenario de pixel art de 16 bits para un RPG de combate por turnos visto de lado, de fantasía medieval oscura. Formato 16:9 (1920×1080), pensado para reducirse a 240×135 píxeles: dibújalo como si cada píxel del juego fuera un bloque de 8×8, con bloques grandes y limpios.

Es solo el escenario: no hay personajes, ni texto, ni interfaz.

COMPOSICIÓN (muy importante)
- Vista lateral pura, como un plataformas 2D. El suelo es una franja plana y horizontal: su borde superior está al 83 % de la altura (fila 112 de 135) y llega hasta abajo.
- Encima del suelo combatirán dos personajes: uno a la izquierda (entre el 8 % y el 33 % del ancho) y otro a la derecha (entre el 58 % y el 96 %), de pie sobre el borde del suelo y de hasta el 75 % de la altura. Detrás de ellos el fondo debe ser oscuro y de poco contraste, sin objetos llamativos.
- La parte de arriba (del 3 % al 40 % de la altura) es cielo limpio; el juego le pone nubes.
- La imagen debe poder repetirse en horizontal: el borde izquierdo y el derecho encajan sin costura.

ESTILO
- Pixel art limpio, sin antialiasing, sin degradados suaves, sin desenfoque, sin texturas de ruido. Los degradados se hacen con tramado (dithering) ordenado.
- Profundidad por planos: lo lejano son siluetas oscuras y planas, y lo cercano tiene más detalle y contraste.
- Contorno oscuro en los objetos cercanos. Luz desde arriba a la izquierda.

PALETA ESTRICTA (solo estos 8 colores):
#0A0E0D negro, #1B2925 oscuro, #3B5247 medio, #7A9380 claro, #D4DEC8 brillo, #9C2E30 rojo (muy poco), #D6A248 oro (muy poco: antorchas, ventanas), #9FE05A verde espectral (muy poco: magia, fuegos fatuos).
````

**El Bosque**

````
ESCENARIO: un bosque siniestro de noche.
- Cielo: oscuro con una luna llena pálida arriba a la izquierda y algunas estrellas.
- Lejos: hileras de pinos y colinas en silueta oscura y plana, con la base envuelta en bruma.
- Plano medio: troncos de árboles muertos, retorcidos y sin hojas, con ramas como garras, separados entre sí y repartidos para no tapar a los personajes.
- Suelo: tierra oscura con raíces, piedras y matas de hierba.
- Primer plano: helechos y raíces muy oscuros, casi negros, solo en los bordes inferiores izquierdo y derecho.
- Detalles: unas pocas setas o luciérnagas en verde espectral.
````

**El Cementerio**

````
ESCENARIO: un cementerio abandonado de noche.
- Cielo: oscuro con una luna creciente y estrellas.
- Lejos: silueta de una iglesia en ruinas con campanario y cipreses altos, en el horizonte.
- Plano medio: verja de hierro forjado rota, lápidas inclinadas, cruces de piedra, un ángel de piedra decapitado y algún árbol muerto, repartidos para no tapar a los personajes.
- Suelo: tierra removida, losas rotas y hierba seca.
- Primer plano: lápidas caídas y zarzas muy oscuras, solo en los bordes inferiores izquierdo y derecho.
- Detalles: dos o tres fuegos fatuos en verde espectral.
````

**El Castillo**

````
ESCENARIO: el gran salón de un castillo en ruinas, en interiores (no hay cielo).
- Fondo (en lugar de cielo): muro de sillares de piedra oscura con ventanales góticos altos por los que entra luz pálida de luna.
- Lejos: arcos y columnas lejanas en penumbra.
- Plano medio: columnas gruesas de piedra, estandartes rojos rasgados que cuelgan y antorchas en la pared con fuego dorado, repartidos para no tapar a los personajes.
- Suelo: losas de piedra con grietas y una alfombra roja raída.
- Primer plano: cascotes y trozos de columna caídos, muy oscuros, solo en los bordes inferiores izquierdo y derecho.
- En este fondo la franja de arriba no tiene que estar vacía (no hay nubes).
````

### Mensajes 2 a 6: cada capa por separado (sobre la escena anterior)

Envíalos uno a uno en la misma conversación, después de la escena completa.

````
Ahora, a partir de la escena que acabas de generar, dame SOLO la capa del CIELO (o del muro del fondo en el Castillo): cielo, luna y estrellas, sin nada delante. Debe ser opaca y ocupar toda la imagen; donde antes había objetos, continúa el cielo. Mismo tamaño, misma paleta y mismo estilo.
````

````
Ahora dame SOLO la capa LEJANA de esa escena (siluetas del horizonte: colinas y pinos, la iglesia o los arcos lejanos), en su misma posición y tamaño, sobre fondo magenta puro #FF00FF. Todo lo demás es magenta. Debe repetirse en horizontal sin costura.
````

````
Ahora dame SOLO la capa del PLANO MEDIO (árboles muertos, lápidas y verja, o columnas, estandartes y antorchas), en su misma posición y tamaño, sobre fondo magenta puro #FF00FF. Nada del suelo ni del cielo. Debe repetirse en horizontal sin costura.
````

````
Ahora dame SOLO la capa del SUELO: la franja inferior desde el 80 % de la altura hasta abajo, sobre fondo magenta puro #FF00FF por encima. Debe repetirse en horizontal sin costura.
````

````
Ahora dame SOLO la capa del PRIMER PLANO: los elementos muy oscuros de los bordes inferiores izquierdo y derecho, en su misma posición y tamaño, sobre fondo magenta puro #FF00FF. El centro queda vacío (magenta).
````

> Pásame por bioma las 6 imágenes (la escena y las 5 capas) con nombres como `bosque_escena.png`, `bosque_cielo.png`, `bosque_lejos.png`, `bosque_medio.png`, `bosque_suelo.png` y `bosque_frente.png`. Si alguna capa no cuadra con la escena, la recorto yo a partir de la escena completa.

---

# Lote 2: enemigos y jefes nuevos

Mismo método que con los primeros enemigos. Para cada personaje se pega en un mensaje:

1. el **bloque de estilo común** (arriba),
2. las **reglas extra del lote 2** (justo debajo),
3. los **fotogramas para los enemigos** (arriba, los 8 de siempre),
4. el bloque del personaje.

La imagen se guarda en `assets/ai/` con el nombre de la tabla (fondo magenta). El importador ya conoce esos nombres y la altura de cada uno (`tools/art/import_ai.py`, `SOURCES`). Mientras falte una imagen, ese enemigo se ignora.

## Reglas extra del lote 2 (pegar siempre, después del estilo común)

```
REGLAS EXTRA (muy importantes)
- Cada fotograma es UNA sola figura conectada: nada suelto ni flotando separado del cuerpo. Sin flechas en vuelo, sin frascos lanzados, sin llamas sueltas, sin chispas, sin partículas ni insectos sueltos. El arma o la magia siempre van en la mano o pegadas al cuerpo (los efectos los añade el juego).
- La parte más baja del personaje (pies, patas, cola o jirones) toca la misma línea de suelo en los 8 fotogramas, aunque sea una criatura que vuela o flota.
- No lo hagas más ancho que 1,3 veces su altura en ningún fotograma: alas plegadas a medias, colas recogidas.
- Deja bastante separación de magenta entre fotogramas.
```

## Tabla de archivos

| Nº | Archivo | Personaje | Bioma | Alto (px) |
|---|---|---|---|---|
| 11 | `11_jabali_magenta.png` | Jabalí de colmillo negro | Bosque | 56 |
| 12 | `12_arquero_magenta.png` | Arquero furtivo | Bosque | 70 |
| 13 | `13_hombre_seta_magenta.png` | Hombre-seta | Bosque | 62 |
| 14 | `14_avispa_magenta.png` | Avispa gigante (reina del enjambre) | Bosque | 60 |
| 15 | `15_espiritu_roble_magenta.png` | Espíritu del roble | Bosque | 88 |
| 16 | `16_trasgo_magenta.png` | Trasgo ladrón | Bosque | 54 |
| 17 | `17_oso_magenta.png` | Oso pardo | Bosque | 84 |
| 18 | `18_druida_magenta.png` | Druida corrupto | Bosque | 76 |
| 19 | `19_serpiente_magenta.png` | Serpiente del pantano | Bosque | 60 |
| 20 | `20_cazador_magenta.png` | Cazador de cabezas | Bosque | 74 |
| 21 | `21_esqueleto_magenta.png` | Esqueleto | Cementerio | 70 |
| 22 | `22_zombi_magenta.png` | Zombi | Cementerio | 72 |
| 23 | `23_necrofago_magenta.png` | Necrófago | Cementerio | 62 |
| 24 | `24_fantasma_magenta.png` | Fantasma | Cementerio | 74 |
| 25 | `25_murcielago_magenta.png` | Murciélago vampiro | Cementerio | 60 |
| 26 | `26_sepulturero_magenta.png` | Sepulturero loco | Cementerio | 72 |
| 27 | `27_banshee_magenta.png` | Banshee | Cementerio | 78 |
| 28 | `28_gargola_magenta.png` | Gárgola | Cementerio | 76 |
| 29 | `29_sabueso_magenta.png` | Sabueso del inframundo | Cementerio | 58 |
| 30 | `30_momia_magenta.png` | Momia | Cementerio | 74 |
| 31 | `31_cuervo_magenta.png` | Cuervo de la muerte | Cementerio | 56 |
| 32 | `32_guardia_real_magenta.png` | Guardia real | Castillo | 76 |
| 33 | `33_armadura_animada_magenta.png` | Armadura animada | Castillo | 80 |
| 34 | `34_alquimista_magenta.png` | Alquimista | Castillo | 70 |
| 35 | `35_bufon_magenta.png` | Bufón siniestro | Castillo | 64 |
| 36 | `36_verdugo_magenta.png` | Verdugo | Castillo | 86 |
| 37 | `37_mago_torre_magenta.png` | Mago de la torre | Castillo | 80 |
| 38 | `38_dragon_joven_magenta.png` | Dragón joven | Castillo | 72 |
| 39 | `39_mimico_magenta.png` | Mímico | Castillo | 56 |
| 40 | `40_espia_magenta.png` | Espía de la corte | Castillo | 70 |
| 41 | `41_ballestero_magenta.png` | Ballestero | Castillo | 72 |
| 42 | `42_mastin_magenta.png` | Mastín de guerra | Castillo | 58 |
| 52 | `52_demonio_magenta.png` | Demonio | Cualquiera | 84 |
| 53 | `53_sucubo_magenta.png` | Súcubo | Cualquiera | 80 |
| 43 | `43_bruja_pantano_magenta.png` | **Jefa:** la Bruja del Pantano | Bosque | 98 |
| 44 | `44_gran_jabali_magenta.png` | **Jefe:** el Gran Jabalí | Bosque | 90 |
| 45 | `45_senor_trasgos_magenta.png` | **Jefe:** el Señor de los Trasgos | Bosque | 96 |
| 46 | `46_nigromante_magenta.png` | **Jefe:** el Nigromante | Cementerio | 104 |
| 47 | `47_viuda_negra_magenta.png` | **Jefa:** la Viuda Negra | Cementerio | 100 |
| 48 | `48_guardian_cripta_magenta.png` | **Jefe:** el Guardián de la Cripta | Cementerio | 106 |
| 49 | `49_reina_hechicera_magenta.png` | **Jefa:** la Reina Hechicera | Castillo | 102 |
| 50 | `50_dragon_ancestral_magenta.png` | **Jefe:** el Dragón Ancestral | Castillo | 100 |
| 51 | `51_rey_loco_magenta.png` | **Jefe:** el Rey Loco | Castillo | 104 |

---

## Bosque: enemigos normales

### 11 · Jabalí de colmillo negro

```
PERSONAJE: un jabalí salvaje enorme, unos 56 px de alto y 84 px de largo, a cuatro patas, mirando a la IZQUIERDA. Embiste con fuerza.
- Cuerpo macizo y alto de hombros, cuarto trasero más bajo; pelaje erizado oscuro (#1B2925 / #3B5247) con una cresta de cerdas a lo largo del lomo.
- Cabeza baja y grande, hocico ancho, dos colmillos curvos negros con la punta pálida (#D4DEC8).
- Ojo pequeño rojo (#9C2E30). Cicatrices claras en el costado.
- Patas cortas y fuertes con pezuñas negras.
- Ataque: baja la cabeza (preparación) y embiste hacia delante con los colmillos.
- Fotograma 8: escarba el suelo con una pata delantera, cabeza baja, ojo brillando en verde espectral.
```

### 12 · Arquero furtivo

```
PERSONAJE: un arquero bandido, unos 70 px de alto, mirando a la IZQUIERDA. Ágil y frágil.
- Capucha verde muy oscura (#1B2925) y capa corta hecha jirones; la cara es una sombra con un brillo de ojos claro.
- Jubón de cuero acolchado, brazal de cuero en el antebrazo, carcaj a la espalda con plumas rojas (#9C2E30).
- Pantalones y botas de cuero, cinturón con hebilla dorada pequeña.
- Un arco largo de madera oscura en la mano delantera.
- Ataque: preparación con la cuerda tensada y la flecha en el arco; golpe con la cuerda suelta y el arco hacia delante (la flecha NO se dibuja en vuelo).
- Fotograma 8: tensa el arco al máximo, la punta de la flecha brilla en verde espectral.
```

### 13 · Hombre-seta

```
PERSONAJE: un hombre-seta, criatura fúngica bípeda, unos 62 px de alto, mirando a la IZQUIERDA. Lento y raro.
- Sombrero de seta enorme y redondeado, más ancho que el cuerpo, rojo oscuro (#9C2E30) con manchas pálidas (#D4DEC8).
- Bajo el sombrero, láminas oscuras y dos ojos pequeños de luz espectral (#9FE05A).
- Tronco grueso y pálido (#7A9380) a modo de cuerpo, con brazos cortos como raíces y pies como bulbos.
- Algo de musgo oscuro en la base.
- Ataque: echa el cuerpo atrás y da un cabezazo con el sombrero hacia delante.
- Fotograma 8: el sombrero se hincha y las manchas brillan en verde espectral.
```

### 14 · Avispa gigante

```
PERSONAJE: una avispa gigante, reina de un enjambre, unos 60 px de alto y 72 px de largo, mirando a la IZQUIERDA. Rápida y venenosa. Vuela bajo: la punta del aguijón o las patas tocan la línea de suelo.
- Cuerpo segmentado: cabeza con mandíbulas, tórax peludo oscuro y abdomen grande con franjas doradas (#D6A248) y negras.
- Ojos compuestos grandes en verde espectral (#9FE05A).
- Dos alas translúcidas en tonos claros (#7A9380 / #D4DEC8), pegadas al cuerpo y medio abiertas.
- Patas finas colgando y un aguijón largo y afilado al final del abdomen.
- Caminar: aleteo (alas arriba y abajo). Ataque: dobla el abdomen bajo el cuerpo y clava el aguijón hacia delante.
- Fotograma 8: abdomen muy curvado, gota verde espectral en la punta del aguijón (pegada).
```

### 15 · Espíritu del roble

```
PERSONAJE: un espíritu del bosque, un roble viviente que camina, unos 88 px de alto, mirando a la IZQUIERDA. Muy resistente.
- Tronco grueso y nudoso como cuerpo (corteza #1B2925 / #3B5247 / #7A9380), con una cara tallada en la corteza: dos huecos con luz espectral (#9FE05A) y una boca de grieta.
- Copa pequeña de ramas sin apenas hojas sobre la cabeza, con algunas hojas oscuras.
- Dos brazos largos de rama que acaban en dedos-ramita.
- Piernas de raíces gruesas que se arrastran.
- Musgo y setas pequeñas en el tronco.
- Ataque: levanta un brazo-rama y lo descarga como un látigo hacia delante.
- Fotograma 8: las grietas de la corteza brillan en verde espectral, raíces tensas.
```

### 16 · Trasgo ladrón

```
PERSONAJE: un trasgo (goblin) ladrón, pequeño, unos 54 px de alto, mirando a la IZQUIERDA. Escurridizo.
- Cabeza grande con orejas puntiagudas larguísimas hacia atrás, nariz ganchuda, sonrisa con dientes torcidos, ojos dorados (#D6A248).
- Piel en tonos medios (#3B5247 / #7A9380).
- Chaleco de cuero roto y un saco de botín atado a la espalda, con una moneda dorada asomando.
- Pies descalzos y grandes. Postura encorvada.
- Un cuchillo curvo corto en la mano delantera.
- Ataque: se agacha y da un salto corto con una puñalada hacia delante.
- Fotograma 8: se frota las manos con el cuchillo en alto, ojos brillando.
```

### 17 · Oso pardo

```
PERSONAJE: un oso enorme erguido sobre las patas traseras, unos 84 px de alto, mirando a la IZQUIERDA. Mucha vida, golpes lentos y fuertes.
- Pelaje espeso y desgreñado en tonos oscuros (#1B2925 / #3B5247), algo más claro en el hocico.
- Cabeza ancha con orejas redondas, fauces con colmillos claros (#D4DEC8) y ojos pequeños rojos (#9C2E30).
- Cicatrices y una flecha rota clavada en el hombro.
- Garras largas y pálidas en las patas delanteras.
- Caminar: a dos patas, balanceándose pesadamente.
- Ataque: levanta una zarpa sobre la cabeza y la descarga en diagonal hacia delante.
- Fotograma 8: ruge con los dos brazos abiertos en alto.
```

### 18 · Druida corrupto

```
PERSONAJE: un druida corrompido por la magia oscura, unos 76 px de alto, mirando a la IZQUIERDA. Lanzador de maldiciones.
- Túnica larga con capucha de tela oscura (#1B2925) con ribetes de hojas secas; la cara es pálida y demacrada, con ojos de luz espectral (#9FE05A).
- Cornamenta pequeña de ramas saliendo de la capucha.
- Collar de huesos pequeños y un cinturón de cuerda.
- Báculo de madera retorcida con una piedra verde espectral engarzada en raíces.
- Pies descalzos, envueltos en raíces.
- Ataque: apunta el báculo hacia delante con el brazo extendido.
- Fotograma 8: alza el báculo con las dos manos; la piedra brilla mucho más.
```

### 19 · Serpiente del pantano

```
PERSONAJE: una serpiente gigante del pantano, unos 60 px de alto (con la cabeza erguida) y 90 px de largo, mirando a la IZQUIERDA. Venenosa.
- Cuerpo grueso enrollado en el suelo detrás, con la parte delantera alzada en forma de S.
- Escamas oscuras (#1B2925 / #3B5247) con dibujo de rombos claros (#7A9380) en el lomo y vientre pálido (#D4DEC8).
- Cabeza ancha y triangular con capuchón abierto, ojos rojos (#9C2E30) y lengua bífida roja.
- Colmillos largos y claros.
- Caminar: las curvas del cuerpo se desplazan (reptar). Ataque: echa la cabeza atrás y la lanza hacia delante con la boca abierta.
- Fotograma 8: boca muy abierta, gotas de veneno verde espectral pegadas a los colmillos.
```

### 20 · Cazador de cabezas

```
PERSONAJE: un mercenario cazador de recompensas, unos 74 px de alto, mirando a la IZQUIERDA. Peligroso cuando estás débil.
- Máscara de hierro sin rostro (#3B5247 / #7A9380) con una sola rendija con brillo rojo (#9C2E30).
- Capa de piel de lobo sobre los hombros, armadura de cuero tachonado.
- Cinturón con trofeos colgando (cráneos pequeños de animal y dientes, en #D4DEC8).
- Botas altas y guantes de cuero.
- Un hacha de mano en la mano delantera y otra colgada del cinturón.
- Ataque: hachazo horizontal de derecha a izquierda.
- Fotograma 8: hacha alzada sobre el hombro, cuerpo agazapado, rendija brillando en verde espectral.
```

## Cementerio: enemigos normales

### 21 · Esqueleto

```
PERSONAJE: un esqueleto guerrero, unos 70 px de alto, mirando a la IZQUIERDA. Se levanta una vez al morir.
- Huesos pálidos (#7A9380 / #D4DEC8) con contorno negro, costillas y columna visibles entre los huecos (magenta NO; los huecos van en #0A0E0D o #1B2925 para que sea una sola figura).
- Cráneo con cuencas que brillan en verde espectral (#9FE05A).
- Restos de armadura: una hombrera oxidada y un cinturón de cuero con jirones de tela roja (#9C2E30).
- Escudo redondo de madera rota en el brazo trasero y espada oxidada y mellada en el delantero.
- Ataque: tajo descendente con la espada.
- Fotograma 8: la mandíbula abierta (risa) y las cuencas brillando mucho más.
```

### 22 · Zombi

```
PERSONAJE: un zombi, unos 72 px de alto, mirando a la IZQUIERDA. Lento y duro.
- Cuerpo encorvado y desgarbado, piel en tonos medios y enfermizos (#3B5247 / #7A9380), con algún hueso claro a la vista.
- Cabeza ladeada, mandíbula colgando, ojos vacíos con un punto de luz espectral (#9FE05A).
- Ropa de campesino hecha jirones (camisa clara y pantalón oscuro), un pie descalzo.
- Manchas de sangre oscura (#9C2E30).
- Brazos extendidos hacia delante.
- Caminar: arrastrando un pie. Ataque: echa los brazos atrás y se abalanza con las dos manos hacia delante.
- Fotograma 8: echa la cabeza atrás con la boca muy abierta (gemido).
```

### 23 · Necrófago

```
PERSONAJE: un necrófago (ghoul), unos 62 px de alto, mirando a la IZQUIERDA, agazapado a cuatro patas casi siempre. Se cura al morder.
- Cuerpo flaco y nudoso, piel gris pálida (#7A9380) con costillas marcadas.
- Cabeza calva y alargada, orejas puntiagudas, boca enorme con dientes afilados, ojos blancos brillantes (#D4DEC8).
- Garras largas en manos y pies. Taparrabos de tela oscura.
- Sangre (#9C2E30) en la boca y las garras.
- Caminar: gatea con brazos y piernas. Ataque: salto con las garras por delante.
- Fotograma 8: erguido a medias, boca abierta, garras abiertas a los lados.
```

### 24 · Fantasma

```
PERSONAJE: un fantasma, unos 74 px de alto, mirando a la IZQUIERDA. Flota: los jirones de abajo tocan la línea de suelo.
- Figura encapuchada de sábana o sudario pálido (#7A9380 / #D4DEC8) que se estrecha hacia abajo en jirones ondulantes.
- Sin rostro: un hueco oscuro con dos ojos de luz espectral (#9FE05A).
- Dos brazos espectrales largos con manos de dedos finos.
- Contorno interior de 1 px en verde espectral dentro del contorno negro.
- Caminar: ondula y sube y baja unos píxeles (los jirones siguen tocando el suelo).
- Ataque: echa los brazos atrás y los lanza hacia delante con las manos abiertas.
- Fotograma 8: se hincha, el hueco del rostro se abre en un grito.
```

### 25 · Murciélago vampiro

```
PERSONAJE: un murciélago vampiro gigante, unos 60 px de alto, mirando a la IZQUIERDA. Rápido, roba vida. Vuela bajo: las patas tocan la línea de suelo.
- Cuerpo peludo pequeño y oscuro (#1B2925), cabeza con orejas enormes y puntiagudas, hocico de cerdo y colmillos claros.
- Ojos rojos brillantes (#9C2E30).
- Dos alas membranosas grandes (membrana #3B5247 con huesos claros #7A9380), medio desplegadas hacia arriba y hacia atrás.
- Patas con garras colgando bajo el cuerpo.
- Caminar: aleteo (alas arriba / alas abajo). Ataque: alas hacia atrás y mordisco hacia delante con la boca abierta.
- Fotograma 8: alas abiertas al máximo hacia arriba, boca abierta con sangre en los colmillos.
```

### 26 · Sepulturero loco

```
PERSONAJE: un sepulturero enloquecido, unos 72 px de alto, mirando a la IZQUIERDA. Su palazo aturde.
- Hombre jorobado y flaco con sombrero de copa abollado y abrigo largo raído (#1B2925).
- Cara pálida y hundida con una sonrisa desquiciada y ojos con brillo espectral (#9FE05A).
- Bufanda roja (#9C2E30) deshilachada, guantes sin dedos.
- Un farol pequeño colgado del cinturón con luz dorada (#D6A248).
- Una pala grande de hierro sujeta a dos manos.
- Ataque: pala alzada por encima del hombro y palazo plano hacia delante.
- Fotograma 8: pala en alto con las dos manos, risa con la boca abierta.
```

### 27 · Banshee

```
PERSONAJE: una banshee, espíritu femenino que grita, unos 78 px de alto, mirando a la IZQUIERDA. Flota: el bajo del vestido toca la línea de suelo.
- Mujer espectral delgada, piel muy pálida (#D4DEC8), cabello larguísimo y blanco que ondea hacia atrás y arriba.
- Rostro demacrado con boca muy abierta y ojos de luz espectral (#9FE05A).
- Vestido de luto oscuro (#1B2925 / #3B5247) que se deshace en jirones abajo.
- Manos huesudas con uñas largas.
- Diferente de la Dama Espectral: más pequeña, oscura y salvaje, sin cadenas ni velo.
- Ataque: echa la cabeza atrás y lanza un grito hacia delante con los brazos abiertos (sin ondas dibujadas).
- Fotograma 8: se encoge sobre sí misma, el cabello erizado, brillo verde en la boca.
```

### 28 · Gárgola

```
PERSONAJE: una gárgola de piedra viva, unos 76 px de alto, mirando a la IZQUIERDA, agazapada sobre dos patas. A veces se vuelve de piedra.
- Cuerpo de piedra gris (#3B5247 / #7A9380) con grietas, musgo oscuro en algunos bordes.
- Cabeza de demonio con cuernos cortos hacia atrás, orejas puntiagudas, boca de colmillos y ojos espectrales (#9FE05A).
- Alas de murciélago de piedra plegadas a la espalda.
- Brazos largos con garras, cola terminada en punta enroscada en el suelo.
- Caminar: salta a cuatro patas, alas pegadas. Ataque: zarpazo con las garras hacia delante.
- Fotograma 8: completamente de piedra, encogida y con las alas cerrándola como un capullo; ojos apagados (sin verde).
```

### 29 · Sabueso del inframundo

```
PERSONAJE: un sabueso infernal, unos 58 px de alto y 86 px de largo, a cuatro patas, mirando a la IZQUIERDA. Cada vez más furioso.
- Perro enorme y esquelético, piel negra pegada a los huesos (#0A0E0D / #1B2925), costillas marcadas.
- Cráneo medio descarnado (#7A9380 / #D4DEC8) con ojos de fuego verde espectral (#9FE05A).
- Una crin de llamas verdes espectrales pegada al lomo y al cuello, como pelo (sin chispas sueltas).
- Cadena rota y collar de pinchos al cuello.
- Cola fina y pelada.
- Caminar: trote. Ataque: se agacha y se lanza con las fauces abiertas.
- Fotograma 8: aúlla con la cabeza en alto, la crin de llamas mucho más alta.
```

### 30 · Momia

```
PERSONAJE: una momia, unos 74 px de alto, mirando a la IZQUIERDA. Sus vendas atrapan.
- Cuerpo envuelto en vendas pálidas (#7A9380 / #D4DEC8) con franjas oscuras entre vuelta y vuelta; algunas vendas cuelgan sueltas pero pegadas al cuerpo.
- Por un hueco de la cara asoma un ojo de luz espectral (#9FE05A).
- Un amuleto dorado (#D6A248) en el pecho y restos de un faldón real oscuro.
- Brazos extendidos al frente.
- Caminar: rígido y arrastrando los pies. Ataque: lanza el brazo hacia delante con una venda larga que sale de la mano (pegada a la mano).
- Fotograma 8: brazos en cruz, las vendas sueltas ondeando, el amuleto brilla en verde.
```

### 31 · Cuervo de la muerte

```
PERSONAJE: un cuervo gigante, unos 56 px de alto, mirando a la IZQUIERDA, posado en el suelo. Débil pero no viene solo.
- Plumaje negro con reflejos oscuros (#0A0E0D / #1B2925 / #3B5247), plumas erizadas en el cuello.
- Pico grande, curvo y afilado, gris pálido (#7A9380).
- Ojo de luz espectral (#9FE05A).
- Patas fuertes con garras.
- Caminar: saltitos con las alas medio abiertas. Ataque: picotazo hacia delante con las alas hacia atrás.
- Fotograma 8: alas abiertas hacia arriba (sin salirse del ancho máximo) y pico abierto graznando.
```

## Castillo: enemigos normales

### 32 · Guardia real

```
PERSONAJE: un guardia real del castillo maldito, unos 76 px de alto, mirando a la IZQUIERDA. Bloquea ataques.
- Yelmo abierto con nasal y cara cadavérica en sombra, ojos de luz espectral (#9FE05A).
- Cota de malla (#3B5247) con sobreveste roja (#9C2E30) y un emblema dorado (#D6A248) de corona en el pecho.
- Escudo grande y rectangular (pavés) de acero delante del cuerpo, con la corona dorada pintada.
- Lanza larga (alabarda) en la mano delantera.
- Ataque: estocada con la alabarda hacia delante, por encima del escudo.
- Fotograma 8: agachado tras el escudo, totalmente cubierto, el emblema brillando.
```

### 33 · Armadura animada

```
PERSONAJE: una armadura completa vacía que se mueve sola, unos 80 px de alto, mirando a la IZQUIERDA. Inmune a críticos.
- Armadura de placas antigua de acero (#3B5247 / #7A9380 / #D4DEC8) con remaches; algunas juntas abiertas por las que se ve oscuridad.
- Yelmo de cubo (yelmo de tonel) con rendija en cruz; dentro de la rendija, solo luz espectral (#9FE05A).
- Hilos de luz espectral en las juntas de codos y rodillas (dentro de la figura).
- Tabardo azul oscuro (usa #1B2925) roto.
- Mangual (bola de pinchos con cadena corta) en la mano delantera.
- Ataque: voltea el mangual por encima y lo descarga hacia delante.
- Fotograma 8: el yelmo levitando un píxel sobre el cuello y la luz de las juntas mucho más intensa.
```

### 34 · Alquimista

```
PERSONAJE: un alquimista loco, unos 70 px de alto, mirando a la IZQUIERDA. Imprevisible.
- Hombre bajo con gafas de lentes redondas que brillan en verde espectral (#9FE05A) y pelo blanco despeinado.
- Delantal de cuero oscuro sobre túnica, con quemaduras y manchas.
- Cinturón lleno de frascos pequeños: uno rojo (#9C2E30), uno verde espectral, uno dorado (#D6A248).
- Mochila con un alambique de cobre a la espalda.
- Un frasco burbujeante en la mano delantera.
- Ataque: echa el brazo atrás con el frasco y lo lanza hacia delante (el frasco sigue en la mano en el fotograma del golpe).
- Fotograma 8: alza el frasco por encima de la cabeza, el frasco brilla en verde.
```

### 35 · Bufón siniestro

```
PERSONAJE: un bufón del rey, siniestro, unos 64 px de alto, mirando a la IZQUIERDA. Imprevisible.
- Gorro de tres puntas con cascabeles dorados (#D6A248).
- Máscara blanca (#D4DEC8) con sonrisa enorme pintada y ojos negros con un punto espectral (#9FE05A).
- Traje a rombos rojo (#9C2E30) y oscuro (#1B2925), cuello de gorguera, zapatos puntiagudos curvados.
- Cuerpo delgado y flexible.
- Un cetro de bufón (palo con una cabeza de muñeco) en la mano delantera.
- Caminar: pasos saltarines. Ataque: golpe con el cetro hacia delante.
- Fotograma 8: haciendo el pino (cabeza abajo) apoyado en una mano, con los pies arriba.
```

### 36 · Verdugo

```
PERSONAJE: un verdugo, unos 86 px de alto, mirando a la IZQUIERDA. Remata a los débiles.
- Hombre enorme y corpulento, torso desnudo y musculoso en tonos medios (#3B5247 / #7A9380).
- Capucha negra puntiaguda que le tapa toda la cabeza, con dos agujeros y ojos rojos (#9C2E30).
- Pantalones oscuros, cinturón ancho de cuero con hebilla, brazales de cuero.
- Hacha de verdugo enorme de hoja ancha, sujeta a dos manos, con manchas de sangre en el filo.
- Ataque: hacha alzada por encima de la cabeza y golpe vertical hacia delante y abajo.
- Fotograma 8: hacha apoyada en el hombro, cabeza ladeada mirando al héroe, filo brillando en verde espectral.
```

### 37 · Mago de la torre

```
PERSONAJE: un mago oscuro del castillo, unos 80 px de alto, mirando a la IZQUIERDA. Escudo mágico.
- Sombrero picudo de ala ancha, larga barba gris (#7A9380) y ojos de luz espectral (#9FE05A).
- Túnica larga oscura (#1B2925) con estrellas y runas doradas (#D6A248) bordadas.
- Libro de hechizos encadenado al cinturón.
- Báculo alto con un orbe engarzado en una garra dorada.
- Ataque: apunta el báculo hacia delante con el brazo extendido.
- Fotograma 8: alza los dos brazos con el báculo en alto; el orbe y las runas de la túnica brillan.
```

### 38 · Dragón joven

```
PERSONAJE: un dragón joven, unos 72 px de alto y 94 px de largo, a cuatro patas, mirando a la IZQUIERDA. Aliento de fuego cada tres turnos.
- Escamas rojo oscuro (#9C2E30) en el lomo con sombras oscuras, vientre claro a placas (#7A9380 / #D4DEC8).
- Cabeza con dos cuernos hacia atrás, hocico con colmillos, ojos dorados (#D6A248).
- Alas membranosas pequeñas plegadas sobre el lomo.
- Cola larga recogida detrás, sin pasar del ancho máximo.
- Garras claras.
- Caminar: andar a cuatro patas. Ataque: mordisco hacia delante con el cuello estirado.
- Fotograma 8: cuello echado atrás, pecho inflado y un brillo dorado en la boca y la garganta (sin llamas fuera).
```

### 39 · Mímico

```
PERSONAJE: un mímico, un cofre del tesoro vivo, unos 56 px de alto y 66 px de ancho, mirando a la IZQUIERDA. Parece botín.
- Cofre de madera oscura (#1B2925 / #3B5247) con refuerzos de hierro y bordes y cerradura dorados (#D6A248).
- La tapa abierta como una boca: filas de dientes pálidos (#D4DEC8) y una lengua roja enorme (#9C2E30).
- Dentro, oscuridad con dos ojos espectrales (#9FE05A).
- Cuatro patas cortas y peludas o de insecto bajo el cofre.
- Fotograma 3 y 4 (reposo): tapa entreabierta con los dientes asomando.
- Ataque: tapa muy abierta y se lanza mordiendo hacia delante.
- Fotograma 8: tapa cerrada del todo, parece un cofre normal, solo asoma la punta de la lengua.
```

### 40 · Espía de la corte

```
PERSONAJE: una asesina de la corte, unos 70 px de alto, mirando a la IZQUIERDA. Primer golpe siempre crítico.
- Mujer ágil con capucha y antifaz negro, solo se ven los ojos claros (#D4DEC8).
- Ropa ajustada oscura (#0A0E0D / #1B2925) con un fajín rojo (#9C2E30) atado a la cintura cuyas puntas caen.
- Brazales de cuero, botas blandas.
- Una daga curva en cada mano, con la hoja clara y el filo verde espectral (#9FE05A).
- Postura agazapada y baja, lista para saltar.
- Ataque: cruza las dagas atrás y da un doble tajo hacia delante.
- Fotograma 8: dagas cruzadas delante de la cara, agachada al máximo.
```

### 41 · Ballestero

```
PERSONAJE: un ballestero del castillo, unos 72 px de alto, mirando a la IZQUIERDA. Su virote ignora la defensa.
- Capacete redondo de acero (#7A9380) y gorjal; cara en sombra con ojos espectrales (#9FE05A).
- Gambesón acolchado oscuro y sobreveste roja (#9C2E30) con la corona dorada.
- Carcaj de virotes al cinturón.
- Ballesta pesada de madera y acero sujeta a dos manos a la altura del pecho, con un virote cargado.
- Ataque: preparación apuntando; golpe con el retroceso del disparo, la ballesta un poco hacia arriba (sin virote en vuelo).
- Fotograma 8: hinca una rodilla y tensa la ballesta con un torno; la punta del virote brilla en verde.
```

### 42 · Mastín de guerra

```
PERSONAJE: un mastín de guerra con armadura, unos 58 px de alto y 84 px de largo, a cuatro patas, mirando a la IZQUIERDA. Rápido.
- Perro enorme y musculoso, pelaje corto oscuro (#1B2925 / #3B5247), papada y hocico corto y ancho.
- Armadura de placas de acero (#7A9380) sobre el lomo y la cabeza, con una gualdrapa roja (#9C2E30) con borde dorado.
- Collar de pinchos de acero.
- Ojos rojos y colmillos claros.
- Caminar: trote. Ataque: salto con mordisco hacia delante.
- Fotograma 8: plantado, cabeza baja, dientes al aire y babeando, ojos en verde espectral.
```

### 52 · Demonio

```
PERSONAJE: un demonio menor, unos 84 px de alto, mirando a la IZQUIERDA. Fuerte y salvaje.
- Cuerpo musculoso y encorvado, piel rojo oscuro (#9C2E30) con sombras oscuras (#1B2925) y placas óseas claras (#7A9380) en hombros y antebrazos.
- Cabeza con dos cuernos grandes curvados hacia atrás, orejas puntiagudas, fauces con colmillos y ojos dorados (#D6A248) brillantes.
- Alas de murciélago pequeñas y rotas, plegadas a la espalda.
- Patas de cabra con pezuñas negras, cola larga terminada en punta de flecha, recogida detrás.
- Garras largas; en la mano delantera, un tridente de hierro negro.
- Ataque: estocada con el tridente hacia delante.
- Fotograma 8: echa los brazos atrás rugiendo, los cuernos y los ojos brillan en verde espectral.
```

### 53 · Súcubo

```
PERSONAJE: una súcubo, demonio femenino seductor, unos 80 px de alto, mirando a la IZQUIERDA. Ágil y traicionera, roba vida.
- Figura muy femenina, sexy, curvilínea y voluptuosa: silueta de reloj de arena muy marcada, caderas anchas y cintura estrecha, postura seductora y desafiante con la cadera ladeada.
- Piel violácea oscura (usa #3B5247 / #7A9380), melena larga negra que ondea hacia atrás.
- Dos cuernos curvados hacia atrás, ojos verde espectral (#9FE05A), labios rojos (#9C2E30) y una sonrisa pícara con colmillos.
- Atuendo ajustado de cuero negro (corsé con ribetes dorados #D6A248, guantes largos y botas altas de tacón), al estilo de una heroína de fantasía clásica; sin desnudos.
- Alas de murciélago grandes medio plegadas a la espalda y una cola fina terminada en punta de corazón, recogida.
- Garras largas; un látigo corto enroscado en la mano delantera (pegado a la mano).
- Ataque: zarpazo con las garras hacia delante con un giro de cadera.
- Fotograma 8: alas abiertas a medias, lanza un beso con una mano y un brillo verde espectral pegado a la palma.
```

---

## Jefes nuevos (90–108 px de alto)

### 43 · La Bruja del Pantano (jefa del Bosque)

```
PERSONAJE: la Bruja del Pantano, unos 98 px de alto, mirando a la IZQUIERDA. Jefa.
- Vieja alta y encorvada, piel verdosa pálida (#7A9380), nariz larguísima y ganchuda, sonrisa de dientes rotos, ojos de luz espectral (#9FE05A).
- Sombrero puntiagudo enorme y torcido, con setas y musgo creciendo en el ala.
- Vestido harapiento oscuro (#1B2925) con algas colgando, chal de piel de rana.
- Un sapo grande sentado en su hombro trasero.
- Cucharón de madera largo en una mano y, colgando del cinturón, un caldero pequeño humeante (sin humo suelto).
- Pies descalzos con dedos largos.
- Ataque: golpe con el cucharón hacia delante, el sapo saca la lengua roja (#9C2E30) a la vez.
- Fotograma 8: alza los brazos, el cucharón y el sapo brillan en verde espectral.
- La cara debe quedar por debajo del 15 % superior de la imagen (el sombrero sí puede subir).
```

### 44 · El Gran Jabalí (jefe del Bosque)

```
PERSONAJE: el Gran Jabalí, el rey de los jabalíes, unos 90 px de alto y 124 px de largo, a cuatro patas, mirando a la IZQUIERDA. Jefe. Distinto del jabalí normal: mucho más grande y antiguo.
- Cuerpo colosal y peludo (#1B2925 / #3B5247) con el lomo cubierto de musgo, helechos y flechas rotas clavadas.
- Cuatro colmillos enormes curvados hacia arriba, pálidos (#D4DEC8) y uno roto.
- Ojo izquierdo cerrado por una cicatriz; el otro brilla en verde espectral (#9FE05A).
- Una corona de raíces y huesos entre las orejas, con algún toque dorado (#D6A248).
- Pezuñas negras gigantes.
- Caminar: pasos pesados. Ataque: baja la cabeza y embiste con los colmillos.
- Fotograma 8: se encabrita sobre las patas traseras (sin pasar del ancho máximo), cabeza alta, ojo brillando.
```

### 45 · El Señor de los Trasgos (jefe del Bosque)

```
PERSONAJE: el Señor de los Trasgos, un trasgo gordo y enorme de pie, unos 96 px de alto, mirando a la IZQUIERDA. Jefe.
- Trasgo gigantesco y obeso, piel en tonos medios (#3B5247 / #7A9380), orejas larguísimas caídas con aros dorados.
- Corona dorada (#D6A248) robada, demasiado pequeña para su cabeza y torcida.
- Capa roja (#9C2E30) de terciopelo raída con ribete de piel.
- Collares de monedas y anillos de oro; barriga al aire.
- Nariz enorme, sonrisa con colmillos, ojos amarillos dorados.
- Un cetro-maza (una maza de hierro con pinchos) en la mano delantera y una bolsa de oro en la trasera.
- Ataque: mazazo de arriba abajo hacia delante.
- Fotograma 8: alza la bolsa de oro y la maza a la vez, riendo, la corona brillando.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

### 46 · El Nigromante (jefe del Cementerio)

```
PERSONAJE: el Nigromante, unos 104 px de alto, mirando a la IZQUIERDA. Jefe que resucita a los muertos.
- Hechicero alto y esquelético, cara pálida y chupada con barba negra y fina, ojos de fuego espectral (#9FE05A).
- Capucha y túnica larga negra (#0A0E0D / #1B2925) con un cuello alto en punta y runas verdes bordadas en el bajo.
- Hombreras hechas de cráneos (#7A9380 / #D4DEC8).
- Un libro abierto flotando pegado a su mano trasera (tocándola).
- Báculo con un cráneo en la punta en la mano delantera.
- Ataque: apunta el báculo hacia delante.
- Fotograma 8: alza el báculo y la otra mano con la palma hacia arriba; del suelo, pegadas a sus pies, asoman dos manos esqueléticas.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

### 47 · La Viuda Negra (jefa del Cementerio)

```
PERSONAJE: la Viuda Negra, una dama vampira, unos 100 px de alto, mirando a la IZQUIERDA. Jefa que roba vida.
- Mujer alta y elegante, piel muy pálida (#D4DEC8), labios rojos (#9C2E30), colmillos y ojos rojos brillantes.
- Pelo negro recogido en un moño alto con una peineta dorada (#D6A248) y un velo negro corto.
- Vestido de luto victoriano negro (#0A0E0D / #1B2925) con cuello alto, mangas abombadas y falda larga con cola; forro rojo.
- Capa con forma de alas de murciélago a la espalda (plegada).
- Abanico de encaje negro en una mano; en la otra, uñas largas como garras.
- Ataque: zarpazo con la mano de las garras hacia delante.
- Fotograma 8: abre la capa como alas de murciélago (sin pasar del ancho máximo), colmillos a la vista.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

### 48 · El Guardián de la Cripta (jefe del Cementerio)

```
PERSONAJE: el Guardián de la Cripta, una estatua funeraria gigante viva, unos 106 px de alto, mirando a la IZQUIERDA. Jefe muy resistente.
- Caballero de piedra (#3B5247 / #7A9380 / #D4DEC8) como los de los sepulcros: yelmo cerrado, armadura de placas tallada, manos sobre el pomo de la espada.
- Grietas y musgo oscuro; un trozo del hombro roto que deja ver un núcleo espectral (#9FE05A).
- Luz espectral en la rendija del yelmo.
- Escudo de piedra con un cráneo tallado en el brazo trasero.
- Espadón de piedra enorme en la mano delantera.
- Base del pedestal pegada aún a un pie (un bloque de piedra).
- Ataque: espadón alzado y golpe vertical hacia delante.
- Fotograma 8: la espada clavada en el suelo delante, las dos manos en el pomo, grietas brillando en verde.
```

### 49 · La Reina Hechicera (jefa del Castillo)

```
PERSONAJE: la Reina Hechicera, unos 102 px de alto, mirando a la IZQUIERDA. Jefa.
- Mujer alta y regia, piel pálida (#D4DEC8), mirada fría con ojos de luz espectral (#9FE05A).
- Corona alta de puntas doradas (#D6A248) con una gema verde.
- Melena negra larga que cae por la espalda.
- Vestido largo negro y rojo (#9C2E30) con cuello alto abierto en abanico (como de encaje rígido) detrás de la cabeza.
- Capa larga real con borde dorado.
- Cetro dorado con un orbe espectral en la mano delantera.
- Ataque: apunta el cetro hacia delante con el brazo extendido.
- Fotograma 8: alza el cetro en alto con las dos manos, la gema de la corona y el orbe brillan.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

### 50 · El Dragón Ancestral (jefe del Castillo)

```
PERSONAJE: el Dragón Ancestral, unos 100 px de alto y 124 px de ancho, mirando a la IZQUIERDA. Jefe. Compacto para caber: sentado sobre las patas traseras, alas plegadas y cola enroscada a sus pies.
- Escamas antiguas negras y gris oscuro (#0A0E0D / #1B2925 / #3B5247) con el borde claro (#7A9380); vientre de placas pálidas.
- Cabeza enorme con una corona natural de cuernos largos hacia atrás, barbas de espinas y ojos dorados (#D6A248).
- Cicatrices, escamas que faltan y una lanza rota clavada en el cuello.
- Alas membranosas rotas, plegadas sobre la espalda.
- Garras delanteras enormes apoyadas en el suelo.
- Caminar: avanza un paso con las patas delanteras. Ataque: lanza la cabeza hacia delante y abajo con las fauces abiertas.
- Fotograma 8: cuello alzado, pecho inflado y grietas brillando en dorado y la boca con brillo dorado (sin fuego fuera de la boca).
```

### 51 · El Rey Loco (jefe del Castillo)

```
PERSONAJE: el Rey Loco, unos 104 px de alto, mirando a la IZQUIERDA. Jefe.
- Rey viejo y demacrado, piel gris pálida (#7A9380), barba larga y blanca enmarañada, ojos desorbitados con luz espectral (#9FE05A) y sonrisa desquiciada.
- Corona dorada (#D6A248) enorme y abollada, inclinada a un lado.
- Manto real rojo (#9C2E30) con borde de armiño (blanco con puntos negros), lleno de rotos.
- Debajo, una armadura antigua dorada y oxidada.
- Cetro-espada: una espada larga ceremonial dorada en la mano delantera; en la trasera, un cáliz.
- Ataque: tajo amplio hacia delante con la espada.
- Fotograma 8: alza el cáliz y la espada riendo con la cabeza echada atrás.
- La cara debe quedar por debajo del 15 % superior de la imagen.
```

> Cuando tengas las imágenes, déjalas en la raíz del repo (o en `assets/ai/` con su nombre) y dime cuáles entran en el juego: yo las renombro, las importo, les doy stats y mecánica y las meto en el bioma que toque.

---

# Biomas nuevos (fondos en 5 franjas)

Mismo formato que los fondos actuales (`assets/ai/bg_*.png`): **una imagen vertical por bioma con las 5 capas apiladas en franjas**. Se piden en la conversación de ChatGPT del proyecto, una imagen por mensaje: el bloque común más el bloque del bioma. Para los siguientes basta con decir «mismo formato y estilo» y pegar el bloque del bioma.

Todo se dibuja con la **paleta del Bosque** (los verdes de siempre). El juego le pone a cada bioma su propio tono (tabla de paletas al final de esta sección), así que no hace falta que la nieve salga azul ni la lava roja: basta con que los tonos claros y oscuros estén en su sitio. Las únicas excepciones son el rojo, el oro y el verde espectral, que el juego respeta tal cual.

| Archivo | Bioma | Encaja con (lote 2) |
|---|---|---|
| `bg_pantano.png` | Pantano | Serpiente, Hombre-seta, Avispa, Bruja del Pantano |
| `bg_cueva.png` | Cueva / mina | Trasgo, Murciélago, Gárgola, Mímico, Señor de los Trasgos |
| `bg_montana.png` | Montañas nevadas | Oso, Cuervo, Cazador, Huargo, Gran Jabalí |
| `bg_desierto.png` | Desierto y ruinas | Momia, Avispa, Alquimista, Guardián de la Cripta |
| `bg_volcan.png` | Volcán | Dragón joven, Sabueso del inframundo, Dragón Ancestral |
| `bg_costa.png` | Costa del naufragio | Esqueleto, Cuervo, Ballestero, Viuda Negra |
| `bg_catacumbas.png` | Catacumbas | Esqueleto, Necrófago, Momia, Nigromante |
| `bg_torre.png` | Torre del mago | Mago de la torre, Armadura animada, Alquimista, Reina Hechicera |

## Bloque común (pegar con el primer bioma)

````
Genera el fondo de un escenario de pixel art de 16 bits para un RPG de combate por turnos visto de lado, de fantasía medieval oscura. Es solo el escenario: sin personajes, sin texto y sin interfaz.

FORMATO (muy importante)
Una sola imagen VERTICAL de 1024×1536 con 5 franjas horizontales apiladas, cada una a todo el ancho y separadas por una línea de magenta puro #FF00FF de al menos 12 px de alto. Cada franja es una capa del escenario (parallax) que se repetirá en horizontal: su borde izquierdo y su borde derecho deben encajar sin costura.

1. CIELO (franja superior, ~38 % de la altura): opaca, ocupa toda la franja. Cielo (o el muro del fondo si es un interior). Liso y oscuro en la parte de abajo.
2. LEJOS (~17 %): siluetas planas y oscuras del horizonte, apoyadas en el borde inferior de la franja, sobre fondo magenta #FF00FF.
3. PLANO MEDIO (~26 %): los objetos del escenario (árboles, rocas, columnas…) apoyados en el borde inferior de la franja, sobre fondo magenta. Repartidos y separados; en el centro de la franja, menos cosas y más bajas, porque delante combatirán los personajes.
4. SUELO (~8 %): una tira de suelo continua y opaca vista de lado, con su borde superior irregular, sobre magenta por encima.
5. PRIMER PLANO (~11 %): elementos muy oscuros, casi negros, SOLO en el extremo izquierdo y en el extremo derecho, apoyados en el borde inferior, sobre magenta. El centro queda vacío (magenta).

ESTILO
- Pixel art limpio, como dibujado a ~240 px de ancho y escalado con vecino más cercano: bloques grandes y cuadrados. Sin antialiasing, sin degradados suaves, sin desenfoque, sin texturas de ruido. Los degradados del cielo, con tramado (dithering) ordenado.
- Profundidad por planos: lo lejano, plano y oscuro; lo cercano, con más detalle y contraste. Contorno oscuro en los objetos del plano medio. Luz desde arriba a la izquierda.
- Ningún objeto suelto flotando dentro de las franjas 2 a 5: todo apoyado en el borde inferior.

PALETA ESTRICTA (solo estos 8 colores, aunque el lugar sea de otro color; el juego lo recolorea después):
#0A0E0D negro, #1B2925 oscuro, #3B5247 medio, #7A9380 claro, #D4DEC8 brillo, #9C2E30 rojo (muy poco), #D6A248 oro (muy poco: fuego, ventanas, tesoro), #9FE05A verde espectral (muy poco: magia, fuegos fatuos, cristales).
````

## Bloques de cada bioma

### Pantano

````
ESCENARIO: un pantano brumoso de noche.
- Cielo: oscuro y bajo, con una luna velada por la bruma, sin estrellas apenas.
- Lejos: siluetas de cipreses de pantano y sauces llorones sobre agua quieta, con una cabaña sobre pilotes a un lado.
- Plano medio: troncos podridos con raíces aéreas, juncos altos y espadañas, un muelle de tablas roto y un poste con una calavera de animal.
- Suelo: barro oscuro y charcos con reflejos claros, nenúfares y raíces.
- Primer plano: juncos y raíces casi negros en los extremos.
- Detalles: unas pocas luciérnagas o fuegos fatuos en verde espectral, pegados a los juncos.
````

### Cueva / mina

````
ESCENARIO: el interior de una mina abandonada y una caverna profunda (no hay cielo).
- Cielo (en su lugar): pared de roca oscura de la caverna con vetas claras y estalactitas colgando del borde superior.
- Lejos: galerías y arcos de roca en penumbra, con la silueta de un puente de madera colgante.
- Plano medio: vigas de madera de entibado, una vagoneta volcada sobre raíles, estalagmitas y racimos de cristales que brillan en verde espectral, un farol colgado con luz dorada.
- Suelo: roca y grava con raíles de mina.
- Primer plano: rocas y estalagmitas casi negras en los extremos.
````

### Montañas nevadas

````
ESCENARIO: un paso de montaña nevado de noche.
- Cielo: oscuro y estrellado, con una aurora en bandas onduladas de verde espectral muy tenue cruzándolo.
- Lejos: cordillera de picos afilados con las cumbres blancas (#D4DEC8) y laderas oscuras.
- Plano medio: abetos cargados de nieve, rocas con nieve encima, un poste indicador de madera y los restos de un campamento (tienda rota y una hoguera apagada).
- Suelo: nieve pisada (tonos claros) con piedras oscuras asomando.
- Primer plano: rocas y abetos pequeños casi negros, con un borde de nieve, en los extremos.
````

### Desierto y ruinas

````
ESCENARIO: un desierto con ruinas antiguas al anochecer.
- Cielo: despejado y oscuro, con una luna enorme muy baja en el horizonte y muchas estrellas.
- Lejos: dunas onduladas y la silueta de una pirámide escalonada y un obelisco.
- Plano medio: columnas rotas de un templo enterradas en la arena, una estatua colosal con cabeza de chacal caída, cactus o palmeras secas y un cráneo de animal enorme.
- Suelo: arena con ondas de viento y losas de piedra asomando.
- Primer plano: rocas y cráneos casi negros en los extremos.
- Detalles: dos antorchas en las columnas con fuego dorado.
````

### Volcán

````
ESCENARIO: la ladera de un volcán activo, de noche.
- Cielo: oscuro y cargado de humo en bandas, con resplandor en la parte baja (más claro abajo, con tramado).
- Lejos: el cono del volcán con ríos de lava en dorado (#D6A248) bajando por la ladera.
- Plano medio: rocas basálticas en columnas hexagonales, árboles calcinados, grietas en la roca con lava dorada y roja, y un altar de obsidiana.
- Suelo: roca negra agrietada con vetas de lava dorada.
- Primer plano: rocas volcánicas casi negras con bordes dorados en los extremos.
````

### Costa del naufragio

````
ESCENARIO: una playa rocosa con un barco naufragado, noche de tormenta.
- Cielo: nubes de tormenta densas en bandas, un relámpago claro a lo lejos y un trozo de luna.
- Lejos: el mar oscuro con olas en la línea del horizonte y la silueta de un faro en un acantilado con luz dorada.
- Plano medio: el casco roto de un galeón varado y escorado, con un mástil partido y velas hechas jirones, rocas con algas, un ancla clavada en la arena y barriles.
- Suelo: arena húmeda con conchas, tablones y cuerdas.
- Primer plano: rocas y tablones rotos casi negros en los extremos.
- Detalles: fuegos de San Telmo en verde espectral en la punta del mástil.
````

### Catacumbas

````
ESCENARIO: catacumbas subterráneas bajo el cementerio (interior, no hay cielo).
- Cielo (en su lugar): muro de piedra con hileras de nichos y cráneos apilados (osario), en penumbra.
- Lejos: un pasillo abovedado que se pierde en la oscuridad, con arcos sucesivos.
- Plano medio: sarcófagos de piedra con tapas talladas, columnas con calaveras incrustadas, candelabros con velas de luz dorada y telarañas en las esquinas.
- Suelo: losas de piedra con huesos y polvo.
- Primer plano: montones de cráneos y huesos casi negros en los extremos.
- Detalles: un brasero con fuego verde espectral.
````

### Torre del mago

````
ESCENARIO: lo alto de la torre de un hechicero (interior, no hay cielo).
- Cielo (en su lugar): muro curvo de piedra con estanterías llenas de libros y un gran ventanal redondo con la luna y estrellas.
- Lejos: una escalera de caracol que sube en la penumbra y un telescopio de latón.
- Plano medio: mesas con alambiques y frascos, un atril con un libro abierto, un orbe flotando sobre un pedestal (pegado al pedestal) en verde espectral, pilas de libros y un esqueleto de criatura colgado.
- Suelo: suelo de madera con un círculo mágico de runas grabado en dorado.
- Primer plano: pilas de libros y cofres casi negros en los extremos.
````

## Paletas propuestas para el juego

Los 8 papeles de siempre (contorno, oscuro, medio, claro, brillo, sangre, oro, espectral) con el tono de cada bioma. Se añaden a `tools/art/palettes.py` al importar cada fondo.

| Bioma | Negro | Oscuro | Medio | Claro | Brillo | Sangre | Oro | Espectral |
|---|---|---|---|---|---|---|---|---|
| Pantano | `0C0E08` | `22281A` | `46502E` | `8C9460` | `E0E0B8` | `962E2A` | `D0A040` | `C8F040` |
| Cueva | `0C0B0A` | `231F1C` | `4A4139` | `8C7F70` | `E0D6C4` | `A8342C` | `E0A83C` | `5ED0E8` |
| Montaña | `0A0D14` | `1C2638` | `3E5170` | `8AA0BC` | `E8F0F8` | `A0303A` | `D8B060` | `7CF0C0` |
| Desierto | `120C08` | `3A2618` | `6E4A2C` | `B88E5C` | `F2E2BE` | `A8382A` | `F0B840` | `4CD8C8` |
| Volcán | `0C0606` | `2A1210` | `54241C` | `8E6A5A` | `F4D8B0` | `D8401C` | `FFB030` | `B060F0` |
| Costa | `080D10` | `15262C` | `2C4C54` | `6E949A` | `D8E8E4` | `9C2E30` | `D6A248` | `7AF0D8` |
| Catacumbas | `0A0908` | `211D19` | `453D35` | `8A8074` | `DCD4C6` | `A02A2A` | `D8A850` | `E0F070` |
| Torre | `0A0A12` | `1E1C34` | `3E3A64` | `8480B0` | `E4E0F4` | `B0304A` | `E8B848` | `60E0F0` |

> Cuando tengas las imágenes, déjalas en `assets/ai/` con el nombre de la tabla y dime cuáles entran: yo añado la paleta, importo el fondo, le pongo nombre y rótulo de entrada, y lo meto en el orden de biomas con los enemigos que le toquen.

---

# Héroes nuevos (mismo método que el héroe v2)

Cada héroe se hace en **dos mensajes dentro de la misma conversación**: primero la tira (bloque común de la tira y el bloque del héroe) y después el mapa de piezas (bloque común del mapa y la lista de piezas del héroe). Se guardan en `assets/ai/` como `<nombre>.png` y `<nombre>_mapa.png`.

Todos usan las **6 piezas** del héroe, porque el botín las cambia: casco, pechera, botas, mano trasera (en el código, el escudo), brazo con arma y cuerpo. El arma y la pieza de la mano trasera cambian de nombre según el héroe.

| Archivo | Héroe | Arma | Mano trasera | Stats (propuesta) | Mecánica propia |
|---|---|---|---|---|---|
| `samurai` | Samurái errante | Katana | Vaina | Crítico alto, poca vida | Zona perfecta de la carga más ancha |
| `enano` | Enano | Martillo | Escudo redondo | Mucha vida y defensa, falla más | Anula el primer crítico recibido en cada combate |
| `espartano` | Espartano | Lanza | Hoplón (escudo redondo) | Defensa alta, ataque medio | Contraataque cuando el enemigo falla |
| `picara` | Pícara | Daga | Segunda daga | Crítico alto, defensa baja | Más botín raro y más pociones |
| `duelista` | Duelista | Estoque | Daga de parada | Poco fallo, crítico medio | Esquiva: probabilidad de no recibir daño |
| `hechicera` | Hechicera | Báculo | Grimorio | Ataque alto, defensa muy baja | Especial cada 2 golpes en vez de 3 |
| `nigromante` | Nigromante | Hoz | Farol de almas | Vida baja | Roba vida de base |
| `druida` | Druida | Cayado | Escudo de corteza | Equilibrado | Se regenera un poco cada turno |
| `clerigo` | Clérigo de batalla | Maza | Libro sagrado | Defensa alta, ataque bajo | Las pociones curan el doble |
| `esqueleto` | Esqueleto (maldición no muerta) | La del héroe | La del héroe | Penalizado (ver GDD §6.5) | Forma del héroe al morir con el talismán |

## Mensaje 1 — bloque común de la tira (pegar antes del héroe)

````
Genera una tira de sprites de pixel art del héroe de un RPG de fantasía medieval oscura.

COMPOSICIÓN
- 7 celdas del mismo ancho, una junto a otra, en este orden:
  1. Reposo A: en guardia, arma lista y la pieza de la mano trasera delante del cuerpo.
  2. Reposo B: idéntico al 1, pero el torso, la cabeza y los brazos 1 píxel más abajo. Las piernas no se mueven.
  3. Ataque, preparación: arma echada atrás.
  4. Ataque, golpe: arma lanzada hacia delante (a la derecha), cuerpo algo adelantado.
  5. Ataque, remate: arma terminando el movimiento, abajo y delante.
  6. Carga: arma en alto sobre la cabeza, postura tensa.
  7. Recibe golpe: echado hacia atrás y encogido.
- El personaje mira a la DERECHA, de perfil, de cuerpo entero, unos 64 px de alto en resolución nativa. Proporciones heroicas compactas: unas 4 cabezas de alto, nada de estilo chibi.
- La pieza de la mano trasera (el brazo más alejado) se ve en los 7 fotogramas.
- En todas las celdas, los pies sobre la misma línea de suelo y en la misma posición horizontal (en el tercio izquierdo de la celda, para que el arma tenga sitio a la derecha). Misma escala en todas. Las piernas cambian lo mínimo: el héroe nunca camina.
- Nada suelto: sin proyectiles, chispas, partículas ni resplandores separados del cuerpo.
- Sin texto, sin números y sin sombra en el suelo.

ESTILO
- Pixel art limpio de 16 bits, como dibujado a mano en Aseprite a unos 64 px de alto y escalado con vecino más cercano. Todos los píxeles son cuadrados del mismo tamaño.
- Contorno negro de 1 píxel alrededor de toda la silueta.
- Luz desde arriba a la izquierda. Tres tonos por material.
- Sin antialiasing, sin degradados, sin píxeles sueltos. El borde con el fondo es duro.

PALETA ESTRICTA (solo estos colores):
#0A0E0D negro (contornos), #1B2925 oscuro (cuero, tela oscura), #3B5247 medio, #7A9380 claro (metal, piedra, hueso), #D4DEC8 brillo (reflejos, piel muy clara), #9C2E30 rojo, #D6A248 oro (con moderación), #9FE05A verde espectral (solo si el héroe lo pide, con moderación).

FONDO: magenta puro #FF00FF, liso.
````

## Mensaje 2 — bloque común del mapa de piezas (sobre la imagen anterior)

````
Ahora, sobre la imagen exacta que acabas de generar, haz una versión de mapa de piezas. Debe tener las mismas 7 celdas, las mismas poses y la misma silueta píxel a píxel. No muevas, añadas ni quites nada; solo cambia los colores.

Pinta cada pieza con un único color plano, sin sombras ni contornos:
- CASCO: azul puro #0000FF
- PECHERA: amarillo puro #FFFF00
- BOTAS: cian puro #00FFFF
- MANO TRASERA: naranja puro #FF8000
- BRAZO CON ARMA: verde puro #00FF00
- CUERPO (todo lo demás): gris #808080

El fondo sigue en magenta #FF00FF. Sin contorno negro: cada píxel del personaje pasa a ser exactamente uno de esos 6 colores.
QUÉ ES CADA PIEZA EN ESTE PERSONAJE:
````

Después de «QUÉ ES CADA PIEZA EN ESTE PERSONAJE:» se pega la lista de piezas del héroe (la que va debajo de su bloque).

## Bloques de cada héroe

### Samurái errante

````
PERSONAJE: un samurái errante, ronin, mirando a la DERECHA.
- Kabuto (casco) de placas lacadas oscuras con un adorno frontal dorado en media luna y una máscara menpo de demonio en la cara.
- Do (coraza) de láminas atadas con cordones rojos (#9C2E30).
- Hombreras de láminas, hakama ancho y oscuro, sandalias de paja y espinilleras de placas.
- Una cinta roja en la cintura cuyas puntas caen detrás.
- Katana larga de hoja clara y guarda dorada redonda en la mano delantera; en la mano trasera, la vaina negra de la katana sujeta delante del cuerpo.
- Carga: katana vertical sobre la cabeza con las dos manos (la vaina queda en la mano trasera, baja).
````
Piezas: casco = kabuto con su máscara · pechera = coraza y hombreras · botas = espinilleras y sandalias · mano trasera = la vaina · brazo con arma = brazo delantero y la katana · cuerpo = hakama, cinta, brazo trasero y lo demás.

### Enano

````
PERSONAJE: un guerrero enano, mirando a la DERECHA. Bajo y muy ancho: unas 3 cabezas de alto y casi tan ancho como alto (mide unos 52 px de alto).
- Yelmo de acero redondo con nasal y dos cuernos cortos.
- Barba enorme y espesa, roja (#9C2E30), trenzada con anillas doradas, que le cae sobre el pecho.
- Peto de acero grueso con remaches y una runa dorada, cinturón ancho con hebilla dorada.
- Cota de malla corta, brazos musculosos, botas de hierro pesadas.
- Martillo de guerra de cabeza enorme de acero con runas en la mano delantera; en la mano trasera, un escudo redondo de madera con borde de hierro y una runa dorada.
- Ataque: martillazo de arriba abajo.
````
Piezas: casco = yelmo con cuernos · pechera = peto y cinturón · botas = botas de hierro · mano trasera = escudo redondo · brazo con arma = brazo delantero y martillo · cuerpo = barba, cara, malla y lo demás.

### Espartano

````
PERSONAJE: un guerrero espartano (hoplita), mirando a la DERECHA.
- Yelmo corintio de bronce (usa el oro #D6A248 sombreado con medio y oscuro) que cubre la cara, con una cresta alta y transversal de crin roja (#9C2E30).
- Coraza de bronce musculada, faldón de tiras de cuero (pteruges).
- Capa roja larga a la espalda.
- Grebas de bronce en las espinillas y sandalias.
- Lanza larga (dory) de madera con punta de acero en la mano delantera, sujeta por encima del hombro; en la mano trasera, un escudo hoplón redondo y grande de bronce con una lambda (Λ) roja.
- Ataque: estocada de lanza hacia delante. Carga: lanza alzada sobre la cabeza para arrojarla (sin soltarla).
````
Piezas: casco = yelmo con cresta · pechera = coraza · botas = grebas y sandalias · mano trasera = hoplón · brazo con arma = brazo delantero y lanza · cuerpo = capa, faldón y lo demás.

### Pícara

````
PERSONAJE: una pícara ladrona, ágil y delgada, mirando a la DERECHA.
- Capucha corta de cuero oscuro con la cara medio en sombra y un pañuelo rojo (#9C2E30) que le tapa la boca; un mechón de pelo claro asomando.
- Jubón de cuero ajustado con correas cruzadas en el pecho y bolsitas.
- Capa corta hecha jirones, pantalones oscuros y botas altas de cuero blando.
- Una daga en cada mano: la delantera, de hoja más larga; la trasera, cruzada delante del cuerpo con la hoja hacia abajo.
- Postura baja y agazapada.
````
Piezas: casco = capucha y pañuelo · pechera = jubón con correas · botas = botas altas · mano trasera = la segunda daga con su mano · brazo con arma = brazo delantero y daga · cuerpo = capa, piernas y lo demás.

### Duelista

````
PERSONAJE: un duelista espadachín, elegante, mirando a la DERECHA.
- Sombrero de ala ancha con una pluma roja (#9C2E30) larga hacia atrás.
- Jubón acolchado con mangas abullonadas y botones dorados, cuello de encaje claro.
- Capa corta sobre un hombro, calzas oscuras y botas altas con vuelta.
- Estoque fino y largo con guarda de lazo dorada en la mano delantera, en postura de esgrima (brazo extendido); en la mano trasera, una daga de parada (main gauche) con guarda grande, levantada.
- Postura de esgrima: piernas abiertas de lado, de perfil.
````
Piezas: casco = sombrero con pluma · pechera = jubón · botas = botas altas · mano trasera = daga de parada con su mano · brazo con arma = brazo delantero y estoque · cuerpo = capa, calzas y lo demás.

### Hechicera

````
PERSONAJE: una hechicera de batalla, mirando a la DERECHA.
- Sombrero puntiagudo de ala ancha, oscuro, con una banda dorada; melena larga y clara que cae por la espalda.
- Corpiño de cuero sobre un vestido largo oscuro con el bajo cortado para poder moverse, y una faja roja (#9C2E30).
- Botas de cuero.
- Báculo largo de madera con un cristal verde espectral (#9FE05A) engarzado en la punta, en la mano delantera; en la mano trasera, un grimorio abierto delante del cuerpo con runas verdes en las páginas.
- Ataque: golpe con el báculo hacia delante. Carga: báculo en alto, el cristal más brillante.
````
Piezas: casco = sombrero · pechera = corpiño y faja · botas = botas · mano trasera = grimorio · brazo con arma = brazo delantero y báculo · cuerpo = pelo, vestido y lo demás.

### Nigromante (héroe)

````
PERSONAJE: un joven nigromante, flaco y pálido, mirando a la DERECHA. Diferente del jefe Nigromante: es humano, joven y lleva armadura ligera.
- Capucha negra con un cráneo pequeño de cuervo como broche, cara pálida (#D4DEC8) con ojeras y ojos verdes espectrales.
- Hombrera de hueso en el hombro delantero y coraza ligera de cuero negro con costillas pintadas.
- Túnica negra larga hecha jirones abajo, botas negras.
- Hoz de mango largo con hoja curva clara en la mano delantera; en la mano trasera, un farol de hierro con una llama verde espectral dentro.
- Ataque: tajo curvo con la hoz.
````
Piezas: casco = capucha · pechera = coraza y hombrera · botas = botas · mano trasera = farol · brazo con arma = brazo delantero y hoz · cuerpo = túnica, cara y lo demás.

### Druida

````
PERSONAJE: un druida guerrero, mirando a la DERECHA.
- Tocado de cráneo de ciervo pequeño con astas cortas sobre una capucha de piel.
- Barba larga castaña, tatuajes de espiral en los brazos.
- Peto de cuero con hojas y una capa de pieles.
- Brazaletes de corteza, pantalones de cuero y botas de piel atadas con cuerda.
- Cayado de madera viva con hojas brotando y una piedra verde espectral en la punta, en la mano delantera; en la mano trasera, un escudo de corteza y ramas trenzadas.
- Ataque: golpe con el cayado.
````
Piezas: casco = tocado de ciervo y capucha · pechera = peto · botas = botas de piel · mano trasera = escudo de corteza · brazo con arma = brazo delantero y cayado · cuerpo = barba, capa y lo demás.

### Clérigo de batalla

````
PERSONAJE: un clérigo guerrero, mirando a la DERECHA.
- Capucha de malla con una diadema dorada; cara seria con barba corta.
- Sobreveste clara (#D4DEC8) con un sol dorado bordado en el pecho, sobre cota de malla; cinturón con una cadena y un relicario.
- Estola roja (#9C2E30) colgando del cuello.
- Grebas de acero y botas.
- Maza de cabeza con aletas de acero en la mano delantera; en la mano trasera, un libro sagrado grande con tapas doradas y cadena, sujeto delante del pecho.
- Ataque: mazazo de arriba abajo.
````
Piezas: casco = capucha de malla y diadema · pechera = sobreveste y estola · botas = grebas y botas · mano trasera = libro · brazo con arma = brazo delantero y maza · cuerpo = malla, cara y lo demás.

### Esqueleto (maldición no muerta)

Es la forma que toma cualquier héroe al morir con el talismán. Conserva su equipo, así que debe parecer un esqueleto guerrero genérico que lleva **restos** de armadura.

````
PERSONAJE: un esqueleto guerrero maldito, mirando a la DERECHA. Es el héroe que ha vuelto de entre los muertos.
- Cráneo con las cuencas ardiendo en verde espectral (#9FE05A) y un yelmo abollado y roto puesto torcido.
- Costillas, columna y huesos de los brazos visibles (hueso #7A9380 / #D4DEC8); los huecos entre huesos van en negro u oscuro, NO en magenta, para que sea una sola figura.
- Restos de peto oxidado colgando de una correa, jirones de una capa roja descolorida.
- Grebas oxidadas sueltas en las piernas de hueso.
- Espada mellada y oxidada en la mano delantera; en la mano trasera, un escudo roto y agrietado.
- Un hilo de luz verde espectral en el pecho, donde estaría el corazón (pegado a las costillas).
- Postura algo encorvada y desmadejada.
````
Piezas: casco = yelmo roto · pechera = restos de peto · botas = grebas oxidadas · mano trasera = escudo roto · brazo con arma = brazo delantero y espada · cuerpo = cráneo, costillas, huesos, capa y lo demás.

> Pásame cada pareja (`<nombre>.png` y `<nombre>_mapa.png`). Yo la importo como el Caballero y la Vikinga, le doy sus stats, su mecánica, el nombre de su arma y de su mano trasera, y la añado a la selección de héroe. El esqueleto sustituye al aspecto provisional de la maldición.
