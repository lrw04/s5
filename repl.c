#include "gc.h"
#include "obarray.h"
#include "reader.h"
#include "printer.h"
#include "eval.h"

// TODO: evaluator

int main() {
    obarray_init();
    gc_init();

    while (true) {
        printf("> ");
        fflush(stdout);
        ptr p = nil;
        push_root(&p);
        p = read(make_input_port(stdin));
        if (eq(p, eof)) break;
        print(p, make_output_port(stdout));
        printf("\n");
        pop_root();
    }
    printf("\n");
    return 0;
}
