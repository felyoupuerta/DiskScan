/* ============================================================
 *  DiskScan — hardlink.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#include "hardlink.h"
#include "arena.h"
#include <stdlib.h>
#include <string.h>
#define HL_CAJONES_INI 1024u


typedef struct Entrada
{
    struct Entrada *sig;
    dev_t dev;
    ino_t ino;
} Entrada;



struct HLSet 
{
    Entrada **cajones;
    size_t n_cajones;
    size_t n;
    Arena *arena;
};

//FUNCION DEL HASH

static uint64_t hash_clave(dev_t dev, ino_t ino)
{

    uint64_t h = (uint64_t)dev * 1099511628211ULL;
    h ^= (uint64_t)ino;
    h *= 1099511628211ULL;
    h ^= h >> 32;
    return h;

}

HLSet *hl_new(void)
{

    HLSet *s = malloc(sizeof *s);
    
    if(!s) return NULL;
    
    //ARENA ME PIDE EL TAMAÑO DEL BLOQUE
    s->arena = arena_new(0);
    if(s->arena == NULL)
    {
        free(s);
        return NULL;
    }

    s->cajones = calloc(HL_CAJONES_INI,sizeof *s->cajones);
    
    if(s->cajones == NULL)
    {
        arena_free(s->arena);
        free(s);
        return NULL;
    }
    s->n_cajones = HL_CAJONES_INI;
    s->n = 0;
    return s;
}

void hl_free(HLSet *s)
{
    if(!s)
    {
        return;
    }
    
    free(s->cajones);
    arena_free(s->arena);
    free(s);
}

static bool hl_crecer(HLSet *s)
{
    size_t nueva_n = s->n_cajones * 2;
    Entrada **nuevos = calloc(nueva_n, sizeof *nuevos); 
    if (!nuevos) return false;

    for (size_t i = 0; i < s->n_cajones; i++) {
        Entrada *e = s->cajones[i];
        while (e) {
            Entrada *sig = e->sig;                        // guardo antes de romper el link
            uint64_t h = hash_clave(e->dev, e->ino);
            size_t c = (size_t)(h & (nueva_n - 1));
            e->sig = nuevos[c];
            nuevos[c] = e;
            e = sig;
        }
    }

    free(s->cajones);           // solo libero el array de punteros, no las Entrada
    s->cajones = nuevos;
    s->n_cajones = nueva_n;
    return true;
}

size_t hl_cuenta(const HLSet *s)
{
    return s ? s->n : 0; 
}

bool hl_visto_o_insertar(HLSet *s, dev_t dev, ino_t ino)
{
    if(!s)
    {
        return false;
    }

    uint64_t h = hash_clave(dev,ino);

    size_t c = (size_t)(h & (s->n_cajones - 1)); /* recalculada más abajo si hl_crecer() dispara */

    for(Entrada *e = s->cajones[c]; e ; e = e->sig)
    {
        if(e->dev == dev && e->ino == ino)
        {
            return true;
        }
    }
    
    Entrada *nueva = ARENA_NEW(s->arena,Entrada);
    //VALIDO LA CORRECTA EJECUCION DE ARENA NEW
    if(nueva == NULL)
    {
        return false;
    }
    //RELLENAR ESTRUCTURA
    nueva->dev = dev;
    nueva->ino = ino;

    if (s->n >= s->n_cajones * 3 / 4)
    {
        hl_crecer(s); /* si falla, seguimos con el tamaño viejo */
        c = (size_t)(h & (s->n_cajones - 1)); /* recalculo: n_cajones pudo cambiar */
    }
    //INSERTO AL PRINCIPIO DE EL CAJÓN
    nueva->sig = s->cajones[c]; /* ME LEO LO VIEJO */
    s->cajones[c] = nueva;      /* SOBREESCRIBO */
    s->n++;
    return false;
}
