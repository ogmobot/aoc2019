#include <stdio.h>
#include "intcode.h"

#define ROM_SIZE 512

struct probe_template {
    num_t data[ROM_SIZE];
    size_t n;
};

int scan(struct probe_template *rom, size_t x, size_t y) {
    struct intcode_vm *vm = vm_from_buffer(rom->data, rom->n);
    push_input(vm, x);
    push_input(vm, y);
    run_vm(vm, 0);
    int res = pop_output(vm);
    free_vm(vm);
    return res;
}

long int find_square(struct probe_template *rom, const size_t square_size) {
    /* Returns top-left x, y as a single value with digits xxxxyyyy */
    /* Trace bottom-left of the square along the beam until top-right fits */
    size_t bottom_y = square_size - 1;
    size_t left_x = 0;
    while (1) {
        while (!(scan(rom, left_x, bottom_y)))
            left_x++;
        const size_t top_y = bottom_y - (square_size - 1);
        const size_t right_x = left_x + (square_size - 1);
        if (scan(rom, right_x, top_y))
            return (10000 * left_x) + top_y;
        bottom_y++;
    }
    return -1;
}

int main(void) {
    struct probe_template rom;
    rom.n = load_file("input19.txt", rom.data);

    /* Part 1 */
    int total = 0;
    for (size_t x = 0; x < 50; x++)
        for (size_t y = 0; y < 50; y++)
            total += scan(&rom, x, y);
    printf("%d\n", total);
    /* Part 2 */
    printf("%ld\n", find_square(&rom, 100));
    return 0;
}
