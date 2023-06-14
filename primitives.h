#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "lisp.h"

//                        args
typedef ptr (*primitive_t)(ptr);

ptr p_eq(ptr a);
ptr p_number(ptr a);
ptr p_integer(ptr a);
ptr p_not(ptr a);
ptr p_boolean(ptr a);
ptr p_pair(ptr a);
ptr p_cons(ptr a);
ptr p_car(ptr a);
ptr p_cdr(ptr a);
ptr p_set_car(ptr a);
ptr p_set_cdr(ptr a);
ptr p_null(ptr a);
ptr p_symbol(ptr a);
ptr p_symbol_to_string(ptr a);
ptr p_string_to_symbol(ptr a);
ptr p_char(ptr a);
ptr p_char_to_integer(ptr a);
ptr p_integer_to_char(ptr a);
ptr p_vector(ptr a);
ptr p_make_vector(ptr a);
ptr p_vector_length(ptr a);
ptr p_vector_ref(ptr a);
ptr p_vector_set(ptr a);
ptr p_gensym(ptr a);
ptr p_interaction_environment(ptr a);
ptr p_current_input_port(ptr a);
ptr p_current_output_port(ptr a);

const static char *prim_name[] = {"eq?",
                                  "number?",
                                  "integer?",
                                  "not",
                                  "boolean?",
                                  "pair?",
                                  "cons",
                                  "car",
                                  "cdr",
                                  "set-car!",
                                  "set-cdr!",
                                  "null?",
                                  "symbol?",
                                  "symbol->string",
                                  "string->symbol",
                                  "char?",
                                  "char->integer",
                                  "integer->char",
                                  "vector?",
                                  "make-vector",
                                  "vector-length",
                                  "vector-ref",
                                  "vector-set!",
                                  "gensym",
                                  "eval",
                                  "apply",
                                  "interaction-environment",
                                  "current-input-port",
                                  "current-output-port"};
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
                                     p_char,
                                     p_char_to_integer,
                                     p_integer_to_char,
                                     p_vector,
                                     p_make_vector,
                                     p_vector_length,
                                     p_vector_ref,
                                     p_vector_set,
                                     p_gensym,
                                     NULL,
                                     NULL,
                                     p_interaction_environment,
                                     p_current_input_port,
                                     p_current_output_port};

ptr make_initial_environment();

#endif
