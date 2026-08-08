/* ============================================================
 *  DiskScan — hardlink.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef HARDLINK_H
#define HARDLINK_H

#include<stdbool.h>
#include<stdint.h>
#include<sys/types.h>



typedef struct HLSet HLSet;


HLSet *hl_new(void);
void   hl_free(HLSet *s);




/*
TRUE:  ya estaba no sumar tamaño
FALSE: es nuevo suma; queda registrado
*/
bool hl_visto_o_insertar(HLSet *s, dev_t dev, ino_t ino);

size_t hl_cuenta(const HLSet *s); /* CUANTOS INODOS HAY */



#endif /* hardlink.h */
