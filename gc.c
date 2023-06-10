#include "gc.h"

#include <stdlib.h>

#include "util.h"

obj *memory;
ll memory_size = 1024, alloc_ptr = 0, scan_ptr;

ptr **root_stack;
ll root_size = 1024, root_sp = 0;

void gc_init() {
    memory = malloc(memory_size * sizeof(obj));
    root_stack = malloc(root_size * sizeof(ptr *));
}

ptr gc_copy(ptr p) {
    ll start, size;
    switch (p.type) {
        case T_NUMBER:
        case T_SYMBOL:
        case T_CHARACTER:
        case T_PRIMITIVE:
        case T_INPUT_PORT:
        case T_OUTPUT_PORT:
        case T_NIL:
        case T_BOOL:
        case T_EOF:
        case T_UNBOUND:
            return p;
        case T_CONS:
        case T_HASHTABLE:
        case T_PROCEDURE:
        case T_MACRO:
        case T_ENVIRONMENT:
            start = p.index;
            break;
        case T_VECTOR:
            start = p.start;
            break;
        default:
            ASSERT(false);
    }
    switch (p.type) {
        case T_CONS:
        case T_ENVIRONMENT:
            size = 2;
            break;
        case T_HASHTABLE:
            size = HASHTABLE_P;
            break;
        case T_PROCEDURE:
        case T_MACRO:
            size = 3;
            break;
        case T_VECTOR:
            size = p.size;
            break;
        default:
            ASSERT(false);
    }
    if (start >= memory_size) return p;
    if (memory[start].moved) {
        switch (p.type) {
            case T_CONS:
            case T_ENVIRONMENT:
            case T_HASHTABLE:
            case T_PROCEDURE:
            case T_MACRO:
                p.index = memory[start].forward;
                return p;
            case T_VECTOR:
                p.start = memory[start].forward;
                return p;
            default:
                ASSERT(false);
        }
    }
    for (ll i = 0; i < size; i++) memory[alloc_ptr + i] = memory[start + i];
    memory[start].moved = true;
    memory[start].forward = alloc_ptr;
    switch (p.type) {
        case T_CONS:
        case T_ENVIRONMENT:
        case T_HASHTABLE:
        case T_PROCEDURE:
        case T_MACRO:
            p.index = alloc_ptr;
            alloc_ptr += size;
            return p;
        case T_VECTOR:
            p.start = alloc_ptr;
            alloc_ptr += size;
            return p;
        default:
            ASSERT(false);
    }
}

void gc_cycle() {
    // create to space
    memory = realloc(memory, 2 * memory_size * sizeof(obj));
    alloc_ptr = memory_size;
    scan_ptr = memory_size;
    // copy non-garbage
    for (ll i = 0; i < root_sp; i++) *root_stack[i] = gc_copy(*root_stack[i]);
    while (scan_ptr < alloc_ptr) {
        memory[scan_ptr].p = gc_copy(memory[scan_ptr].p);
        scan_ptr++;
    }
    // relocate
    for (ll i = memory_size; i < alloc_ptr; i++)
        memory[i - memory_size] = memory[i];
    alloc_ptr -= memory_size;
    for (ll i = 0; i < alloc_ptr; i++) {
        ASSERT(!memory[i].moved);
        switch (memory[i].p.type) {
            case T_NUMBER:
            case T_SYMBOL:
            case T_CHARACTER:
            case T_PRIMITIVE:
            case T_INPUT_PORT:
            case T_OUTPUT_PORT:
            case T_NIL:
            case T_EOF:
            case T_UNBOUND:
                break;
            case T_CONS:
            case T_HASHTABLE:
            case T_PROCEDURE:
            case T_MACRO:
            case T_ENVIRONMENT:
                memory[i].p.index -= memory_size;
                break;
            case T_VECTOR:
                memory[i].p.start -= memory_size;
            default:
                ASSERT(false);
        }
    }
    memory = realloc(memory, memory_size * sizeof(obj));
}

ll gc_alloc(ll size) {
#ifdef ALWAYS_GC
    gc_cycle();
#endif
    if (alloc_ptr + size >= memory_size) gc_cycle();
    while (alloc_ptr + size >= memory_size) memory_size *= 2;
    memory = realloc(memory, memory_size * sizeof(obj));
    ll p = alloc_ptr;
    for (ll i = 0; i < size; i++) {
        memory[p + i].moved = false;
        memory[p + i].p = nil;
    }
    alloc_ptr += size;
    return p;
}

ptr cons(ptr car, ptr cdr) {
    ptr p;
    p.type = T_CONS;
    p.index = gc_alloc(2);
    cons_setcar(p, car);
    cons_setcdr(p, cdr);
    return p;
}

ptr make_hash() {
    ptr p;
    p.type = T_HASHTABLE;
    p.index = gc_alloc(HASHTABLE_P);
    for (int i = 0; i < HASHTABLE_P; i++) memory[p.index + i].p = nil;
    return p;
}

ptr make_proc(ptr formals, ptr body, ptr env, int type) {
    ptr p;
    p.type = type;
    p.index = gc_alloc(3);
    memory[p.index].p = formals;
    memory[p.index + 1].p = body;
    memory[p.index + 2].p = env;
    return p;
}

ptr make_env(ptr car, ptr cdr) {
    ptr p = cons(car, cdr);
    p.type = T_ENVIRONMENT;
    return p;
}

ptr make_vector(ll size) {
    ptr p;
    p.type = T_VECTOR;
    p.start = gc_alloc(size);
    p.size = size;
    return p;
}

void push_root(ptr *p) {
    if (root_sp >= root_size) {
        root_size *= 2;
        root_stack = realloc(root_stack, root_size * sizeof(ptr *));
    }
    root_stack[root_sp++] = p;
}

void pop_root() {
    ASSERT(root_sp);
    root_sp--;
}
