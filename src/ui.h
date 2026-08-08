/* ============================================================
 *  DiskScan — ui.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef UI_H
#define UI_H

#include "opciones.h"

#include <stddef.h>
#include <stdbool.h>

/* Imprime la cabecera ASCII del programa */
void ui_banner(void);

/* Asistente guiado: rellena o mediante preguntas por stdin.
   ruta_buf/cap es el almacenamiento donde se guarda la ruta
   introducida por el usuario (o->ruta apuntará ahí). */
bool ui_menu_interactivo(Opciones *o, char *ruta_buf, size_t cap);

#endif /* UI_H */
