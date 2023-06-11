#include "reader.h"

#include <ctype.h>
#include <errno.h>

#include "gc.h"
#include "util.h"

#define BUFFER_SIZE 128

bool delim_p(int c) { return isspace(c) || c == '(' || c == ')'; }

ptr read_cdr(ptr p) {
    ASSERT(p.type == T_INPUT_PORT);
    int c = fgetc(p.port);
    while (isspace(c)) c = fgetc(p.port);
    if (c == EOF) ASSERT(false);  // unexpected EOF
    if (c == '.') {
        ptr cdr = read(p);
        do {
            c = fgetc(p.port);
        } while (isspace(c));
        if (c != ')') ASSERT(false);
        return cdr;
    }
    if (c == ')') return nil;
    ungetc(c, p.port);
    ptr car = nil, cdr = nil;
    push_root(&car);
    push_root(&cdr);
    car = read(p);
    cdr = read_cdr(p);
    ptr l = cons(&car, &cdr);
    pop_root();
    pop_root();
    return l;
}

ptr read(ptr p) {
    ASSERT(p.type == T_INPUT_PORT);
    int c = fgetc(p.port);
    while (isspace(c)) c = fgetc(p.port);
    if (c == EOF) return eof;

    if (c == '(') return read_cdr(p);
    if (c == '#') {
        c = fgetc(p.port);
        if (c == '\\') {
            static char buf[BUFFER_SIZE];
            int pt = 0;
            buf[pt++] = fgetc(p.port);
            while (!delim_p(c = fgetc(p.port))) {
                if (pt >= BUFFER_SIZE - 1) ASSERT(false);
                buf[pt++] = c;
            }
            ungetc(c, p.port);
            buf[pt++] = 0;
            if (!strcmp(buf, "space")) {
                return make_char(' ');
            } else if (!strcmp(buf, "newline")) {
                return make_char('\n');
            } else {
                return make_char(buf[0]);
            }
        } else if (c == '<') {
            ASSERT(false);  // unreadable object
        } else if (c == '(') {
            ptr l = nil;
            push_root(&l);
            l = read_cdr(p);
            ptr v = nil;
            push_root(&v);
            v = cons(&vector, &l);
            pop_root();
            pop_root();
            return v;
        } else if (c == 't') {
            return make_bool(true);
        } else if (c == 'f') {
            return make_bool(false);
        }
        ASSERT(false);  // unknown object
    } else if (c == '.') {
        ASSERT(false);  // unexpected dot
    } else if (c == ';') {
        while (c != '\n' && c != EOF) c = fgetc(p.port);
        if (c != EOF) ungetc(c, p.port);
        return read(p);
    } else if (c == ')') {
        ASSERT(false);
    } else if (c == '\'') {
        ptr text = nil;
        push_root(&text);
        text = read(p);
        ptr text_l = cons(&text, &nil);
        push_root(&text_l);
        ptr quotation = cons(&quote, &text_l);
        pop_root();
        pop_root();
        return quotation;
    } else if (c == '`') {
        ptr text = nil;
        push_root(&text);
        text = read(p);
        ptr text_l = cons(&text, &nil);
        push_root(&text_l);
        ptr quotation = cons(&quasiquote, &text_l);
        pop_root();
        pop_root();
        return quotation;
    } else if (c == ',') {
        c = fgetc(p.port);
        if (c != '@') {
            ungetc(c, p.port);
            ptr text = nil;
            push_root(&text);
            text = read(p);
            ptr text_l = cons(&text, &nil);
            push_root(&text_l);
            ptr quotation = cons(&unquote, &text_l);
            pop_root();
            pop_root();
            return quotation;
        }
        ptr text = nil;
        push_root(&text);
        text = read(p);
        ptr text_l = cons(&text, &nil);
        push_root(&text_l);
        ptr quotation = cons(&unquote_splice, &text_l);
        pop_root();
        pop_root();
        return quotation;
    } else if (c == '\"') {
        static char buf[BUFFER_SIZE];
        int pt = 0;
        while (true) {
            c = fgetc(p.port);
            if (c == '\"') break;
            if (c == '\\') {
                c = fgetc(p.port);
                if (c == 'n') {
                    buf[pt++] = '\n';
                } else {
                    buf[pt++] = c;
                }
            } else {
                buf[pt++] = c;
            }
        }
        ptr v = make_vector(pt);
        for (ll i = 0; i < pt; i++) vector_set(v, i, make_char(buf[i]));
        return v;
    } else {
        ungetc(c, p.port);
        static char buf[BUFFER_SIZE];
        int pt = 0;
        while (!delim_p(c = fgetc(p.port))) {
            if (pt >= BUFFER_SIZE - 1) ASSERT(false);
            buf[pt++] = c;
        }
        buf[pt++] = 0;
        ungetc(c, p.port);
        char *e;
        ld val = strtold(buf, &e);
        if (*e != '\0' || errno) return INTERN(buf);
        return make_number(val);
    }
}
