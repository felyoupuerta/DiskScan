#include "../src/arena.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

int main(void)
{
    Arena *a = arena_new(256);
    assert(a);

    char *c = arena_alloc(a, 1, 1);
    uint64_t *q = ARENA_NEW(a, uint64_t);
    assert(((uintptr_t)q % 8) == 0);
    *q = 0xDEADBEEFCAFEULL;
    *c = 'x';

    assert(*c == 'x' && *q == 0xDEADBEEFCAFEULL);

    for (int i = 0; i < 1000; i++) {
        uint32_t *p = ARENA_NEW(a, uint32_t);
        assert(p);
        *p = (uint32_t)i;
        assert(*p == (uint32_t)i);
    }

    void *grande = arena_alloc(a, 10000, 16);
    assert(grande && ((uintptr_t)grande % 16) == 0);

    printf("reservado: %zu B, repartido: %zu B\n",
           arena_reservado(a), arena_repartido(a));

    arena_free(a);
    arena_free(NULL);          
    puts("OK");
    return 0;
}
