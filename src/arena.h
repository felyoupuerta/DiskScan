/* ============================================================
 *  DiskScan — arena.h
 *  Hecho por Felipe Angeriz
 * ============================================================ */
#ifndef ARENA_H
#define ARENA_H
#include<stddef.h>
#include<stdint.h>

#define ARENA_BLOQUE_DEF (1u << 20)

typedef struct Arena Arena;

//DEVUELVE NULL SI NO HAY MEMORIA
Arena *arena_new(size_t tam_bloque);

//RESERVA LOS n BYTES ALINEADOS A ALIGN --> !!TIENE Q SER POTENCIA DE 2
void *arena_alloc(Arena *a,size_t n, size_t align);

//LO MISMO PERO RELLENANDO A 0
void *arena_calloc(Arena *a, size_t n, size_t align);

//LIBERAR ARENA COMPLETA
void arena_free(Arena *a);

//STATS
size_t arena_reservado(const Arena *a); /* Bytes pedidos al sistema */
size_t arena_repartido(const Arena *a); /* Bytes entregados a el usuario */


//RESEVAR NODO CON  ALINEACION CORRECTA
#define ARENA_NEW(a,T)      ((T *)arena_alloc((a), sizeof(T), _Alignof(T)))
#define ARENA_NEW_N(a,T,n)  ((T *)arena_alloc((a), sizeof(T) * (n), _Alignof(T)))


#endif /* ARENA.H*/
