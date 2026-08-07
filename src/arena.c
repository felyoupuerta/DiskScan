//MI HEADER
#include "arena.h"

#include<stdlib.h>
#include<string.h>
#include<stdint.h>

typedef struct Bloque
{
    struct Bloque *sig;
    size_t usado;
    size_t tam;
    unsigned char datos[];
} Bloque;


struct Arena
{
    Bloque *actual;
    size_t tam_bloque;
    size_t reservado;
    size_t repartido;
};

static size_t alinear(size_t n, size_t align)
{
    return (n + align - 1) & ~(align - 1);
}


static size_t offset_alineado(const unsigned char *base, size_t usado, size_t align)
{
    uintptr_t dir = (uintptr_t)(base + usado);
    return usado + (alinear(dir, align) - dir);
}


static Bloque *bloque_nuevo(size_t tam)
{
    Bloque *b = malloc(sizeof(Bloque) + tam);
    if(!b) return NULL;
    b->sig   = NULL;
    b->usado = 0;
    b->tam   = tam;
    return b;

}

Arena *arena_new(size_t tam_bloque)
{
    if(tam_bloque == 0)
    {
        tam_bloque = ARENA_BLOQUE_DEF;
    }
    Arena *a = malloc(sizeof *a);
    if(!a) return NULL;
    a->actual = bloque_nuevo(tam_bloque);

    if(!a->actual)
    {
        free(a);
        return NULL;
    }
    a->tam_bloque = tam_bloque;
    a->reservado = sizeof(Bloque) + tam_bloque;
    a->repartido = 0;
    return a;
}

void arena_free(Arena *a)
{

    if(!a)
    {
        return;
    }
    Bloque *b = a->actual;
    
    while(b)
    {
        Bloque *sig = b->sig;
        free(b);
        b = sig;
    }
    free(a);
}

void *arena_alloc(Arena *a, size_t n, size_t align)
{
    if (!a || align == 0) return NULL;   /* guarda: salir YA */

    Bloque *b  = a->actual;              /* ahora sí, ámbito de función */
    size_t off = offset_alineado(b->datos,b->usado,align); 

    if (off + n > b->tam) {
        size_t tam = a->tam_bloque;
        if (n + align > tam) tam = n + align;

        Bloque *nb = bloque_nuevo(tam);
        if (!nb) return NULL;

        nb->sig       = a->actual;
        a->actual     = nb;
        a->reservado += sizeof(Bloque) + tam;

        b   = nb;
        off = offset_alineado(b->datos,0,align);
    }

    void *p = b->datos + off;
    b->usado = off + n;
    a->repartido += n;
    return p;
}

void *arena_calloc(Arena *a, size_t n, size_t align)
{
    void *p = arena_alloc(a,n,align);
    if(p) memset(p,0,n);
    return p;
}

size_t arena_reservado(const Arena *a) { return a ? a->reservado : 0; }
size_t arena_repartido(const Arena *a) { return a ? a->repartido : 0; }

