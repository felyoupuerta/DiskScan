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
