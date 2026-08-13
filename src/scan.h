/* ============================================================
 *  DiskScan — scan.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef SCAN_H
#define SCAN_H
#include "hardlink.h"
#include "tree.h"

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define SCAN_PROFUNDIDAD_MAX 200

typedef struct
{
    dev_t     raiz_dev;
    bool      un_solo_fs;
    uint64_t  n_fich;
    uint64_t  n_dirs;
    uint64_t  n_errores;
    HLSet     *hl;
    bool      dedup;
    Arbol     *arbol;
} Ctx;

//IDX indice de el nodo que repre el directorio
uint64_t scan_dir(Ctx *c,int dir_fd,uint32_t idx ,int profundidad);


#endif /* SCAN.H */
