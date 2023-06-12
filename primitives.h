#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "lisp.h"

//                        args  env
typedef ptr (*primitive_t)(ptr, ptr);

ptr p_eq(ptr a, ptr r);
ptr p_number(ptr a, ptr r);
ptr p_integer(ptr a, ptr r);
ptr p_not(ptr a, ptr r);
ptr p_boolean(ptr a, ptr r);
ptr p_pair(ptr a, ptr r);
ptr p_cons(ptr a, ptr r);
ptr p_car(ptr a, ptr r);
ptr p_cdr(ptr a, ptr r);
ptr p_set_car(ptr a, ptr r);
ptr p_set_cdr(ptr a, ptr r);
ptr p_null(ptr a, ptr r);
ptr p_symbol(ptr a, ptr r);
ptr p_symbol_to_string(ptr a, ptr r);
ptr p_string_to_symbol(ptr a, ptr r);
ptr p_char(ptr a, ptr r);

const static char *prim_name[] = {
    "eq?",      "number?",        "integer?",       "not",
    "boolean?", "pair?",          "cons",           "car",
    "cdr",      "set-car!",       "set-cdr!",       "null?",
    "symbol?",  "symbol->string", "string->symbol", "char?"};
const static primitive_t prim_f[] = {p_eq,
                                     p_number,
                                     p_integer,
                                     p_not,
                                     p_boolean,
                                     p_pair,
                                     p_cons,
                                     p_car,
                                     p_cdr,
                                     p_set_car,
                                     p_set_cdr,
                                     p_null,
                                     p_symbol,
                                     p_symbol_to_string,
                                     p_string_to_symbol,
                                     p_char};

ptr make_initial_environment();

#endif
