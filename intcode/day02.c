#include <stdio.h>
#include "intcode.h"

int main(void) {
    struct intcode_vm *vm;
    num_t file_buffer[8192];
    size_t n;

    n = load_file("input02.txt", file_buffer);
    /* Part 1 */
    vm = vm_from_buffer(file_buffer, n);
    set_memory_direct(vm, 1, 12);
    set_memory_direct(vm, 2,  2);
    run_vm(vm, 0);
    printf(NUM_T_FORMAT "\n", get_memory_direct(vm, 0));
    free_vm(vm);
    /* Part 2 */
    const num_t target = 19690720;
    for (num_t noun = 0; noun < 99; noun++) {
        for (num_t verb = 0; verb < 99; verb++) {
            vm = vm_from_buffer(file_buffer, n);
            set_memory_direct(vm, 1, noun);
            set_memory_direct(vm, 2, verb);
            run_vm(vm, 0);
            num_t res = get_memory_direct(vm, 0);
            free_vm(vm);
            if (res == target) {
                printf("%2d%2d\n", (int) noun, (int) verb);
                goto finish;
            }
        }
    }
finish:
    return 0;
}
