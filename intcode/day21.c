#include <stdio.h>
#include "intcode.h"

int main(void) {
    num_t file_buffer[8192];
    size_t n;

    n = load_file("input21.txt", file_buffer);
    struct intcode_vm *vm = vm_from_buffer(file_buffer, n);
    run_vm_interactive(vm, I_ASCII_SILENT);
    free_vm(vm);
    return 0;
}
