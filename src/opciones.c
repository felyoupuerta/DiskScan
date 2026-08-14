/* ============================================================
 *  DiskScan — opciones.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#include "opciones.h"
#include "ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>   
#include <string.h>
#include<errno.h>


static const struct option largas[] = {
    {"depth",    required_argument, NULL, 'd'},
    {"bytes",    no_argument,       NULL, 'b'},
    {"one-fs",   no_argument,       NULL, 'x'},
    {"cross-fs", no_argument,       NULL,  1 },
    {"total",    no_argument,       NULL, 't'},
    {"afondo",   required_argument, NULL, 'a'},
    {"help",     no_argument,       NULL, 'h'},
    {"menu",     no_argument,       NULL, 'm'},
    {0, 0, 0, 0}
};
void opciones_uso(const char *prog)
{
    printf("Uso: %s [OPCIONES] RUTA\n\n", prog);
    printf("Analiza el espacio ocupadao por un directorio o fichero.\n");
    printf("Opciones: \n");
    printf("-d,  --depth N        niveles a mostrar(por defecto 1)\n");
    printf("-b,  --bytes          mostrar bytes exactos sin formatear\n");
    printf("-x,  --one-fs         no cruzar puntos de montaje (activado pòr defecto)\n");
    printf("     --cross-fs       PERMITIR cruzar puntos de montaje\n");
    printf("-t,  --total          mostrar solo el total en GB (sin arbol)\n");
    printf("-a,  --afondo N       mostrar el top de ficheros mas pesados N es la cantidad que quieres mostrar\n");
    printf("-m,  --menu           Ejecutar el menu interactivo (Más sencillo que por parámtros)\n");
    printf("-h,  --help           mostrar esta ayuda\n");
    
}


bool opciones_parse(Opciones *o, int argc, char **argv)
{
    //INICIALIZO TODO EL STRUCT
    o->ruta = NULL;
    o->prof_max = 1;
    o->bytes_exactos = false;
    o->un_solo_fs = true;
    o->solo_total = false;
    o->top_ficheros = 0;

    int c;
    char ruta_interactiva[4096];
    while((c = getopt_long(argc,argv,"d:bxtmha:",largas,NULL)) != -1)
    {
        switch (c)
        {
            case 'a':
                if(optarg == NULL)
                {
                    printf("I'M HERE?!? Something has gone pretty, pretty WRONG\n");
                    fprintf(stderr,"Error: %s",strerror(errno));
                    return false;
                }
                o->top_ficheros = (int)strtol(optarg,NULL,10);
                
                if(o->top_ficheros < 1)
                {
                    fprintf(stderr,"Error: %s", strerror(errno));
                    return false;
                }
                break;
            case 'd':
                o->prof_max = (int)strtol(optarg, NULL, 10);
                if (o->prof_max < 1)
                {
                    fprintf(stderr, "La profundidad debe ser al menos 1\n");
                    return false;
                }
                break;

            case 'b':
                o->bytes_exactos = true;
                break;
            case 'x':
                o->un_solo_fs = true;
                break;
            case 't':
                o->solo_total = true;
                break;
            case 1:
                o->un_solo_fs = false;
                break;
            case 'm':
                bool execute_menu = ui_menu_interactivo(o,ruta_interactiva,sizeof(ruta_interactiva),0);
                printf("[DEBUG] Ruta Interactiva guardada: %s\n",ruta_interactiva);
                if(execute_menu == false)
                {
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                opciones_uso(argv[0]);
                exit(EXIT_SUCCESS);
            
            case '?':
                opciones_uso(argv[0]);
                return false;
            default:
                break;
        }
    }
    if(optind < argc)
    {
        o->ruta = argv[optind];
    }
    else
    {
        fprintf(stderr,"Falta la ruta\n");
        opciones_uso(argv[0]);
        return false;
    }

    return true;
}
