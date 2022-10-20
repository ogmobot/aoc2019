#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

enum tile_s {
    S_EMPTY  = 0,
    S_WALL   = 1,
    S_BLOCK  = 2,
    S_PADDLE = 3,
    S_BALL   = 4
};

int main(void) {
    num_t file_buffer[8192];
    size_t n;

    n = load_file("input13.txt", file_buffer);

    struct intcode_vm *vm = vm_from_buffer(file_buffer, n);
    set_memory_direct(vm, 0, 2); /* freeplay mode */
    int score = 0;
    int block_count = 0;
    int paddle_location = 0;
    int ball_location = 0;
    while (1) {
        run_vm(vm, 1);
        if (vm->status & F_HALTED)
            break;
        if (vm->status & F_REQUIRE_INPUT) {
            /* Part 1 */
            if (block_count != -1) {
                printf("%d\n", block_count);
                block_count = -1;
            }
            /* Part 2 */
            push_input(vm, paddle_location == ball_location ? 0
                        : (paddle_location < ball_location ? 1 : -1));
        } else {
            run_vm(vm, 1);
            run_vm(vm, 1);
            int x = (int) pop_output(vm);
            pop_output(vm); /* discard y */
            int val = (int) pop_output(vm);
            if (x == -1) {
                score = val;
                continue;
            }
            if (val == S_PADDLE)
                paddle_location = x;
            if (val == S_BALL)
                ball_location = x;
            if (val == S_BLOCK && block_count != -1)
                block_count++;
        }
    }
    printf("%d\n", score);
    free_vm(vm);
    return 0;
}
