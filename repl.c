#include "repl.h"

#include "eval.h"
#include "gc.h"
#include "obarray.h"
#include "primitives.h"
#include "printer.h"
#include "reader.h"
#include "util.h"

ptr global_env, cip, cop;

// s5 stdlib.scm program.scm <args>
// s5 stdlib.scm - <args>
int main(int argc, char **argv) {
    ASSERT(argc >= 3);
    char *stdlib = argv[1], *prog = argv[2];
    bool repl = !strcmp(prog, "-");
    obarray_init();
    gc_init();
    global_env = nil;
    cip = make_input_port(stdin);
    cop = make_output_port(stdout);
    push_root(&global_env);
    global_env = make_initial_environment();

    FILE *std = fopen(stdlib, "r");
    ASSERT(std);
    while (true) {
        ptr p = nil;
        push_root(&p);
        p = read(make_input_port(std));
        if (eq(p, eof)) {
            pop_root();
            break;
        }
        eval(p, global_env);
        // fprintf(stderr, "%lld %lld\n", root_sp, memory_size);
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
            p = eval(p, global_env);
            print(p, make_output_port(stdout));
            printf("\n");
            // fprintf(stderr, "%lld %lld\n", root_sp, memory_size);
            pop_root();
        }
        printf("\n");
    } else {
        FILE *p = fopen(prog, "r");
        ASSERT(p);
        while (true) {
            ptr e = nil;
            push_root(&e);
            e = read(make_input_port(p));
            if (eq(e, eof)) break;
            eval(e, global_env);
            pop_root();
        }
        fclose(p);
    }
    return 0;
}
