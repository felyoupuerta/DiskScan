/* ============================================================
 *  DiskScan — opciones.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef OPCIONES_H
#define OPCIONES_H

#include<stdbool.h>

typedef struct {
    const char *ruta;
    int  prof_max;
    bool bytes_exactos;
    bool un_solo_fs;
    bool solo_total;
} Opciones;

bool opciones_parse(Opciones *o, int argc, char **argv);
void opciones_uso(const char *prog);

#endif /*  OPCIONES_H  */