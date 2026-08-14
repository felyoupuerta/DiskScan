# Arquitectura de DiskScan

Documentación técnica del funcionamiento interno. Pensada para quien vaya a leer o modificar
el código fuente en [`src/`](../src).

## 1. Módulos y dependencias

Quién incluye a quién (`#include` propios, no del sistema):

```mermaid
graph TD
    main[main.c]
    opciones[opciones.c / opciones.h]
    ui[ui.c / ui.h]
    scan[scan.c / scan.h]
    report[report.c / report.h]
    tree[tree.c / tree.h]
    hardlink[hardlink.c / hardlink.h]
    arena[arena.c / arena.h]

    main --> opciones
    main --> ui
    main --> scan
    main --> report

    opciones -.-> ui
    ui --> opciones

    scan --> tree
    scan --> hardlink
    report --> tree
    hardlink --> arena

    style main fill:#4c566a,color:#fff
```

`opciones.c` llama a `ui_menu_interactivo()` cuando se pasa `-m/--menu` (línea punteada), y a
la vez `ui.h` incluye `opciones.h` porque necesita el tipo `Opciones`. No es un ciclo real —
solo las cabeceras (`.h`) definen las dependencias de compilación, y ninguna se incluye a sí
misma indirectamente — pero es la zona más acoplada del código y la primera candidata a romper
si se refactoriza.

`arena.c` es la única pieza sin dependencias propias: es el módulo base sobre el que se apoya
`hardlink.c`.

## 2. Estructuras de datos principales

```mermaid
classDiagram
    class Opciones {
        +ruta: char*
        +prof_max: int
        +bytes_exactos: bool
        +un_solo_fs: bool
        +solo_total: bool
        +top_ficheros: int
    }

    class Ctx {
        +raiz_dev: dev_t
        +un_solo_fs: bool
        +n_fich: uint64_t
        +n_dirs: uint64_t
        +n_errores: uint64_t
        +hl: HLSet*
        +dedup: bool
        +arbol: Arbol*
    }

    class Arbol {
        +v: Nodo[]
        +n: uint32_t
        +cap: uint32_t
        +cadenas: Pool
    }

    class Nodo {
        +bytes: uint64_t
        +bytes_ap: uint64_t
        +nombre_off: uint32_t
        +padre: uint32_t
        +hijo: uint32_t
        +hermano: uint32_t
        +n_fich: uint32_t
        +modo: uint16_t
    }

    class Pool {
        +buf: char*
        +usado: size_t
        +cap: size_t
    }

    class HLSet {
        +cajones: Entrada*[]
        +n_cajones: size_t
        +n: size_t
        +arena: Arena*
    }

    class Entrada {
        +sig: Entrada*
        +dev: dev_t
        +ino: ino_t
    }

    class Arena {
        +actual: Bloque*
        +tam_bloque: size_t
        +reservado: size_t
        +repartido: size_t
    }

    class Bloque {
        +sig: Bloque*
        +usado: size_t
        +tam: size_t
        +datos: byte[]
    }

    Ctx --> Arbol : arbol
    Ctx --> HLSet : hl
    Ctx ..> Opciones : construido a partir de

    Arbol "1" --> "*" Nodo : v[]
    Arbol --> Pool : cadenas

    HLSet --> Arena : arena
    HLSet "1" --> "*" Entrada : cajones[]

    Arena "1" --> "*" Bloque : lista enlazada
```

Puntos clave de este diseño:

- **`Arbol` no usa punteros entre nodos**, usa índices `uint32_t` dentro de `v[]` (`padre`,
  `hijo`, `hermano`). Es una lista enlazada de hijos, no un array de hijos por nodo — cada nodo
  solo conoce a su primer hijo y a su siguiente hermano.
- **`Pool` guarda todos los nombres de fichero concatenados** en un único buffer; cada `Nodo`
  solo guarda el offset (`nombre_off`) dentro de ese buffer. Evita un `malloc` por nombre.
- **`HLSet` (deduplicación de hardlinks)** es un hashset de `(dev, ino)` con listas de
  colisión, cuyas entradas (`Entrada`) viven en un `Arena` en vez de mallocs sueltos.
- **`Arena`** reserva memoria en bloques grandes (`Bloque`) y reparte trozos alineados dentro
  de cada bloque; solo libera todo de golpe al final (`arena_free`), nunca entrada por entrada.

## 3. Flujo principal (`main`)

```mermaid
sequenceDiagram
    actor Usuario
    participant main
    participant opciones as opciones_parse
    participant ui as ui_menu_interactivo
    participant scan as scan_dir
    participant report as report_*

    Usuario->>main: ./dsk [opciones] RUTA
    main->>main: ui_banner()
    main->>opciones: opciones_parse(argc, argv)

    opt se pasó -m / --menu
        opciones->>ui: ui_menu_interactivo(&o, ...)
        ui-->>opciones: Opciones rellenadas por el usuario
    end

    opciones-->>main: Opciones o

    main->>main: open(ruta, O_DIRECTORY|O_NOFOLLOW)
    main->>main: fstat() -> raiz_dev
    main->>main: hl_new() / arbol_init()
    main->>main: arbol_add(raiz)

    main->>scan: scan_dir(ctx, fd_raiz, idx_raiz, 0)
    Note over scan: recorre el árbol de directorios<br/>recursivamente (ver diagrama 4)
    scan-->>main: total bytes reales

    alt --total
        main->>Usuario: imprime total en GB
    else --afondo N
        main->>report: report_ficheros_top(N)
        report-->>Usuario: top N ficheros más pesados
    else por defecto
        main->>report: report_ordenar() + report_arbol()
        report-->>Usuario: árbol ordenado por tamaño
    end

    main->>main: arbol_destroy() / hl_free()
```

## 4. Algoritmo de escaneo (`scan_dir`)

```mermaid
flowchart TD
    A[scan_dir recibe: fd del directorio, índice del nodo, profundidad] --> B{profundidad > SCAN_PROFUNDIDAD_MAX?}
    B -- sí --> Z[return 0]
    B -- no --> C[fdopendir + fstat del propio directorio]
    C --> D[total = st_blocks * 512]
    D --> E{quedan entradas por leer? readdir}
    E -- no --> Y[Guarda bytes/bytes_ap/n_fich en el nodo<br/>return total]
    E -- sí --> F{". o "..?}
    F -- sí --> E
    F -- no --> G[fstatat de la entrada]
    G --> H{es directorio?}

    H -- sí --> I{un_solo_fs && dev distinto de raiz_dev?}
    I -- sí, distinto FS --> E
    I -- no --> J[arbol_add del subdirectorio]
    J --> K[openat + scan_dir recursivo]
    K --> L[acumula total, bytes_ap y n_fich del subárbol]
    L --> E

    H -- no, es fichero --> M[arbol_add del fichero]
    M --> N{dedup activo && st_nlink > 1?}
    N -- sí --> O{hl_visto_o_insertar: ya visto?}
    O -- sí --> P[bytes = 0, bytes_ap = 0<br/>no se vuelve a contar]
    O -- no --> Q[se registra dev+ino, cuenta normal]
    N -- no --> Q
    P --> R[acumula bytes en el nodo y en total]
    Q --> R
    R --> E
```

Las dos decisiones que definen el comportamiento de DiskScan frente a `du` están ahí: el corte
por punto de montaje (`un_solo_fs`) y la deduplicación de hardlinks (`dedup` + `HLSet`), ambas
resueltas por fichero/directorio durante el mismo recorrido, sin una segunda pasada.

## 5. Ejemplo de árbol por índices

Para visualizar cómo queda `Arbol.v[]` tras escanear `/home/felipe` con dos subcarpetas:

```mermaid
flowchart TD
    R["v[0] home/felipe<br/>hijo=1"]
    A["v[1] docs<br/>hermano=2, hijo=3"]
    B["v[2] fotos<br/>hermano=NODO_NULO"]
    C["v[3] tesis.pdf<br/>hermano=NODO_NULO"]

    R -->|hijo| A
    A -->|hermano| B
    A -->|hijo| C
```

No hay array de hijos por nodo: `docs` solo apunta a su primer hijo (`tesis.pdf`) y a su
siguiente hermano (`fotos`); recorrer todos los hijos de un nodo es seguir la cadena de
`hermano` a partir de `hijo`.
