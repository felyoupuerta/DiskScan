#include "scan.h"
#include "report.h"
//Felipe Angeriz
#include<stdio.h>
#include<fcntl.h>
#include<inttypes.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/stat.h>

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("debes introducir la ruta con argumento\n");
        exit(EXIT_FAILURE);

    }
       
    Ctx ctx = {0};
    struct stat inf;

    ctx.un_solo_fs = true;
    ctx.dedup = true;

    int d_open = open(argv[1], O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    
    if(d_open == -1)
    {
        perror(argv[1]);
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

    uint32_t indice = arbol_add(&arbol,argv[1],(uint16_t)inf.st_mode,NODO_NULO);
    if(indice == NODO_NULO)
    {
        fprintf(stderr,"Error al crear el nodo raiz\n");
        exit(EXIT_FAILURE);
    }
    uint64_t total = scan_dir(&ctx, d_open, indice, 0);
    report_ordenar(&arbol, indice);
    report_arbol(&arbol, indice, 1);
    printf("\nTotal: %" PRIu64 " bytes\n", total);

    arbol_destroy(&arbol);
    hl_free(ctx.hl);
    return 0;
}

