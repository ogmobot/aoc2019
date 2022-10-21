#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

#define HT_LENGTH 1024

enum direction {
    END_PATH = 0,
    D_NORTH  = 1,
    D_SOUTH  = 2,
    D_WEST   = 3,
    D_EAST   = 4
};

struct coord {
    int x, y;
};

struct ht_node {
    struct coord key;
    char symbol;
    struct ht_node *next;
};

struct maze {
    struct ht_node *grid[HT_LENGTH];
    struct coord droid;
    struct coord oxygen;
};

struct explore_node {
    struct coord position;
    struct explore_node *next;
    size_t distance;
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

void push_queue(struct explore_node **q, struct coord p, size_t d) {
    struct explore_node *new_node = calloc(1, sizeof(struct coord));
    new_node->position = p;
    new_node->distance = d;
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

struct explore_node *pop_queue(struct explore_node **q) {
    struct explore_node *head = (*q)->next;
    if (head->next == head) {
        *q = (void *) 0;
    } else {
        (*q)->next = head->next;
    }
    /* Caller must free this */
    return head;
}

/* Maze solving functions */

/* Robot manipulation functions */

void walk_path(struct intcode_vm *vm, enum direction *path) {
    while (*path != END_PATH) {
        push_input(vm, (num_t) *path);
        vm_run(vm, 0);
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
