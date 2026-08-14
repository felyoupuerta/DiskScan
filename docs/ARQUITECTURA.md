# Arquitectura de DiskScan

Documentación técnica del funcionamiento interno. Pensada para quien vaya a leer o modificar el código fuente en [`src/`](../src).

## 1. Módulos y dependencias

Quién incluye a quién (`#include` propios, no del sistema):

```mermaid
graph TD
    %% Subgrafos para dar estructura y contexto visual
    subgraph Core["Core App"]
        main["fa:fa-play main.c"]
    end

    subgraph Config["Configuración & UI"]
        opciones["opciones.c / .h"]
        ui["ui.c / .h"]
    end

    subgraph Processing["Escaneo y Estructuras"]
        scan["scan.c / .h"]
        tree["tree.c / .h"]
    end

    subgraph Memory["Gestión de Memoria & Deduplicación"]
        hardlink["hardlink.c / .h"]
        arena["arena.c / .h"]
    end

    subgraph Output["Generación de Informes"]
        report["report.c / .h"]
    end

    %% Relaciones
    main --> opciones
    main --> ui
    main --> scan
    main --> report

    opciones -.-|Llamada condicional| ui
    ui --> opciones

    scan --> tree
    scan --> hardlink
    report --> tree
    hardlink --> arena

    %% Estilos de los nodos
    style main fill:#5E81AC,stroke:#81A1C1,stroke-width:2px,color:#ECEFF4
    style opciones fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style ui fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style scan fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style report fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style tree fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style hardlink fill:#4C566A,stroke:#D8DEE9,stroke-width:1px,color:#ECEFF4
    style arena fill:#8FBCBB,stroke:#88C0D0,stroke-width:2px,color:#2E3440
```

`opciones.c` llama a `ui_menu_interactivo()` cuando se pasa `-m/--menu` (línea punteada), y a la vez `ui.h` incluye `opciones.h` porque necesita el tipo `Opciones`. No es un ciclo real — solo las cabeceras (`.h`) definen las dependencias de compilación, y ninguna se incluye a sí misma indirectamente — pero es la zona más acoplada del código y la primera candidata a romper si se refactoriza.

`arena.c` es la única pieza sin dependencias propias: es el módulo base sobre el que se apoya `hardlink.c`.

## 2. Estructuras de datos principales

```mermaid
classDiagram
    direction TB

    class Opciones {
        +char* ruta
        +int prof_max
        +bool bytes_exactos
        +bool un_solo_fs
        +bool solo_total
        +int top_ficheros
    }

    class Ctx {
        +dev_t raiz_dev
        +bool un_solo_fs
        +uint64_t n_fich
        +uint64_t n_dirs
        +uint64_t n_errores
        +HLSet* hl
        +bool dedup
        +Arbol* arbol
    }

    class Arbol {
        +Nodo[] v
        +uint32_t n
        +uint32_t cap
        +Pool cadenas
    }

    class Nodo {
        +uint64_t bytes
        +uint64_t bytes_ap
        +uint32_t nombre_off
        +uint32_t padre
        +uint32_t hijo
        +uint32_t hermano
        +uint32_t n_fich
        +uint16_t modo
    }

    class Pool {
        +char* buf
        +size_t usado
        +size_t cap
    }

    class HLSet {
        +Entrada*[] cajones
        +size_t n_cajones
        +size_t n
        +Arena* arena
    }

    class Entrada {
        +Entrada* sig
        +dev_t dev
        +ino_t ino
    }

    class Arena {
        +Bloque* actual
        +size_t tam_bloque
        +size_t reservado
        +size_t repartido
    }

    class Bloque {
        +Bloque* sig
        +size_t usado
        +size_t tam
        +byte[] datos
    }

    %% Relaciones de dependencia y composición
    Ctx --> Arbol : arbol
    Ctx --> HLSet : hl
    Ctx ..> Opciones : construido a partir de

    Arbol "1" *-- "*" Nodo : v[]
    Arbol *-- Pool : cadenas

    HLSet o-- Arena : arena
    HLSet "1" *-- "*" Entrada : cajones[]

    Arena "1" *-- "*" Bloque : lista enlazada
```

Puntos clave de este diseño:

- **`Arbol` no usa punteros entre nodos**, usa índices `uint32_t` dentro de `v[]` (`padre`, `hijo`, `hermano`). Es una lista enlazada de hijos, no un array de hijos por nodo — cada nodo solo conoce a su primer hijo y a su siguiente hermano.
- **`Pool` guarda todos los nombres de fichero concatenados** en un único buffer; cada `Nodo` solo guarda el offset (`nombre_off`) dentro de ese buffer. Evita un `malloc` por nombre.
- **`HLSet` (deduplicación de hardlinks)** es un hashset de `(dev, ino)` con listas de colisión, cuyas entradas (`Entrada`) viven en un `Arena` en vez de mallocs sueltos.
- **`Arena`** reserva memoria en bloques grandes (`Bloque`) y reparte trozos alineados dentro de cada bloque; solo libera todo de golpe al final (`arena_free`), nunca entrada por entrada.

## 3. Flujo principal (`main`)

```mermaid
sequenceDiagram
    autonumber
    actor U as Usuario
    participant M as main
    participant O as opciones_parse
    participant UI as ui_menu_interactivo
    participant S as scan_dir
    participant R as report_*

    U->>M: ./dsk [opciones] RUTA
    M->>M: ui_banner()
    M->>O: opciones_parse(argc, argv)

    opt Se pasó -m / --menu
        O->>UI: ui_menu_interactivo(&o, ...)
        UI-->>O: Opciones rellenadas por el usuario
    end

    O-->>M: Opciones o

    rect rgb(46, 52, 64)
        Note over M: Inicialización de Contexto
        M->>M: open(ruta, O_DIRECTORY|O_NOFOLLOW)
        M->>M: fstat() -> raiz_dev
        M->>M: hl_new() / arbol_init()
        M->>M: arbol_add(raiz)
    end

    M->>S: scan_dir(ctx, fd_raiz, idx_raiz, 0)
    Note over S: Recorre el árbol recursivamente<br/>(Ver Diagrama 4)
    S-->>M: Total bytes reales

    alt Opciones de salida: --total
        M->>U: Imprime total en GB
    else Opciones de salida: --afondo N
        M->>R: report_ficheros_top(N)
        R-->>U: Muestra Top N ficheros más pesados
    else Salida por defecto
        M->>R: report_ordenar() + report_arbol()
        R-->>U: Muestra árbol ordenado por tamaño
    end

    rect rgb(46, 52, 64)
        Note over M: Limpieza de Memoria
        M->>M: arbol_destroy() / hl_free()
    end
```

## 4. Algoritmo de escaneo (`scan_dir`)

```mermaid
flowchart TD
    A([scan_dir recibe: fd, índice, prof]) --> B{profundidad ><br/>SCAN_PROFUNDIDAD_MAX?}
    
    B -- Sí --> Z([return 0])
    B -- No --> C[fdopendir + fstat del directorio]
    C --> D[total = st_blocks * 512]
    
    D --> E{¿Quedan entradas<br/>por leer? readdir}
    E -- No --> Y[Guarda bytes / bytes_ap / n_fich en nodo]
    Y --> Y_ret([return total])

    E -- Sí --> F{¿Es '.' o '..'? =}
    F -- Sí --> E
    F -- No --> G[fstatat de la entrada]
    
    G --> H{¿Es directorio?}

    %% Rama Directorios
    subgraph DirBranch["Tratamiento de Directorios"]
        H -- Sí --> I{un_solo_fs &&<br/>dev != raiz_dev?}
        I -- Sí (distinto FS) --> E
        I -- No --> J[arbol_add del subdirectorio]
        J --> K[openat + scan_dir recursivo]
        K --> L[Acumula total, bytes_ap y n_fich]
        L --> E
    end

    %% Rama Ficheros
    subgraph FileBranch["Tratamiento de Ficheros / Hardlinks"]
        H -- No (Fichero) --> M[arbol_add del fichero]
        M --> N{dedup activo &&<br/>st_nlink > 1?}
        
        N -- Sí --> O{hl_visto_o_insertar:<br/>¿Ya visto?}
        O -- Sí --> P[bytes = 0, bytes_ap = 0<br/>No se vuelve a contar]
        O -- No --> Q[Registra dev+ino, cuenta normal]
        N -- No --> Q

        P --> R[Acumula bytes en nodo y total]
        Q --> R
        R --> E
    end

    %% Estilos de Nodos
    style A fill:#5E81AC,color:#FFF,stroke-width:0px
    style Z fill:#BF616A,color:#FFF,stroke-width:0px
    style Y_ret fill:#A3BE8C,color:#2E3440,stroke-width:0px
    style DirBranch fill:#2E3440,stroke:#4C566A,stroke-width:1px
    style FileBranch fill:#2E3440,stroke:#4C566A,stroke-width:1px
```

Las dos decisiones que definen el comportamiento de DiskScan frente a `du` están ahí: el corte por punto de montaje (`un_solo_fs`) y la deduplicación de hardlinks (`dedup` + `HLSet`), ambas resueltas por fichero/directorio durante el mismo recorrido, sin una segunda pasada.

## 5. Ejemplo de árbol por índices

Para visualizar cómo queda `Arbol.v[]` tras escanear `/home/felipe` con dos subcarpetas:

```mermaid
flowchart LR
    R["<b>v[0] home/felipe</b><hr/>hijo: 1<br/>hermano: NODO_NULO"]
    A["<b>v[1] docs</b><hr/>hijo: 3<br/>hermano: 2"]
    B["<b>v[2] fotos</b><hr/>hijo: NODO_NULO<br/>hermano: NODO_NULO"]
    C["<b>v[3] tesis.pdf</b><hr/>hijo: NODO_NULO<br/>hermano: NODO_NULO"]

    R ==>|hijo| A
    A -->|hermano| B
    A ==>|hijo| C

    %% Estilos de los nodos
    style R fill:#5E81AC,color:#ECEFF4,stroke:#81A1C1,stroke-width:2px
    style A fill:#4C566A,color:#ECEFF4,stroke:#D8DEE9,stroke-width:1px
    style B fill:#4C566A,color:#ECEFF4,stroke:#D8DEE9,stroke-width:1px
    style C fill:#D08770,color:#2E3440,stroke:#EBCB8B,stroke-width:1px
```