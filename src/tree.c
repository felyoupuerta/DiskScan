#include "tree.h"

#include <stdlib.h>
#include <string.h>

/* ---------- Pool de cadenas ---------- */

static bool pool_init(Pool *p, size_t cap)
{
    if (cap < 64) cap = 64;
    p->buf = malloc(cap);
    if (!p->buf) return false;
    p->cap   = cap;
    p->usado = 0;
    p->buf[p->usado++] = '\0';   /* offset 0 = cadena vacía */
    return true;
}

static void pool_destroy(Pool *p)
{
    free(p->buf);
    p->buf = NULL;
    p->cap = p->usado = 0;
}

/* >>> pool_add  */

static uint32_t pool_add(Pool *p, const char *s)
{
    size_t tam = strlen(s) + 1;

    if(p->usado + tam > p->cap)
    {
        size_t nueva = p->cap;
        while(p->usado + tam > nueva)
        {
            nueva *= 2;
        }
        char *nb = realloc(p->buf,nueva);
        if(!nb) return POOL_ERR;
        p->buf = nb;
        p->cap = nueva;
    }
    uint32_t off = (uint32_t)p->usado;
    memcpy(p->buf + p->usado,s,tam);
    p->usado += tam;
    return off;

}





/* ---------- Árbol ---------- */

bool arbol_init(Arbol *t, uint32_t cap_ini)
{
    if (cap_ini < 16) cap_ini = 16;
    t->v = malloc((size_t)cap_ini * sizeof *t->v);
    if (!t->v) return false;
    if (!pool_init(&t->cadenas, 4096)) { free(t->v); t->v = NULL; return false; }
    t->n   = 0;
    t->cap = cap_ini;
    return true;
}

void arbol_destroy(Arbol *t)
{
    if (!t) return;
    free(t->v);
    t->v = NULL;
    t->n = t->cap = 0;
    pool_destroy(&t->cadenas);
}

/*DUPLICO CAPACIDAD */

static bool arbol_crecer(Arbol *t)
{
    if (t->n < t->cap) return true;

    size_t nueva = (size_t)t->cap * 2;
    if (nueva > UINT32_MAX - 1) nueva = UINT32_MAX - 1;
    if (nueva <= t->cap) return false;          /* límite alcanzado */

    Nodo *nv = realloc(t->v, nueva * sizeof *t->v);
    if (!nv) return false;

    t->v   = nv;
    t->cap = (uint32_t)nueva;
    return true;
}

/*arbol_add */
uint32_t arbol_add(Arbol *t, const char *nombre, uint16_t modo, uint32_t padre)
{
    if(!t) return NODO_NULO;
    
    if(arbol_crecer(t) == false)
    {
        return NODO_NULO;
    }
    
    uint32_t nombre_off = pool_add(&t->cadenas,nombre);
    if(nombre_off == POOL_ERR)
    {
        return NODO_NULO;
    }
    uint32_t idx = t->n;
    t->n++;

    t->v[idx].bytes = 0;
    t->v[idx].bytes_ap = 0;
    t->v[idx].n_fich = 0;
    t->v[idx].nombre_off = nombre_off;
    t->v[idx].modo = modo;
    t->v[idx].padre = padre;
    t->v[idx].hijo = NODO_NULO;
    t->v[idx].hermano = NODO_NULO;
    if(padre != NODO_NULO)
    {
        t->v[idx].hermano = t->v[padre].hijo;
        t->v[padre].hijo = idx;
    }
    return idx;
}

const char *arbol_nombre(const Arbol *t, uint32_t idx)
{
    return t->cadenas.buf + t->v[idx].nombre_off;
}


size_t arbol_ruta(const Arbol *t, uint32_t idx, char *dst, size_t cap)
{
    uint32_t pila[256];
    size_t np = 0;

    for (uint32_t i = idx; i != NODO_NULO; i = t->v[i].padre) {
        if (np == 256) return (size_t)-1;       /* demasiado profundo */
        pila[np++] = i;
    }

    size_t len = 0;
    while (np > 0) {
        const char *nom = arbol_nombre(t, pila[--np]);
        size_t l = strlen(nom);

        if (len > 0 && len + 1 < cap) dst[len++] = '/';
        if (len + l + 1 > cap) return (size_t)-1;

        memcpy(dst + len, nom, l);
        len += l;
    }
    dst[len] = '\0';
    return len;
}
