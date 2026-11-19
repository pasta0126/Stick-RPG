# Stick-RPG

RPG de combate por turnos con scroll lateral automático para **M5StickC Plus2** (pantalla horizontal 240 × 135, paleta de 16 colores).

El guerrero avanza solo entre encuentros; el jugador decide cómo atacar (normal o cargado) y qué recompensa aceptar tras cada victoria. El objetivo es sobrevivir al mayor número posible de enemigos.

- Diseño: [GDD_RPG_M5StickC_Plus2.md](GDD_RPG_M5StickC_Plus2.md)
- Estado: prototipo jugable con gráficos provisionales.

## Compilar

```sh
pio run -t upload        # firmware al M5StickC Plus2
make test                # pruebas de la lógica en el PC
make sim ARGS="--skill 0.7 --games 10000"   # simulador de balance
```
