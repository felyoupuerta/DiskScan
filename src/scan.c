/* ============================================================
 *  DiskScan — scan.c
 *  Hecho por Felipe Angeriz
 * ============================================================ */
//HEADER MIO
#include "scan.h"
//HEADERS SISTEMA
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include <stdint.h>




uint64_t scan_dir(Ctx *c,int dir_fd,uint32_t idx,int profundidad)
{
    if(profundidad > SCAN_PROFUNDIDAD_MAX)
    {
        return 0;
    }
    
    DIR *dir = fdopendir(dir_fd);

    if(dir == NULL)
    {
        c->n_errores++;
        return 0;
    }
    
    uint64_t total = 0;

    //USO DE FSTAT ----> int fstat(int fd, struct stat *buf);
    //DECLARO LA ESTRUCTUIRA PARA GUARDAR LOS DATOS RECOGIDOS CON STAT
    struct stat diskinfo;
    
    if(fstat(dir_fd,&diskinfo) == -1)
    {
        closedir(dir);
        c->n_errores++;
        return 0;
    }
    //SI RECOGEMOS BIEN LOS DATOS MULTIPLICAMOS EL ST_BLOCKS X 512 Y LO ASIGNAMOS A LA
    //VARIABLE TOTAL.
    total = (uint64_t)diskinfo.st_blocks * 512;
    
    struct dirent *e;
    while((e = readdir(dir)) != NULL)
    {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
        {
            continue;
        }
        struct stat ne;
        if(fstatat(dir_fd,e->d_name,&ne,AT_SYMLINK_NOFOLLOW) == -1)
        {
            //SI ES -1 LA ENTRADA NO SE PUDO LEER, PASAMOS A LA SIGUIENTE
            c->n_errores++;
            continue;
        }
        if(S_ISDIR(ne.st_mode))
        //RAMA DEL DIRECTORIO LINEAS 62-71
        {
            if(c->un_solo_fs && ne.st_dev != c->raiz_dev)
            {
                continue;
            }
 
            uint32_t indice = arbol_add(c->arbol,e->d_name,(uint16_t)ne.st_mode,idx);
            
            if(indice == NODO_NULO)
            {
                c->n_errores++;
                continue;
            }
            c->n_dirs++; 
            int feliopen =  openat(dir_fd,e->d_name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW 
                                                               | O_CLOEXEC);
            if(feliopen == -1)
            {
                //SI DA -1 SUELEN SER PERMISOS ASI Q CONTINUO
                c->n_errores++;
                continue;
            }
            total += scan_dir(c,feliopen,indice,profundidad + 1);
        }
        //RAMA DEL FICHERO
        else
        {
            c->n_fich++;
            uint32_t indice1 = arbol_add(c->arbol,e->d_name,(uint16_t)ne.st_mode,idx);
            if(indice1 == NODO_NULO)
            {
                c->n_errores++;
                continue;
            }
            uint64_t bytes = (uint64_t)ne.st_blocks * 512;
            if (c->dedup && ne.st_nlink > 1 && hl_visto_o_insertar(c->hl, ne.st_dev, ne.st_ino))
            {
                bytes = 0;
            }

            c->arbol->v[indice1].bytes = bytes;
            total += bytes;
        }

    }
    closedir(dir);
    c->arbol->v[idx].bytes = total;
    return total;
}