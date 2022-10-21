#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

#define HT_LENGTH 1024
#define MAX_PATH_LENGTH 2048

enum direction {
    END_PATH = 0,
    D_NORTH  = 1,
    D_SOUTH  = 2,
    D_WEST   = 3,
    D_EAST   = 4
};

struct coord {
    struct coord *next; /* for keeping track of seen coords */
    int x, y;
};

struct ht_node {
    struct ht_node *next;
    struct ht_node *path; /* path to caller of find_path */
    struct coord key;
    char symbol;
    size_t distance; /* distance to last caller of populate_distances */
};

struct maze {
    struct ht_node *grid[HT_LENGTH];
    struct coord droid;
    struct coord oxygen;
};

struct explore_node {
    struct ht_node *grid_location;
    struct explore_node *next;
};

/* Hash table functions */

size_t coord_hash(struct coord xy) {
    return (size_t) (xy.x ^ xy.y) % HT_LENGTH;
}

void ht_insert(struct ht_node **grid, struct coord key, char symbol) {
    size_t hash = coord_hash(key);
    struct ht_node *new_node = calloc(1, sizeof(struct ht_node));
    new_node->key = key;
    new_node->symbol = symbol;
    new_node->next = grid[hash];
    grid[hash] = new_node;
    return;
}

struct ht_node *ht_lookup(struct ht_node **grid, struct coord key) {
    size_t hash = coord_hash(key);
    for (struct ht_node *tmp = grid[hash]; tmp; tmp = tmp->next)
        if (key.x == tmp->key.x && key.y == tmp->key.y)
            return tmp;
    return (void *) 0;
}

/* Exploration queue functions */

void push_queue(struct explore_node **q, struct ht_node *gl) {
    struct explore_node *new_node = calloc(1, sizeof(struct coord));
    new_node->grid_location = gl;
    if (!(*q)) {
        new_node->next = new_node;
        *q = new_node;
    } else {
        new_node->next = (*q)->next;
        (*q)->next = new_node;
        *q = new_node;
    }
    return;
}

struct ht_node *pop_queue(struct explore_node **q) {
    struct explore_node *head = (*q)->next;
    if (head->next == head) {
        *q = (void *) 0;
    } else {
        (*q)->next = head->next;
    }
    struct ht_node *tmp = head->grid_location;
    free(head);
    return tmp;
}

void push_stack(struct coord **cs, struct coord c) {
    struct coord *new_node = calloc(1, sizeof(struct coord));
    new_node->next = *cs;
    *cs = new_node;
    return;
}

void free_stack(struct coord *cs) {
    struct coord *tmp = cs;
    while (cs) {
        free(tmp);
        tmp = cs;
        cs = cs->next;
    }
    return;
}

int stack_contains(struct coord *cs, struct coord c) {
    while (cs) {
        if (cs->x == c.x && cs->y == c.y)
            return 1;
        cs = cs->next;
    }
    return 0;
}

/* Maze solving functions */

/* Robot manipulation functions */

void find_path(
    struct maze *m,
    struct coord start,
    struct coord end,
    enum direction *buffer
) {
    struct explore_node *xq = (void *) 0;
    struct ht_node *target = ht_lookup(m->grid, start);
    target->path = (void *) 0;
    target->distance = 0;
    push_queue(&xq, target);
    target = (void *) 0;
    struct coord *seen;
    while (xq) {
        struct ht_node *current = pop_queue(&xq);
        if (stack_contains(seen, (struct coord) { .x = current->key.x,
                                                  .y = current->key.y}))
            continue;
        if (current->key.x == end.x && current->key.y == end.y) {
            /* Found path! */
            while (xq)
                pop_queue(&xq);
            free_stack(seen);
            target = current;
            break;
        }
        push_stack(&seen, current->key);
        struct coord neighbours[4] = {
            (struct coord) {.x = current->key.x + 1, .y = current->key.y},
            (struct coord) {.x = current->key.x - 1, .y = current->key.y},
            (struct coord) {.x = current->key.x, .y = current->key.y + 1},
            (struct coord) {.x = current->key.x, .y = current->key.y - 1}
        };
        for (size_t i = 0; i < 4; i++) {
            struct ht_node *tmp = ht_lookup(m->grid, neighbours[i]);
            if (!tmp)
                continue;
            if (stack_contains(seen, neighbours[i]))
                continue;
            tmp->path = current;
            tmp->distance = current->distance + 1;
            push_queue(&xq, tmp);
        }
    }
    if (!target) {
        *buffer = END_PATH;
    } else {
        buffer[target->distance] = END_PATH;
        while (target->distance > 0) {
            enum direction to_write;
            target = target->path;
        }
    }
    return;
}

void walk_path(struct intcode_vm *vm, enum direction *path) {
    while (*path != END_PATH) {
        push_input(vm, (num_t) *path);
        run_vm(vm, 0);
        num_t tmp = pop_output(vm);
        if (!tmp) { /* Damn! We hit a wall! */
            printf("Oh no! Something went horribly wrong!\n");
            return;
        }
        path++;
    }
    return;
}

void explore_maze(struct intcode_vm *vm, struct maze *m) {
    return;
}

/* Main function */

int main(void) {
    /*
    num_t file_buffer[8192];
    size_t n = load_file("input15.txt", file_buffer);
    struct intcode_vm *vm = vm_from_buffer(file_buffer, n);
    */

    struct maze m = {0};
    /*
    explore_maze(vm, &m);
    populate_distances(&m, m.droid);
    */
    ht_insert(m.grid, (struct coord) {0, 0}, '@');
    printf("%c\n", ht_lookup(m.grid, (struct coord) {0, 0})->symbol);
    return 0;
}
