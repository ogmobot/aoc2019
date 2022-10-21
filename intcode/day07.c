#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

#define VM_COUNT 5
#define NUM_PERMS 120

#define swap(a, b) do { \
(a) = (a) ^ (b); \
(b) = (b) ^ (a); \
(a) = (a) ^ (b); \
} while (0);

struct permutation {
    num_t a[VM_COUNT];
};

int factorial(int n) {
    int res = 1;
    while (n > 0)
        res *= n--;
    return res;
}

void write_permutation(struct permutation p, struct permutation *buffer) {
    for (size_t i = 0; i < VM_COUNT; i++)
        buffer->a[i] = p.a[i];
    return;
}

void heaps_algorithm(struct permutation p, struct permutation *buffer) {
    /* Writes each permutation to *buffer, starting with initial */
    /* Thanks to Wikipedia */
    size_t c[VM_COUNT];
    for (size_t i = 0; i < VM_COUNT; i++)
        c[i] = 0;

    write_permutation(p, buffer++);

    size_t i = 1;
    while (i < VM_COUNT) {
        if (c[i] < i) {
            if (i % 2 == 0) {
                swap(p.a[0], p.a[i]);
            } else {
                swap(p.a[c[i]], p.a[i]);
            }
            write_permutation(p, buffer++);
            c[i]++;
            i = 1;
        } else {
            c[i] = 0;
            i++;
        }
    }
    return;
}

int main(void) {
    struct intcode_vm *vms[VM_COUNT];
    num_t file_buffer[8192];
    size_t n;

    const int num_perms = factorial(VM_COUNT);
    struct permutation *perms = calloc(num_perms, sizeof(struct permutation));

    n = load_file("input07.txt", file_buffer);
    /* Part 1 */
    heaps_algorithm((struct permutation) {.a={0, 1, 2, 3, 4}}, perms);
    num_t highest_output = 0;
    for (size_t i = 0; i < NUM_PERMS; i++) {
        for (size_t j = 0; j < VM_COUNT; j++) {
            vms[j] = vm_from_buffer(file_buffer, n);
            push_input(vms[j], perms[i].a[j]);
        }
        push_input(vms[0], 0);
        for (size_t j = 0; j < VM_COUNT; j++) {
            run_vm(vms[j], 0);
            if (j == VM_COUNT - 1) {
                num_t res = pop_output(vms[j]);
                if (res > highest_output)
                    highest_output = res;
            } else {
                push_input(vms[j+1], pop_output(vms[j]));
            }
        }
        for (size_t j = 0; j < VM_COUNT; j++)
            free_vm(vms[j]);
    }
    printf(NUM_T_FORMAT "\n", highest_output);
    /* Part 2 */
    heaps_algorithm((struct permutation) {.a={5, 6, 7, 8, 9}}, perms);
    highest_output = 0;
    for (size_t i = 0; i < NUM_PERMS; i++) {
        for (size_t j = 0; j < VM_COUNT; j++) {
            vms[j] = vm_from_buffer(file_buffer, n);
            push_input(vms[j], perms[i].a[j]);
        }
        push_input(vms[0], 0);
        size_t j = 0;
        num_t last_output = 0;
        while (!(vms[j]->status & F_HALTED)) {
            run_vm(vms[j], 1);
            if (vms[j]->status & F_PUSHED_OUTPUT) {
                num_t res = pop_output(vms[j]);
                push_input(vms[(j + 1) % VM_COUNT], res);
                if (j == VM_COUNT - 1)
                    last_output = res;
            }
            j = (j + 1) % VM_COUNT;
        }
        if (last_output > highest_output)
            highest_output = last_output;
        for (size_t j = 0; j < VM_COUNT; j++)
            free_vm(vms[j]);
    }
    printf(NUM_T_FORMAT "\n", highest_output);
    free(perms);
    return 0;
}
