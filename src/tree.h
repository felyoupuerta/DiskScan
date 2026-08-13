/* ============================================================
 *  DiskScan — tree.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef TREE_H
#define TREE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NODO_NULO  UINT32_MAX   /* "no hay nodo" */
#define POOL_ERR   UINT32_MAX   /* error al añadir al pool */

typedef struct {
    uint64_t bytes;       /* espacio real acumulado del subárbol   */
    uint64_t bytes_ap;    /* tamaño aparente acumulado  */
    uint32_t nombre_off;  /* offset del nombre en el pool          */
    uint32_t padre;       /* índice del padre,NODO_NULO         */
    uint32_t hijo;        /* índice del PRIMER hijo, o NODO_NULO   */
    uint32_t hermano;     /* índice del SIGUIENTE hermano          */
    uint32_t n_fich;      /* ficheros acumulados en el subárbol    */
    uint16_t modo;        /* st_mode: distingue fichero/directorio */
} Nodo;

typedef struct {
    char  *buf;
    size_t usado;
    size_t cap;
} Pool;

typedef struct {
    Nodo    *v;       /* array de nodos */
    uint32_t n;       /* nodos usados   */
    uint32_t cap;     /* capacidad      */
    Pool     cadenas;
} Arbol;

bool arbol_init(Arbol *t, uint32_t cap_ini);
void arbol_destroy(Arbol *t);

uint32_t arbol_add(Arbol *t, const char *nombre, uint16_t modo, uint32_t padre);

static inline Nodo *arbol_nodo(Arbol *t, uint32_t idx) { return &t->v[idx]; }

const char *arbol_nombre(const Arbol *t, uint32_t idx);

size_t arbol_ruta(const Arbol *t, uint32_t idx, char *dst, size_t cap);

#endif /* TREE_H */
