#include "printer.h"

#include "obarray.h"
#include "util.h"

void print_cdr(ptr p, ptr port) {
    if (eq(p, nil)) {
        fprintf(port.port, ")");
        return;
    }
    print(cons_car(p), port);
    if (cons_cdr(p).type == T_CONS) {
        fprintf(port.port, " ");
        print_cdr(cons_cdr(p), port);
    } else {
        if (eq(cons_cdr(p), nil)) {
            fprintf(port.port, ")");
            return;
        }
        fprintf(port.port, " . ");
        print(cons_cdr(p), port);
        fprintf(port.port, ")");
    }
}

void print(ptr p, ptr port) {
    ASSERT(port.type == T_OUTPUT_PORT);
    int n;
    switch (p.type) {
        case T_NUMBER:
            fprintf(port.port, "%Lf", p.number);
            break;
        case T_SYMBOL:
            n = obarray[p.index].depth;
            char *s = malloc((n + 1) * sizeof(char));
            for (ll u = p.index, i = n - 1; u; u = obarray[u].father, i--) {
                s[i] = obarray[u].path;
            }
            fprintf(port.port, "%s", s);
            free(s);
            break;
        case T_CHARACTER:
            if (p.character == ' ') {
                fprintf(port.port, "#\\space");
            } else if (p.character == '\n') {
                fprintf(port.port, "#\\newline");
            } else {
                fprintf(port.port, "#\\%c", p.character);
            }
            break;
        case T_BOOL:
            fprintf(port.port, p.boolean ? "#t" : "#f");
            break;
        case T_CONS:
        case T_NIL:
            fprintf(port.port, "(");
            print_cdr(p, port);
            break;
        case T_VECTOR:
            if (vector_stringp(p)) {
                fprintf(port.port, "\"");
                for (ll i = 0; i < p.size; i++) {
                    char c = vector_ref(p, i).character;
                    if (c == '\"') {
                        fprintf(port.port, "\\\"");
                    } else if (c == '\n') {
                        fprintf(port.port, "\\n");
                    } else {
                        fprintf(port.port, "%c", c);
                    }
                }
                fprintf(port.port, "\"");
            } else {
                fprintf(port.port, "#(");
                for (ll i = 0; i < p.size; i++) {
                    if (i) fprintf(port.port, " ");
                    print(vector_ref(p, i), port);
                }
                fprintf(port.port, ")");
            }
            break;
        case T_HASHTABLE:
            fprintf(port.port, "#<hashtable>");
            break;
        case T_PROCEDURE:
            fprintf(port.port, "#<procedure>");
            break;
        case T_PRIMITIVE:
            fprintf(port.port, "#<primitive>");
            break;
        case T_MACRO:
            fprintf(port.port, "#<macro>");
            break;
        case T_INPUT_PORT:
            fprintf(port.port, "#<input-port>");
            break;
        case T_OUTPUT_PORT:
            fprintf(port.port, "#<output-port>");
            break;
        case T_ENVIRONMENT:
            fprintf(port.port, "#<environment>");
            break;
        case T_EOF:
            fprintf(port.port, "#<eof>");
            break;
        case T_UNBOUND:
            fprintf(port.port, "#<unbound>");
            break;
        default:
            ASSERT(false);
    }
}
