# Snake

Juego clasico Snake desarrollado con Qt6 y Deepin Tool Kit (DTK6) para Deepin Linux.

## Caracteristicas

- Movimiento fluido con flechas o WASD
- Puntuacion por cada fruta comida (+10 puntos)
- Game Over al chocar paredes o con ella misma

## Dependencias

```bash
sudo apt install qt6-base-dev libdtk6widget-dev cmake g++
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
./snake
```

O si se instalo globalmente:

```bash
sudo make install
snake
```

## Controles

| Tecla | Accion |
|-------|--------|
| `Flechas` o `WASD` | Mover serpiente |
