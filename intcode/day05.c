#include <stdio.h>
#include "intcode.h"

int main(void) {
    struct intcode_vm *vm;
    num_t file_buffer[8192];
    size_t n;

    n = load_file("input05.txt", file_buffer);
    /* Part 1 */
    vm = vm_from_buffer(file_buffer, n);
    push_input(vm, 1);
    run_vm(vm, 0);
    num_t diag;
    do {
        diag = pop_output(vm);
    } while (!diag);
    printf(NUM_T_FORMAT "\n", diag);
    free_vm(vm);
    /* Part 2 */
    vm = vm_from_buffer(file_buffer, n);
    push_input(vm, 5);
    run_vm(vm, 0);
    printf(NUM_T_FORMAT "\n", pop_output(vm));
    free_vm(vm);

    return 0;
}
