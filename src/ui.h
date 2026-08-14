/* ============================================================
 *  DiskScan — ui.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef UI_H
#define UI_H

#include "opciones.h"

#include <stddef.h>
#include <stdbool.h>

void ui_banner(void);

bool ui_menu_interactivo(Opciones *o, char *ruta_buf, size_t cap,int flagbanner);

#endif /* UI_H */
