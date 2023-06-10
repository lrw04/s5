#ifndef LISP_H
#define LISP_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int64_t ll;
typedef long double ld;
typedef uint8_t byte;

#define HASHTABLE_P 19260817

enum val_type {
    T_NUMBER,
    T_SYMBOL,
    T_CHARACTER,
    T_BOOL,
    T_CONS,
    T_VECTOR,
    T_HASHTABLE,
    T_PROCEDURE,
    T_PRIMITIVE,
    T_MACRO,
    T_INPUT_PORT,
    T_OUTPUT_PORT,
    T_ENVIRONMENT,
    T_EOF,
    T_NIL,
    T_UNBOUND,
};

typedef struct ptr {
    int type;
    union {
        // number
        ld number;
        // symbol, cons, hashtable, procedure, primitive, macro, environment
        ll index;
        // character
        char character;
        // bool
        bool boolean;
        // vector
        struct {
            ll start, size;
        };
        // port
        FILE *port;
    };
} ptr;

ptr make_number(ld number);
ptr intern(const char *s, ll n);
#define INTERN(s) (intern((s), strlen(s)))
ptr make_char(char character);
ptr make_bool(bool boolean);
ptr make_input_port(FILE *port);
ptr make_output_port(FILE *port);
ptr make_primitive(ll index);
ptr make_eof();
ptr make_nil();
ptr make_unbound();

#define eof (make_eof())
#define nil (make_nil())
#define unbound (make_unbound())

bool eq(ptr a, ptr b);

typedef struct obj {
    bool moved;
    union {
        ptr p;
        ll forward;
    };
} obj;

extern obj *memory;

ptr cons_car(ptr p);
ptr cons_cdr(ptr p);
void cons_setcar(ptr p, ptr car);
void cons_setcdr(ptr p, ptr cdr);

ptr hashtable_at(ptr p, int h);
ptr get_hash(ptr p, ptr k);
void set_hash(ptr p, ptr k, ptr v);

ptr proc_formals(ptr p);
ptr proc_body(ptr p);
ptr proc_env(ptr p);

ptr env_car(ptr p);
ptr env_cdr(ptr p);

ptr vector_ref(ptr p, ll i);
void vector_set(ptr p, ll i, ptr v);
bool vector_stringp(ptr p);

ll list_length(ptr l);
ptr list_to_vector(ptr l);
bool list_p(ptr l);

int next_hash(int prev, byte cur);
int hash(ptr p);

#endif
