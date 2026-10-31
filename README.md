# Snake

La viborita clásica: come, crece y no choques. Desarrollado con **Qt6** y **Deepin Tool Kit (DTK6)** para Deepin Linux.

## Características

- Niveles: la velocidad aumenta cada 50 puntos
- Récord guardado
- Cada partida espera a tu primera dirección
- Giros rápidos fiables (se encolan, ya no te hacen chocar contigo mismo)
- Traducido a 8 idiomas

## Controles

| Tecla | Acción |
|-------|--------|
| Flechas / WASD | Dirección (la primera inicia la partida) |
| P / Esc | Pausa |
| Enter | Reiniciar tras perder |
| Ctrl+N | Nuevo juego |
| Ctrl+Q | Salir |

## Instalación

Descarga el paquete de la [última release](https://github.com/deepin-espanol/snake/releases/latest):

- `.deb` (amd64, arm64, riscv64, loong64):
  ```bash
  sudo apt install ./snake_*.deb
  ```
- `.uab` de linyaps (amd64, arm64): ejecutable directamente.
  ```bash
  chmod +x org.deepinenespanol.*.uab && ./org.deepinenespanol.*.uab
  ```

## Compilar

```bash
sudo apt build-dep ./          # dependencias declaradas en debian/control
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/snake
```

O generar el paquete `.deb`:

```bash
dpkg-buildpackage -us -uc -b
```

## Publicar una versión

Añade una entrada nueva en `debian/changelog` y haz push: el workflow
*Build and publish .deb* crea la release con los `.deb` y los `.uab`.

## Licencia

GPL-3.0 o posterior. © Deepin en Español — https://deepinenespanol.org
