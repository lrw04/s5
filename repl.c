#include "eval.h"
#include "gc.h"
#include "obarray.h"
#include "primitives.h"
#include "printer.h"
#include "reader.h"
#include "util.h"

// s5 stdlib.scm program.scm <args>
// s5 stdlib.scm - <args>
int main(int argc, char **argv) {
    ASSERT(argc >= 3);
    char *stdlib = argv[1], *prog = argv[2];
    bool repl = !strcmp(prog, "-");
    obarray_init();
    gc_init();
    ptr env = nil;
    push_root(&env);
    env = make_initial_environment();

    FILE *std = fopen(stdlib, "r");
    while (true) {
        ptr p = nil;
        push_root(&p);
        p = read(make_input_port(std));
        if (eq(p, eof)) {
            pop_root();
            break;
        }
        eval(p, env);
        pop_root();
    }
    fclose(std);

    if (repl) {
        while (true) {
            printf("> ");
            fflush(stdout);
            ptr p = nil;
            push_root(&p);
            p = read(make_input_port(stdin));
            if (eq(p, eof)) break;
            p = eval(p, env);
            print(p, make_output_port(stdout));
            printf("\n");
            // printf("%lld %lld\n", root_sp, memory_size);
            pop_root();
        }
        printf("\n");
    } else {
        FILE *p = fopen(prog, "r");
        while (true) {
            ptr e = nil;
            push_root(&e);
            e = read(make_input_port(p));
            if (eq(e, eof)) break;
            eval(e, env);
            pop_root();
        }
        fclose(p);
    }
    return 0;
}
