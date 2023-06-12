#include "gc.h"
#include "obarray.h"
#include "reader.h"
#include "printer.h"
#include "eval.h"
#include "primitives.h"

int main() {
    obarray_init();
    gc_init();
    ptr env = make_initial_environment();
    push_root(&env);

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
        pop_root();
        // printf("%lld\n", root_sp);
    }
    printf("\n");
    return 0;
}
