#include "scan.h"

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


    uint64_t total = scan_dir(&ctx,d_open,indice,0);
    
    printf("%" PRIu64 " bytes\n",total);
    printf("%" PRIu64 " ficheros\n",ctx.n_fich);
    printf("%" PRIu64 " Directorios\n",ctx.n_dirs);
    printf("%" PRIu64 " Errores encontrados\n",ctx.n_errores);
    printf("%u nodos\n", arbol.n);
    printf("raiz: %" PRIu64 "\n", arbol.v[indice].bytes);
    arbol_destroy(&arbol);
    hl_free(ctx.hl);
    return 0;
}




/*
 uint32_t arbol_add(Arbol *t, const char *nombre, uint16_t modo, uint32_t padre);
 bool arbol_init(Arbol *t, uint32_t cap_ini);

 12 typedef struct
 13 {
 14     dev_t     raiz_dev;
 15     bool      un_solo_fs;
 16     uint64_t  n_fich;
 17     uint64_t  n_dirs;
 18     uint64_t  n_errores;
 19     HLSet     *hl;
 20     bool      dedup;
 21     Arbol     *arbol;
 22 } Ctx;
 23
 24 //IDX indice de el nodo que repre el directorio
 25 uint64_t scan_dir(Ctx *c,int dir_fd,uint32_t idx ,int profundidad);

*/
