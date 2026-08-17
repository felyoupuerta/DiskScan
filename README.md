# DiskScan

Analizador de espacio en disco en C (estilo du / ncdu) para sistemas POSIX (Linux/WSL).
Recorre un directorio, deduplica hardlinks y muestra un árbol ordenado por tamaño.

## Por qué existe

Este proyecto lo hice yo solo, de cero, mientras estudio el Grado Superior de ASIR en Madrid y trabajo como soporte de Microinformática y administrador de sistemas Linux. Quería una herramienta propia para ver dónde se va el espacio en mis
servidores y de paso entender de verdad cómo se gestiona memoria y syscalls en C — sin
depender de librerías que hicieran el trabajo sucio por mí. Arena allocator, deduplicación de
hardlinks con hashset propio, árbol en memoria por índices en vez de punteros sueltos: cada
pieza la pensé y la rompí varias veces hasta que quedó bien. Es el proyecto personal del que
más orgulloso estoy hasta ahora.

— Felipe Angeriz Estefanell

## Compilar
make            
make debug      
make test       


## Uso

```bash
./dsk [OPCIONES] RUTA
```

| Opción | Descripción |
|--------|-------------|
| `-d, --depth N`  | niveles a mostrar (por defecto 1) |
| `-b, --bytes`    | bytes exactos sin formatear |
| `-x, --one-fs`   | no cruzar puntos de montaje (por defecto) |
| `--cross-fs`     | permitir cruzar puntos de montaje |
| `-t, --total`    | mostrar solo el total en GB (sin árbol) |
| `-a, --afondo N` | mostrar el top N de ficheros más pesados |
| `-m, --menu`     | ejecutar el menú interactivo |
| `-h, --help`     | ayuda |

``` c
Esta es la estructura estática que uso en el  codigo para declarar las opciones de argumentos:

    {"depth",    required_argument, NULL, 'd'},
    {"bytes",    no_argument,       NULL, 'b'},
    {"one-fs",   no_argument,       NULL, 'x'},
    {"cross-fs", no_argument,       NULL,  1 },
    {"total",    no_argument,       NULL, 't'},
    {"afondo",   required_argument, NULL, 'a'},
    {"help",     no_argument,       NULL, 'h'},
    {"menu",     no_argument,       NULL, 'm'},

Si deseas añadir más opciones de argumentos deberás modificar esta lista y el getopt que está mas abajo en opciones.c:
    while((c = getopt_long(argc,argv,"d:bxtmha:",largas,NULL)) != -1)
Si el argumento va a necesitar parámetros deberás poner los ":" luego de la letra

Y luego modificar el case, par tratar esa opcion con sus respectivas funciones que puedes crear tú.
```

## Ejemplo
```bash
./dsk -d 2 /home/felipe
```

## Arquitectura

Módulos, estructuras de datos y el algoritmo de escaneo, con diagramas:
[docs/ARQUITECTURA.md](docs/ARQUITECTURA.md).

