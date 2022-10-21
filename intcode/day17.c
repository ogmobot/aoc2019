#include <stdio.h>
#include "intcode.h"

#define LINE_BUFFER_LENGTH 256
#define S_PATH '#'

int main(void) {
    num_t file_buffer[8192];
    char line_buffer[3][LINE_BUFFER_LENGTH]; /* upper, middle, lower */
    for (size_t i = 0; i < LINE_BUFFER_LENGTH; i++) {
        line_buffer[0][i] = '\0';
        line_buffer[1][i] = '\0';
        line_buffer[2][i] = '\0';
    }
    size_t n;

    n = load_file("input17.txt", file_buffer);
    struct intcode_vm *vm;
    /* Part 1 */
    vm = vm_from_buffer(file_buffer, n);
    int row_index = -1, col_index = 0, sum = 0;
    while (1) {
        run_vm(vm, 1);
        if (vm->status & F_HALTED)
            break;
        char c = (char) pop_output(vm);
        if (c == '\n') {
            for (size_t i = 1; i < LINE_BUFFER_LENGTH - 1; i++) {
                if (line_buffer[0][i]        == S_PATH
                    && line_buffer[1][i - 1] == S_PATH
                    && line_buffer[1][i]     == S_PATH
                    && line_buffer[1][i + 1] == S_PATH
                    && line_buffer[2][i]     == S_PATH
                )
                    sum += row_index * i;
            }
            col_index = 0;
            row_index++;
            for (size_t i = 0; i < LINE_BUFFER_LENGTH; i++) {
                line_buffer[0][i] = line_buffer[1][i];
                line_buffer[1][i] = line_buffer[2][i];
            }
        } else {
            line_buffer[2][col_index++] = c;
        }
    }
    printf("%d\n", sum);
    free_vm(vm);
    /* Part 2 */
    vm = vm_from_buffer(file_buffer, n);
    set_memory_direct(vm, 0, 2);
    run_vm_interactive(vm, I_ASCII_SILENT);
    free_vm(vm);
    return 0;
}
