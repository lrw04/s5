#include "eval.h"

#include "gc.h"
#include "util.h"

ptr lookup(ptr r, ptr v) {
    if (eq(r, nil)) return unbound;
    ASSERT(r.type == T_ENVIRONMENT);
    ASSERT(v.type == T_SYMBOL);
    int h = hash(v);
    ptr l = hashtable_at(env_car(r), h);
    for (ptr c = l; !eq(c, nil); c = cons_cdr(c)) {
        ptr car = cons_car(c);
        if (eq(cons_car(car), v))
            return eq(cons_cdr(car), unbound) ? unbound : car;
    }
    return lookup(env_cdr(r), v);
}

ptr evlis(ptr vs, ptr r) {
    push_root(&vs);
    push_root(&r);
    if (eq(vs, nil)) return nil;
    ptr evcar = nil, evcdr = nil, ev = nil;
    push_root(&evcar);
    push_root(&evcdr);
    push_root(&ev);
    evcar = eval(cons_car(vs), r);
    evcdr = evlis(cons_cdr(vs), r);
    ev = cons(evcar, evcdr);
    pop_root();
    pop_root();
    pop_root();
    pop_root();
    pop_root();
    return ev;
}

ptr make_frame(ptr formals, ptr args) {
    push_root(&formals);
    push_root(&args);
    ptr f = nil;
    push_root(&f);
    f = make_hash();
    while (!eq(formals, nil)) {
        if (formals.type == T_SYMBOL) {
            set_hash(f, formals, args);
            break;
        } else {
            ASSERT(cons_car(formals).type == T_SYMBOL);
            set_hash(f, cons_car(formals), cons_car(args));
            formals = cons_cdr(formals);
            args = cons_cdr(args);
        }
    }
    pop_root();
    pop_root();
    pop_root();
    return f;
}

ptr eval(ptr e, ptr r) {
eval_start:
    push_root(&e);
    push_root(&r);
    if (e.type != T_CONS) {
        ASSERT(!eq(e, nil));
        if (e.type != T_SYMBOL) {
            pop_root();
            pop_root();
            return e;
        }
        ptr p = lookup(r, e);
        ASSERT(!eq(p, unbound));
        pop_root();
        pop_root();
        return cons_cdr(p);
    }
    ASSERT(list_p(e));
    ptr car = cons_car(e);
    if (eq(car, INTERN("quote"))) {
        ASSERT(eq(cons_cdr(cons_cdr(e)), nil));
        pop_root();
        pop_root();
        return cons_car(cons_cdr(e));
    }
    if (eq(car, INTERN("if"))) {
        // (if cond conseq alt)
        // (if cond conseq)
        ASSERT(list_length(e) == 3 || list_length(e) == 4);
        ptr condition = cons_car(cons_cdr(e));
        ptr consequence = cons_car(cons_cdr(cons_cdr(e)));
        ptr alternative = list_length(e) == 4
                              ? cons_car(cons_cdr(cons_cdr(cons_cdr(e))))
                              : make_bool(false);
        ptr result = nil;
        push_root(&result);
        result = eval(condition, r);
        pop_root();
        pop_root();
        if (eq(result, make_bool(false))) {
            e = alternative;
            pop_root();
            goto eval_start;
        }
        e = consequence;
        pop_root();
        goto eval_start;
    }
    if (eq(car, INTERN("set!"))) {
        // (set! var val)
        ASSERT(list_length(e) == 3);
        ptr var = cons_car(cons_cdr(e)),
            val = eval(cons_car(cons_cdr(cons_cdr(e))), r);
        ptr p = lookup(r, var);
        set_hash(env_car(r), var, val);
        pop_root();
        pop_root();
        return var;
    }
    if (eq(car, INTERN("lambda"))) {
        ASSERT(list_length(e) > 2);
        ptr formals = cons_car(cons_cdr(e));
        ptr body = cons_cdr(cons_cdr(e));
        pop_root();
        pop_root();
        return make_proc(formals, body, r, T_PROCEDURE);
    }
    if (eq(car, INTERN("syntax"))) {
        ASSERT(list_length(e) > 2);
        ptr formals = cons_car(cons_cdr(e));
        ptr body = cons_cdr(cons_cdr(e));
        pop_root();
        pop_root();
        return make_proc(formals, body, r, T_MACRO);
    }
    if (eq(car, INTERN("begin"))) {
        ptr body = cons_cdr(e);
        pop_root();
        pop_root();
        if (eq(body, nil)) return nil;
    }

    // application
}
