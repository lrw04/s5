#include "obarray.h"

#include <stdlib.h>

trie_node *obarray;
ll obarray_size = 1024, obarray_allocp = 0, obarray_root = 0;

void obarray_node_init(ll p) {
    obarray[p].depth = 0;
    obarray[p].father = -1;
    obarray[p].path = 0;
    for (int i = 0; i < 128; i++) obarray[p].children[i] = -1;
}

void obarray_init() {
    obarray = malloc(obarray_size * sizeof(trie_node));
    obarray_node_init(obarray_allocp++);
}

ll obarray_alloc() {
    while (obarray_allocp == obarray_size) obarray_size *= 2;
    obarray = realloc(obarray, obarray_size * sizeof(trie_node));
    return obarray_allocp++;
}

ll obarray_intern(const char *s, ll n) {
    ll u = obarray_root;
    for (ll i = 0; i < n; i++) {
        ll *v = &(obarray[u].children[s[i]]);
        if (*v < 0) {
            *v = obarray_alloc();
            obarray_node_init(*v);
            obarray[*v].depth = obarray[u].depth + 1;
            obarray[*v].father = u;
            obarray[*v].path = s[i];
        }
        u = *v;
    }
    return u;
}
