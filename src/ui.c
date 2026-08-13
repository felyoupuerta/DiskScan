/* ============================================================
 *  DiskScan — ui.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UI_ANCHO 50

static bool es_terminal(void)
{
    return isatty(fileno(stdout)) != 0;
}

static void repetir_utf8(const char *s, int n)
{
    for (int i = 0; i < n; i++)
    {
        fputs(s, stdout);
    }
}

void ui_banner(void)
{
    const char *verde  = es_terminal() ? "\033[1;32m" : "";
    const char *reset  = es_terminal() ? "\033[0m"    : "";

    printf("%s", verde);
    printf("\n    \xe2\x95\x94"); repetir_utf8("\xe2\x95\x90", UI_ANCHO); printf("\xe2\x95\x97\n");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "   [ Escaneo de disco ]  -  v1.0");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "   Analizador de espacio en disco");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "   Felipe Angeriz - Agosto 2026");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "");
    printf("    \xe2\x95\x9a"); repetir_utf8("\xe2\x95\x90", UI_ANCHO); printf("\xe2\x95\x9d\n");
    printf("%s\n", reset);
}

static void leer_linea(char *buf, size_t cap)
{
    if (fgets(buf, (int)cap, stdin) == NULL)
    {
        buf[0] = '\0';
        return;
    }

    size_t l = strlen(buf);
    if (l > 0 && buf[l - 1] == '\n')
    {
        buf[l - 1] = '\0';
    }
}

static bool leer_si_no(const char *pregunta, bool valor_defecto)
{
    char linea[16];

    printf("[?] %s [%s]: ", pregunta, valor_defecto ? "S/n" : "s/N");
    leer_linea(linea, sizeof linea);

    if (linea[0] == '\0')
    {
        return valor_defecto;
    }
    return (linea[0] == 's' || linea[0] == 'S');
}

bool ui_menu_interactivo(Opciones *o, char *ruta_buf, size_t cap)
{
    ui_banner();
    
    printf("  Asistente guiado. Pulsa Enter para aceptar el valor\n");
    printf("  por defecto que aparece entre corchetes.\n\n");

    printf("[?] Ruta a analizar [.]: ");
    leer_linea(ruta_buf, cap);
    if (ruta_buf[0] == '\0')
    {
        snprintf(ruta_buf, cap, ".");
    }
    o->ruta = ruta_buf;

    o->solo_total = leer_si_no("¿Mostrar solo el total en GB (equivale a -t)?", false);

    if (o->solo_total)
    {
        o->prof_max = 1;
        o->bytes_exactos = false;
    }
    else
    {
        char linea[32];

        printf("[?] Profundidad de niveles a mostrar [1]: ");
        leer_linea(linea, sizeof linea);

        if (linea[0] == '\0')
        {
            o->prof_max = 1;
        }
        else
        {
            o->prof_max = (int)strtol(linea, NULL, 10);
            if (o->prof_max < 1)
            {
                fprintf(stderr, "[!] La profundidad debe ser al menos 1\n");
                return false;
            }
        }

        o->bytes_exactos = leer_si_no("¿Mostrar bytes exactos sin formatear?", false);
    }

    o->un_solo_fs = !leer_si_no("¿Cruzar puntos de montaje?", false);

    printf("\n[*] Analizando \"%s\"...\n\n", o->ruta);

    return true;
}
