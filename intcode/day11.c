#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

enum colour {
    C_BLACK = 0,
    C_WHITE = 1
};

/* store coords as an associative linked list */
struct grid {
    int x;
    int y;
    enum colour value;
    struct grid *next;
};

struct robot {
    int x;
    int y;
    enum {
        DIR_UP    = 0,
        DIR_RIGHT = 1,
        DIR_DOWN  = 2,
        DIR_LEFT  = 3
    } facing;
};

enum colour lookup(struct grid *g, int x, int y) {
    /* Panels are initially black */
    if (!g) return C_BLACK;
    if (g->x == x && g->y == y) return g->value;
    return lookup(g->next, x, y);
}

int contains_coord(struct grid *g, int x, int y) {
    if (!g) return 0;
    if (g->x == x && g->y == y) return 1;
    return contains_coord(g->next, x, y);
}

void update_grid(struct grid **g, int x, int y, enum colour value) {
    struct grid *new_cell = calloc(1, sizeof(struct grid));
    new_cell->x = x;
    new_cell->y = y;
    new_cell->value = value;
    new_cell->next = *g;
    *g = new_cell;
    return;
}

void free_grid(struct grid *g) {
    if (g) {
        free_grid(g->next);
        free(g);
    }
    return;
}

void turn_and_move_robot(struct robot *r, int turning_right) {
    r->facing = (r->facing + (turning_right ? 1 : -1)) % 4;
    switch (r->facing) {
    case DIR_UP:
        r->y--; return;
    case DIR_RIGHT:
        r->x++; return;
    case DIR_DOWN:
        r->y++; return;
    case DIR_LEFT:
        r->x--; return;
    }
}

int main(void) {
    num_t file_buffer[8192];
    size_t n;

    n = load_file("input11.txt", file_buffer);
    /* Part 1 */
    struct intcode_vm *vm = vm_from_buffer(file_buffer, n);
    struct grid *g = (void *) 0;
    struct robot rob = {0, 0, DIR_UP};
    size_t locations_visited = 0;
    while (!(vm->status & F_HALTED)) {
        if (!contains_coord(g, rob.x, rob.y))
            locations_visited++;
        push_input(vm, lookup(g, rob.x, rob.y));
        run_vm(vm, 0);
        update_grid(&g, rob.x, rob.y, pop_output(vm));
        turn_and_move_robot(&rob, pop_output(vm));
    }
    printf("%lu\n", locations_visited);
    free_vm(vm);
    free_grid(g);
    /* Part 2 */
    vm = vm_from_buffer(file_buffer, n);
    g = (void *) 0;
    rob = (struct robot) {0, 0, DIR_UP};
    update_grid(&g, 0, 0, C_WHITE);
    while (!(vm->status & F_HALTED)) {
        push_input(vm, lookup(g, rob.x, rob.y));
        run_vm(vm, 0);
        update_grid(&g, rob.x, rob.y, pop_output(vm));
        turn_and_move_robot(&rob, pop_output(vm));
    }
    int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    for (struct grid *tmp = g; tmp; tmp = tmp->next) {
        if (tmp->value == C_WHITE && tmp->x < xmin) xmin = tmp->x;
        if (tmp->value == C_WHITE && tmp->x > xmax) xmax = tmp->x;
        if (tmp->value == C_WHITE && tmp->y < ymin) ymin = tmp->y;
        if (tmp->value == C_WHITE && tmp->y > ymax) ymax = tmp->y;
    }
    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            printf("%s", lookup(g, x, y) == C_WHITE ? "@@" : "  ");
        }
        printf("\n");
    }
    free_vm(vm);
    free_grid(g);
    return 0;
}
