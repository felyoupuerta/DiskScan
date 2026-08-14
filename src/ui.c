/* ============================================================
 *  DiskScan — ui.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "   [ Escaneo de disco ]  -  v3.1");
    printf("    \xe2\x95\x91%-*s\xe2\x95\x91\n", UI_ANCHO, "   Analisis dispositivos de alamacenamiento");
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

bool ui_menu_interactivo(Opciones *o, char *ruta_buf, size_t cap,int flagbanner)
{
    if(flagbanner == 1)
    {
        ui_banner();
    }
    else
    {

    }
    printf("  Asistente guiado. Pulsa Enter para aceptar el valor\n");
    printf("  por defecto que aparece entre corchetes.\n\n");

    printf("[?] ¿Que carpeta quieres analizar? (Enter = carpeta actual) [.]: ");
    leer_linea(ruta_buf, cap);
    if (ruta_buf[0] == '\0')
    {
        snprintf(ruta_buf, cap, ".");
    }
    o->ruta = ruta_buf;

    o->solo_total = leer_si_no("¿Ver solo el tamano total, sin el detalle por carpetas?", false);

    if (o->solo_total)
    {
        o->prof_max = 1;
        o->bytes_exactos = false;
    }
    else
    {
        char linea[32];

        printf("[?] ¿Cuantos niveles de subcarpetas quieres ver? (1 = solo el primer nivel) [1]: ");
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

        o->bytes_exactos = leer_si_no("¿Mostrar el tamano en bytes exactos? (Enter = verlo como KB/MB/GB)", false);
    }
    printf("\n\n");

    printf("[INFO PARA USUARIO] Un 'disco montado' es un USB u otra unidad que aparece dentro de la carpeta.)\n\n");
    printf("\nCuando hayas leido presiona ENTER\n");
    getc(stdin);
    o->un_solo_fs = !leer_si_no("¿Analizar tambien otros discos o USB conectados dentro de esa carpeta? (normalmente No)", false);


    char linea[32];

    printf("[?] ¿Quieres ver la lista de los archivos mas grandes? Escribe cuantos (0 = no) [0]: ");
    
    leer_linea(linea,sizeof linea);
    if(linea[0] == '\0')
    {
        o->top_ficheros = 0;
    }
    else
    {
        o->top_ficheros = (int)strtol(linea,NULL,10);
        if(o->top_ficheros < 0)
        {
            o->top_ficheros = 0;
        }
    }
    printf("\n[*] Analizando \"%s\"...\n\n", o->ruta);
    return true;
}
