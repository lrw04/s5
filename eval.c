#include "eval.h"

#include "gc.h"
#include "printer.h"
#include "util.h"
#include "primitives.h"

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
    ASSERT(vs.type == T_CONS || vs.type == T_NIL);
    push_root(&vs);
    push_root(&r);
    if (eq(vs, nil)) {
        pop_root();
        pop_root();
        return nil;
    }
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
            args = nil;
            break;
        } else {
            ASSERT(cons_car(formals).type == T_SYMBOL);
            set_hash(f, cons_car(formals), cons_car(args));
            formals = cons_cdr(formals);
            args = cons_cdr(args);
        }
    }
    ASSERT(eq(args, nil));
    pop_root();
    pop_root();
    pop_root();
    return f;
}

// enum trampoline_state {
//     S_EVAL,
//     S_RESUME,
//     S_APPLY,
// };

// // trampoline:
// // eval(e, r, k) -> trampoline(S_EVAL, e, r, k);
// // resume(k, v) -> trampoline(S_RESUME, k, v, nil);
// // apply(f, args, k) -> trampoline(S_APPLY, f, args, k);

// ptr trampoline(enum trampoline_state func, ptr arg1, ptr arg2, ptr arg3) {
//     push_root(&arg1);
//     push_root(&arg2);
//     push_root(&arg3);
//     while (true) {
//         switch (func) {
//             case S_EVAL:
//                 ptr *e = &arg1, *r = &arg2, *k = &arg3;
//                 if (e->type != T_CONS) {
//                     ASSERT(!eq(*e, nil));
//                     if (e->type != T_SYMBOL) {
//                         pop_root();
//                         pop_root();
//                         pop_root();
//                         return *e;
//                     }

//                 }
//         }
//     }
// }

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
        push_root(&body);
        ASSERT(list_length(body));
        while (!eq(cons_cdr(body), nil)) {
            eval(cons_car(body), r);
            body = cons_cdr(body);
        }
        e = cons_car(body);
        pop_root();
        pop_root();
        pop_root();
        goto eval_start;
    }

    // application
    ptr func = nil, args = nil, frame = nil, body = nil;
    push_root(&func);
    push_root(&args);
    push_root(&frame);
    push_root(&body);
    func = eval(car, r);
    switch (func.type) {
        case T_PROCEDURE:
            args = evlis(cons_cdr(e), r);
            frame = make_frame(proc_formals(func), args);
            r = make_env(frame, proc_env(func));
            body = proc_body(func);
            ASSERT(list_length(body));
            while (!eq(cons_cdr(body), nil)) {
                eval(cons_car(body), r);
                body = cons_cdr(body);
            }
            e = cons_car(body);
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            goto eval_start;
            break;
        case T_MACRO:
            args = cons_cdr(e);
            frame = make_frame(proc_formals(func), args);
            ptr orig_env = r;
            push_root(&orig_env);
            r = make_env(frame, proc_env(func));
            body = proc_body(func);
            push_root(&body);
            ASSERT(list_length(body));
            ptr res = nil;
            push_root(&res);
            while (!eq(body, nil)) {
                res = eval(cons_car(body), r);
                body = cons_cdr(body);
            }
            e = res;
            r = orig_env;
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            goto eval_start;
            break;
        case T_PRIMITIVE:
            args = evlis(cons_cdr(e), r);
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            pop_root();
            return prim_f[func.index](args, r);
            break;
        default:
            ASSERT(false);
    }
}
