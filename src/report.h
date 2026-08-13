/* ============================================================
 *  DiskScan — report.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef REPORT_H
#define REPORT_H
#include <stdint.h>
#include "tree.h"

void report_ordenar(Arbol *t, uint32_t raiz);

void report_arbol(const Arbol *t, uint32_t raiz, int prof_max);

void report_ficheros_top(Arbol *t, int limite, bool exactos);

#endif /* REPORT_H */