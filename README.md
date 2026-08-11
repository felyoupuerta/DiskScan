# DiskScan

Analizador de espacio en disco en C (estilo `du`/`ncdu`) para sistemas POSIX (Linux/WSL).
Recorre un directorio, deduplica hardlinks y muestra un árbol ordenado por tamaño.

## Compilar

```bash
make            # binario release: ./dsk
make debug      # binario con ASan/UBSan: ./dsk-debug
make test       # ejecuta los tests
```

## Uso

```bash
./dsk [OPCIONES] RUTA     # sin argumentos: menú interactivo
```

| Opción | Descripción |
|--------|-------------|
| `-d, --depth N` | niveles a mostrar (por defecto 1) |
| `-b, --bytes`   | bytes exactos sin formatear |
| `-x, --one-fs`  | no cruzar puntos de montaje (por defecto) |
| `--cross-fs`    | permitir cruzar puntos de montaje |
| `-t, --total`   | mostrar solo el total en GB |
| `-h, --help`    | ayuda |

## Ejemplo

```bash
./dsk -d 2 /home/felipe
```
