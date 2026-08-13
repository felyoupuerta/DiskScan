#include "../src/tree.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    Arbol t;
    assert(arbol_init(&t, 2));          

    uint32_t raiz = arbol_add(&t, "home", 0, NODO_NULO);
    assert(raiz != NODO_NULO);
    assert(t.v[raiz].padre == NODO_NULO);
    assert(t.v[raiz].hijo  == NODO_NULO);

    uint32_t a = arbol_add(&t, "felipe", 0, raiz);
    uint32_t b = arbol_add(&t, "invitado", 0, raiz);
    uint32_t c = arbol_add(&t, "docs", 0, a);
    assert(a != NODO_NULO && b != NODO_NULO && c != NODO_NULO);

    assert(strcmp(arbol_nombre(&t, raiz), "home") == 0);
    assert(strcmp(arbol_nombre(&t, c), "docs") == 0);

    assert(t.v[raiz].hijo == b);
    assert(t.v[b].hermano == a);
    assert(t.v[a].hermano == NODO_NULO);
    assert(t.v[a].hijo == c);

    int nh = 0;
    for (uint32_t h = t.v[raiz].hijo; h != NODO_NULO; h = t.v[h].hermano) nh++;
    assert(nh == 2);

    char ruta[512];
    size_t l = arbol_ruta(&t, c, ruta, sizeof ruta);
    assert(l != (size_t)-1);
    assert(strcmp(ruta, "home/felipe/docs") == 0);

    for (int i = 0; i < 5000; i++) {
        char nom[32];
        snprintf(nom, sizeof nom, "fichero_%d", i);
        uint32_t x = arbol_add(&t, nom, 0, c);
        assert(x != NODO_NULO);
        assert(strcmp(arbol_nombre(&t, x), nom) == 0);
    }
    assert(strcmp(arbol_nombre(&t, raiz), "home") == 0);

    printf("nodos: %u, pool: %zu B\n", t.n, t.cadenas.usado);
    arbol_destroy(&t);
    puts("OK");
    return 0;
}
