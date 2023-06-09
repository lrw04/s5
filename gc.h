#ifndef GC_H
#define GC_H

#include "lisp.h"

// #define ALWAYS_GC

void gc_init();

ptr cons(ptr car, ptr cdr);
ptr make_hash();
ptr make_proc(ptr formals, ptr body, ptr env, int type);
ptr make_env(ptr car, ptr cdr);
ptr make_vector(ll size);

extern ptr **root_stack;
extern ll root_size, root_sp;
void push_root(ptr *p);
void pop_root();

#endif
