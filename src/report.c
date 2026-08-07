#include "report.h"
//Felipe Angeriz 06-08-2026
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>
#include <inttypes.h>
#define BARRA_ANCHO 20
static void formatear(uint64_t bytes,char *dst,size_t cap)
{

    double bytes_local = (double)bytes;

    static const char *sufijos[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    
    size_t indice = 0;
    size_t n = sizeof(sufijos) / sizeof(sufijos[0]);

    while (bytes_local >= 1024 && indice < n - 1)
    {
        bytes_local /= 1024;
        indice++;
        
    }
    snprintf(dst, cap, "%.1f %s", bytes_local, sufijos[indice]);
}
static void imprimir(const Arbol *t, uint32_t idx, int nivel, int prof_max, uint64_t total_raiz)
{
    for (uint32_t h = t->v[idx].hijo; h != NODO_NULO; h = t->v[h].hermano)
    {
        char buf[32];
        formatear(t->v[h].bytes, buf, sizeof buf);

        double pct = total_raiz ? (100.0 * (double)t->v[h].bytes / (double)total_raiz) : 0.0;
        int n = (int)(pct * BARRA_ANCHO / 100.0);
        printf("%*s%10s  %.*s%.*s  %5.1f%%  %s\n",nivel * 2, "",buf,n * 3, "████████████████████",(BARRA_ANCHO - n) * 3, "░░░░░░░░░░░░░░░░░░░░",pct,arbol_nombre(t, h));

        if (nivel + 1 < prof_max)
        {
            imprimir(t, h, nivel + 1, prof_max, total_raiz);
        }
    }
}
void report_arbol(const Arbol *t, uint32_t raiz, int prof_max)
{
    imprimir(t, raiz, 0, prof_max, t->v[raiz].bytes);
}
static const Arbol *g_arbol;

static int cmp_nodos(const void *a, const void *b)
{
    uint32_t ia = *(const uint32_t *)a;
    uint32_t ib = *(const uint32_t *)b;

    uint64_t ba = g_arbol->v[ia].bytes;
    uint64_t bb = g_arbol->v[ib].bytes;

    if (ba > bb) return -1;
    if (ba < bb) return  1;
    return 0;
}

void report_ordenar(Arbol *t, uint32_t raiz)
{
    uint32_t n = 0;
    for (uint32_t h = t->v[raiz].hijo; h != NODO_NULO; h = t->v[h].hermano)
    {
        n++;
    }

    if (n < 2)
    {
        for (uint32_t h = t->v[raiz].hijo; h != NODO_NULO; h = t->v[h].hermano)
        {
            report_ordenar(t, h);
        }
        return;
    }

    uint32_t *arr = malloc((size_t)n * sizeof *arr);
    if (arr == NULL)
    {
        return;
    }

    uint32_t i = 0;
    for (uint32_t h = t->v[raiz].hijo; h != NODO_NULO; h = t->v[h].hermano)
    {
        arr[i] = h;
        i++;
    }

    g_arbol = t;
    qsort(arr, n, sizeof *arr, cmp_nodos);

    t->v[raiz].hijo = arr[0];
    for (i = 0; i + 1 < n; i++)
    {
        t->v[arr[i]].hermano = arr[i + 1];
    }
    t->v[arr[n - 1]].hermano = NODO_NULO;

    for (i = 0; i < n; i++)
    {
        report_ordenar(t, arr[i]);
    }

    free(arr);
}
