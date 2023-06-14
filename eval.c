#include "eval.h"

#include "gc.h"
#include "primitives.h"
#include "printer.h"
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
    ASSERT(vs.type == T_CONS || vs.type == T_NIL);
    push_root(&vs);
    push_root(&r);
    if (eq(vs, nil)) {
        pop_root_n(2);
        return nil;
    }
    ptr evcar = nil, evcdr = nil, ev = nil;
    push_root(&evcar);
    push_root(&evcdr);
    push_root(&ev);
    evcar = eval(cons_car(vs), r);
    evcdr = evlis(cons_cdr(vs), r);
    ev = cons(evcar, evcdr);
    pop_root_n(5);
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
    pop_root_n(3);
    return f;
}

enum trampoline_state {
    S_EVAL,
    S_APPLY,
    S_BEGIN,
};

void print_hash(ptr h, ptr p) {
    fprintf(p.port, "(");
    for (ll i = 0; i < HASHTABLE_P; i++) {
        ptr l = hashtable_at(h, i);
        for (ptr i = l; !eq(i, nil); i = cons_cdr(i)) {
            print(cons_car(cons_car(i)), p);
            fprintf(p.port, ":");
            print(cons_cdr(cons_car(i)), p);
            fprintf(p.port, " ");
        }
    }
    fprintf(p.port, ")");
}

void print_env(ptr r, ptr p) {
    for (ptr i = r; !eq(i, nil); i = env_cdr(i)) {
        print_hash(env_car(i), p);
        fprintf(p.port, " ");
    }
}

// trampoline:
// eval(e, r) -> trampoline(S_EVAL, e, r);
// apply(f, args) -> trampoline(S_APPLY, f, args);
// begin(e*, r) -> trampoline(S_BEGIN, e*, r);

ptr eval(ptr e, ptr r);
ptr apply(ptr f, ptr args);
ptr begin(ptr es, ptr r);

ptr apply_arg(ptr args) {
    push_root(&args);
    ASSERT(list_length(args));
    if (eq(cons_cdr(args), nil)) {
        pop_root();
        return cons_car(args);
    }
    ptr p = nil;
    push_root(&p);
    p = cons(cons_car(args), apply_arg(cons_cdr(args)));
    pop_root_n(2);
    return p;
}

ptr trampoline(enum trampoline_state func, ptr arg1, ptr arg2) {
    while (true) {
        push_root(&arg1);
        push_root(&arg2);
        if (func == S_EVAL) {
            ptr *e = &arg1, *r = &arg2;
            if (e->type != T_CONS) {
                ASSERT(e->type != T_NIL);
                if (e->type != T_SYMBOL) {
                    pop_root_n(2);
                    return *e;
                }
                ptr p = nil;
                push_root(&p);
                p = lookup(*r, *e);
                // print(*e, make_output_port(stderr));
                // fprintf(stderr, " ");
                // print_env(*r, make_output_port(stderr));
                // fprintf(stderr, "\n\n");
                ASSERT(!eq(p, unbound));
                pop_root_n(3);
                return cons_cdr(p);
            }
            if (eq(cons_car(*e), INTERN("quote"))) {
                ASSERT(list_length(*e) == 2);
                pop_root_n(2);
                return cons_car(cons_cdr(*e));
            }
            if (eq(cons_car(*e), INTERN("if"))) {
                ASSERT(list_length(*e) > 2 && list_length(*e) <= 4);
                ptr cond = nil, conseq = nil, alt = nil, sel = nil;
                push_root(&cond);
                push_root(&conseq);
                push_root(&alt);
                push_root(&sel);
                cond = cons_car(cons_cdr(*e));
                conseq = cons_car(cons_cdr(cons_cdr(*e)));
                alt = list_length(*e) == 4
                          ? cons_car(cons_cdr(cons_cdr(cons_cdr(*e))))
                          : make_bool(false);
                sel = eval(cond, *r);
                if (eq(sel, make_bool(false))) {
                    arg1 = alt;
                    pop_root_n(6);
                    continue;
                }
                arg1 = conseq;
                pop_root_n(6);
                continue;
            }
            if (eq(cons_car(*e), INTERN("set!"))) {
                ASSERT(list_length(*e) == 3);
                ptr var = cons_car(cons_cdr(*e)),
                    val = eval(cons_car(cons_cdr(cons_cdr(*e))), *r);
                ASSERT(var.type == T_SYMBOL);
                set_hash(env_car(*r), var, val);
                pop_root_n(2);
                return var;
            }
            if (eq(cons_car(*e), INTERN("lambda"))) {
                ASSERT(list_length(*e) > 2);
                pop_root_n(2);
                return make_proc(cons_car(cons_cdr(*e)), cons_cdr(cons_cdr(*e)),
                                 *r, T_PROCEDURE);
            }
            if (eq(cons_car(*e), INTERN("syntax"))) {
                ASSERT(list_length(*e) > 2);
                pop_root_n(2);
                return make_proc(cons_car(cons_cdr(*e)), cons_cdr(cons_cdr(*e)),
                                 *r, T_MACRO);
            }
            if (eq(cons_car(*e), INTERN("begin"))) {
                arg1 = cons_cdr(*e);
                func = S_BEGIN;
                pop_root_n(2);
                continue;
            }
            ptr f = nil, args = nil;
            push_root(&f);
            push_root(&args);
            f = eval(cons_car(*e), *r);
            if (f.type == T_PROCEDURE || f.type == T_PRIMITIVE) {
                args = evlis(cons_cdr(*e), *r);
                // return apply(f, args);
                func = S_APPLY;
                arg1 = f;
                arg2 = args;
                pop_root_n(4);
                continue;
            } else if (f.type == T_MACRO) {
                args = cons_cdr(*e);
                arg1 = apply(f, args);
                pop_root_n(4);
                continue;
            } else {
                ASSERT(false);
            }
        } else if (func == S_APPLY) {
            ptr *f = &arg1, *args = &arg2;
            if (f->type != T_PRIMITIVE) {
                ptr frame = nil, new_env = nil, body = nil;
                push_root(&frame);
                push_root(&new_env);
                push_root(&body);
                frame = make_frame(proc_formals(*f), *args);
                new_env = make_env(frame, proc_env(*f));
                body = proc_body(*f);
                func = S_BEGIN;
                arg1 = body;
                arg2 = new_env;
                pop_root_n(5);
                continue;
            } else {
                if (!strcmp(prim_name[f->index], "eval")) {
                    ASSERT(list_length(*args) == 2);
                    func = S_EVAL;
                    arg1 = cons_car(*args);
                    arg2 = cons_car(cons_cdr(*args));
                    pop_root_n(2);
                    continue;
                } else if (!strcmp(prim_name[f->index], "apply")) {
                    ASSERT(list_length(*args) >= 2);
                    func = S_APPLY;
                    arg1 = cons_car(*args);
                    arg2 = apply_arg(cons_cdr(*args));
                    pop_root_n(2);
                    continue;
                } else {
                    pop_root_n(2);
                    return prim_f[f->index](*args);
                }
            }
        } else if (func == S_BEGIN) {
            ptr *es = &arg1, *r = &arg2;
            while (!eq(cons_cdr(*es), nil)) {
                eval(cons_car(*es), *r);
                *es = cons_cdr(*es);
            }
            arg1 = cons_car(*es);
            func = S_EVAL;
            pop_root_n(2);
            continue;
        } else {
            ASSERT(false);
        }
    }
}

ptr eval(ptr e, ptr r) { return trampoline(S_EVAL, e, r); }
ptr apply(ptr f, ptr args) { return trampoline(S_APPLY, f, args); }
ptr begin(ptr es, ptr r) { return trampoline(S_BEGIN, es, r); }
