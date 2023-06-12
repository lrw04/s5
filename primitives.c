#include "primitives.h"

#include <math.h>

#include "gc.h"
#include "obarray.h"
#include "util.h"

ptr p_eq(ptr a, ptr r) {
    ASSERT(list_length(a) == 2);
    return make_bool(eq(cons_car(a), cons_car(cons_cdr(a))));
}

ptr p_number(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_NUMBER);
}

bool integerp(ld a) { return floorl(a) == a; }

ptr p_integer(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_NUMBER &&
                     integerp(cons_car(a).number));
}

ptr p_not(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(eq(cons_car(a), make_bool(false)));
}

ptr p_boolean(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_BOOL);
}

ptr p_pair(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_CONS);
}

ptr p_cons(ptr a, ptr r) {
    push_root(&a);
    push_root(&r);
    ASSERT(list_length(a) == 2);
    ptr p = cons(cons_car(a), cons_car(cons_cdr(a)));
    pop_root();
    pop_root();
    return p;
}

ptr p_car(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return cons_car(cons_car(a));
}

ptr p_cdr(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return cons_cdr(cons_car(a));
}

ptr p_set_car(ptr a, ptr r) {
    cons_setcar(cons_car(a), cons_car(cons_cdr(a)));
    return INTERN("set-car!");
}

ptr p_set_cdr(ptr a, ptr r) {
    cons_setcdr(cons_car(a), cons_car(cons_cdr(a)));
    return INTERN("set-cdr!");
}

ptr p_null(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_NIL);
}

ptr p_symbol(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_SYMBOL);
}

ptr p_symbol_to_string(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    ASSERT(cons_car(a).type == T_SYMBOL);
    push_root(&a);
    push_root(&r);
    ll u = cons_car(a).index, n = obarray[u].depth;
    ptr v = make_vector(n);
    for (ll i = n - 1; i >= 0; i--) {
        vector_set(v, i, make_char(obarray[u].path));
        u = obarray[u].father;
    }
    pop_root();
    pop_root();
    return v;
}

ptr p_string_to_symbol(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    ASSERT(cons_car(a).type == T_VECTOR);
    ASSERT(vector_stringp(cons_car(a)));
    ptr p = cons_car(a);
    ll n = p.size;
    char *s = malloc((n + 1) * sizeof(char));
    for (ll i = 0; i < n; i++) {
        s[i] = vector_ref(p, i).character;
    }
    s[n] = 0;
    ptr ret = INTERN(s);
    free(s);
    return ret;
}

ptr p_char(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_CHARACTER);
}

ptr make_initial_environment() {
    ptr f = nil, e = nil;
    push_root(&f);
    push_root(&e);
    f = make_hash();
    ll n = (sizeof prim_f) / (sizeof(primitive_t));
    for (ll i = 0; i < n; i++) {
        set_hash(f, INTERN(prim_name[i]), make_primitive(i));
    }
    e = make_env(f, nil);
    pop_root();
    pop_root();
    return e;
}
