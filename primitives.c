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
    pop_root_n(2);
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
    pop_root_n(2);
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

ptr p_char_to_integer(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    ptr p = cons_car(a);
    ASSERT(p.type == T_CHARACTER);
    return make_number(p.character);
}

ptr p_integer_to_char(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    ptr p = cons_car(a);
    ASSERT(p.type == T_NUMBER);
    ld d = p.number;
    ASSERT(floorl(d) == d);
    ASSERT(0 <= d && d < 128);
    return make_char(d);
}

ptr p_vector(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    return make_bool(cons_car(a).type == T_VECTOR);
}

ptr p_make_vector(ptr a, ptr r) {
    push_root(&a);
    push_root(&r);
    ASSERT(list_length(a) && list_length(a) <= 2);
    ASSERT(cons_car(a).type == T_NUMBER);
    ll k = cons_car(a).number;
    ptr v = list_length(a) > 1 ? cons_car(cons_cdr(a)) : nil;
    ptr p = nil;
    push_root(&p);
    p = make_vector(k);
    for (ll i = 0; i < k; i++) vector_set(p, i, v);
    pop_root_n(3);
    return p;
}

ptr p_vector_length(ptr a, ptr r) {
    ASSERT(list_length(a) == 1);
    ASSERT(cons_car(a).type == T_VECTOR);
    return make_number(cons_car(a).size);
}

ptr p_vector_ref(ptr a, ptr r) {
    ASSERT(list_length(a) == 2);
    ptr v = cons_car(a), k = cons_car(cons_cdr(a));
    ASSERT(v.type == T_VECTOR);
    ASSERT(k.type == T_NUMBER);
    return vector_ref(v, k.number);
}

ptr p_vector_set(ptr a, ptr r) {
    ASSERT(list_length(a) == 3);
    ptr v = cons_car(a), k = cons_car(cons_cdr(a)),
        p = cons_car(cons_cdr(cons_cdr(a)));
    ASSERT(v.type == T_VECTOR);
    ASSERT(k.type == T_NUMBER);
    vector_set(v, k.number, p);
    return INTERN("vector-set!");
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
    pop_root_n(2);
    return e;
}
