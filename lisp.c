#include "lisp.h"

#include "gc.h"
#include "obarray.h"
#include "util.h"

ptr make_number(ld number) {
    ptr p;
    p.type = T_NUMBER;
    p.number = number;
    return p;
}

ptr intern(const char *s, ll n) {
    ptr p;
    p.type = T_SYMBOL;
    p.index = obarray_intern(s, n);
    return p;
}

ptr make_char(char character) {
    ptr p;
    p.type = T_CHARACTER;
    p.character = character;
    return p;
}

ptr make_bool(bool boolean) {
    ptr p;
    p.type = T_BOOL;
    p.boolean = boolean;
    return p;
}

ptr make_input_port(FILE *port) {
    ptr p;
    p.type = T_INPUT_PORT;
    p.port = port;
    return p;
}

ptr make_output_port(FILE *port) {
    ptr p;
    p.type = T_OUTPUT_PORT;
    p.port = port;
    return p;
}

ptr make_primitive(ll index) {
    ptr p;
    p.type = T_PRIMITIVE;
    p.index = index;
    return p;
}

ptr make_eof() {
    ptr p;
    p.type = T_EOF;
    return p;
}

ptr make_nil() {
    ptr p;
    p.type = T_NIL;
    return p;
}

ptr make_unbound() {
    ptr p;
    p.type = T_UNBOUND;
    return p;
}

bool eq(ptr a, ptr b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case T_NUMBER:
            return a.number == b.number;
        case T_SYMBOL:
        case T_CONS:
        case T_HASHTABLE:
        case T_PROCEDURE:
        case T_PRIMITIVE:
        case T_MACRO:
        case T_ENVIRONMENT:
            return a.index == b.index;
        case T_CHARACTER:
            return a.character == b.character;
        case T_VECTOR:
            return a.start == b.start;
        case T_INPUT_PORT:
        case T_OUTPUT_PORT:
            return a.port == b.port;
        default:
            ASSERT(false);
    }
}

ptr cons_car(ptr p) {
    ASSERT(p.type == T_CONS);
    return memory[p.index].p;
}

ptr cons_cdr(ptr p) {
    ASSERT(p.type == T_CONS);
    return memory[p.index + 1].p;
}

void cons_setcar(ptr p, ptr car) {
    ASSERT(p.type == T_CONS);
    memory[p.index].p = car;
}

void cons_setcdr(ptr p, ptr cdr) {
    ASSERT(p.type == T_CONS);
    memory[p.index + 1].p = cdr;
}

ptr hashtable_at(ptr p, int h) {
    ASSERT(p.type == T_HASHTABLE);
    return memory[p.index + h].p;
}

ptr get_hash(ptr p, ptr k) {
    ASSERT(p.type == T_HASHTABLE);
    ptr l = hashtable_at(p, hash(k));
    for (ptr c = l; !eq(c, nil); c = cons_cdr(c)) {
        ptr h = cons_car(c);
        if (eq(cons_car(h), k)) return cons_cdr(h);
    }
    return unbound;
}

void set_hash(ptr p, ptr k, ptr v) {
    ASSERT(p.type == T_HASHTABLE);
    ptr l = hashtable_at(p, hash(k));
    for (ptr c = l; !eq(c, nil); c = cons_cdr(c)) {
        ptr h = cons_car(c);
        if (eq(cons_car(h), k)) {
            cons_setcdr(h, v);
            return;
        }
    }
    ptr pair = nil;
    push_root(&pair);
    pair = cons(k, v);
    ptr c = cons(pair, p);
    memory[p.index + hash(k)].p = c;
}

ptr proc_formals(ptr p) {
    ASSERT(p.type == T_PROCEDURE || p.type == T_MACRO);
    return memory[p.index].p;
}

ptr proc_body(ptr p) {
    ASSERT(p.type == T_PROCEDURE || p.type == T_MACRO);
    return memory[p.index + 1].p;
}

ptr proc_env(ptr p) {
    ASSERT(p.type == T_PROCEDURE || p.type == T_MACRO);
    return memory[p.index + 2].p;
}

ptr env_car(ptr p) {
    ASSERT(p.type == T_ENVIRONMENT);
    return memory[p.index].p;
}

ptr env_cdr(ptr p) {
    ASSERT(p.type == T_ENVIRONMENT);
    return memory[p.index + 1].p;
}

ptr vector_ref(ptr p, ll i) {
    ASSERT(p.type == T_VECTOR);
    ASSERT(0 <= i && i < p.size);
    return memory[p.start + i].p;
}

void vector_set(ptr p, ll i, ptr v) {
    ASSERT(p.type == T_VECTOR);
    ASSERT(0 <= i && i < p.size);
    memory[p.start + i].p = v;
}

bool vector_stringp(ptr p) {
    ASSERT(p.type == T_VECTOR);
    for (ll i = 0; i < p.size; i++)
        if (memory[p.start + i].p.type != T_CHARACTER) return false;
    return true;
}

int next_hash(int prev, byte cur) { return (prev * 256 + cur) % HASHTABLE_P; }

#define MAKE_HASH(T)                                                      \
    int hash_##T(int prev, T cur) {                                       \
        byte c[sizeof(T)];                                                \
        memcpy(c, &cur, sizeof(T));                                       \
        for (int i = 0; i < sizeof(T); i++) prev = next_hash(prev, c[i]); \
        return prev;                                                      \
    }

MAKE_HASH(int)
MAKE_HASH(ld)
MAKE_HASH(ll)
MAKE_HASH(char)

#undef MAKE_HASH

int hash_port(int prev, FILE *cur) {
    byte c[sizeof(FILE *)];
    memcpy(c, &cur, sizeof(FILE *));
    for (int i = 0; i < sizeof(FILE *); i++) prev = next_hash(prev, c[i]);
    return prev;
}

int hash(ptr p) {
    int h = 0;
    h = hash_int(h, p.type);
    switch (p.type) {
        case T_NUMBER:
            return hash_ld(h, p.number);
        case T_SYMBOL:
        case T_PRIMITIVE:
        case T_ENVIRONMENT:
            return hash_ll(h, p.index);
        case T_CHARACTER:
            return hash_char(h, p.character);
        case T_INPUT_PORT:
        case T_OUTPUT_PORT:
            return hash_port(h, p.port);
        default:
            ASSERT(false);  // cannot hash pointers that might change after GC
                            // cycle
    }
}