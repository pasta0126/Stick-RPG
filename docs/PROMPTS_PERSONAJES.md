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
