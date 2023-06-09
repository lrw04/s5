#ifndef OBARRAY_H
#define OBARRAY_H

#include "lisp.h"

#include <string.h>

typedef struct trie_node {
    ll depth, father, children[128];
    char path;
} trie_node;

extern trie_node *obarray;

void obarray_init();
ll obarray_intern(const char *s, ll n);

#endif
