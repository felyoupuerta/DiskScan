/* ============================================================
 *  DiskScan — main.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#include "scan.h"
#include "report.h"
#include "opciones.h"
#include "ui.h"
#include<stdio.h>
#include<fcntl.h>
#include<inttypes.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/stat.h>



int main(int argc, char **argv)
{
    Opciones opciones;
    char ruta_interactiva[4096];

    if(argc == 1)
    {
        ui_banner();

        bool comp = opciones_parse(&opciones, argc,argv);
        if(comp == false)
        {
            exit(EXIT_FAILURE);
        }
    }
    
    /*
    {
    
        if(ui_menu_interactivo(&opciones, ruta_interactiva, sizeof ruta_interactiva) == false)
        {
            exit(EXIT_FAILURE);
        }
    }
    */
    else
    {
        ui_banner();

        bool comp = opciones_parse(&opciones, argc,argv);
        if(comp == false)
        {
            exit(EXIT_FAILURE);
        }
    }


    Ctx ctx = {0};
    struct stat inf;

    ctx.un_solo_fs = opciones.un_solo_fs;
    ctx.dedup = true;

    int d_open = open(opciones.ruta, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    
    if(d_open == -1)
    {
        perror(opciones.ruta);
        exit(EXIT_FAILURE);
    }
    
    if(fstat(d_open,&inf) == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    } 

    ctx.raiz_dev = inf.st_dev;
    
    ctx.hl = hl_new();

    if(ctx.hl == NULL)
    {
        fprintf(stderr, "No hay memoria para la tabla de hardlinks\n");
        exit(EXIT_FAILURE);
    }
    
    Arbol arbol;

    if(!arbol_init(&arbol,4096))
    {
        exit(EXIT_FAILURE);
    }
     
    ctx.arbol = &arbol;

    uint32_t indice = arbol_add(&arbol,opciones.ruta,(uint16_t)inf.st_mode,NODO_NULO);
    if(indice == NODO_NULO)
    {
        fprintf(stderr,"Error al crear el nodo raiz\n");
        exit(EXIT_FAILURE);
    }
    uint64_t total = scan_dir(&ctx, d_open, indice, 0);

    if(opciones.solo_total)
    {
        double gb = (double)total / (1024.0 * 1024.0 * 1024.0);
        printf("Total: %.2f GB\n", gb);
    }
    else if(opciones.top_ficheros)
    {
        report_ficheros_top(&arbol,opciones.top_ficheros,opciones.bytes_exactos);
        printf("\nTotal:%" PRIu64 " bytes\n",total );
    }
    else
    {
        report_ordenar(&arbol, indice);
        report_arbol(&arbol, indice, opciones.prof_max);
        printf("\nTotal: %" PRIu64 " bytes\n", total);
    }

    arbol_destroy(&arbol);
    hl_free(ctx.hl);
    return 0;
}

