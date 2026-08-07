#include "../src/hardlink.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    HLSet *s = hl_new();
    assert(s);
    assert(hl_cuenta(s) == 0);

    assert(hl_visto_o_insertar(s, 2049, 100) == false);
    assert(hl_visto_o_insertar(s, 2049, 100) == true);
    assert(hl_visto_o_insertar(s, 2049, 100) == true);
    assert(hl_cuenta(s) == 1);

    assert(hl_visto_o_insertar(s, 2050, 100) == false);
    assert(hl_cuenta(s) == 2);

    assert(hl_visto_o_insertar(s, 2049, 101) == false);
    assert(hl_cuenta(s) == 3);

    assert(hl_visto_o_insertar(s, 2049, 100) == true);
    assert(hl_visto_o_insertar(s, 2050, 100) == true);
    assert(hl_visto_o_insertar(s, 2049, 101) == true);
    assert(hl_cuenta(s) == 3);

    for (int i = 0; i < 50000; i++)
        assert(hl_visto_o_insertar(s, 8, (ino_t)(1000 + i)) == false);
    assert(hl_cuenta(s) == 50003);

    for (int i = 0; i < 50000; i++)
        assert(hl_visto_o_insertar(s, 8, (ino_t)(1000 + i)) == true);
    assert(hl_cuenta(s) == 50003);

    assert(hl_visto_o_insertar(s, 8, 999) == false);
    assert(hl_visto_o_insertar(s, 8, 999999) == false);

    printf("inodos unicos: %zu\n", hl_cuenta(s));
    hl_free(s);
    hl_free(NULL); 
    puts("OK");
    return 0;
}
