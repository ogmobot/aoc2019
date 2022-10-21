#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

#define MAX_LOCATIONS 8192

enum direction {
    END_PATH = 0,
    D_NORTH  = 1,
    D_SOUTH  = 2,
    D_WEST   = 3,
    D_EAST   = 4
};

enum response {
    R_WALL   = 0,
    R_FLOOR  = 1,
    R_OXYGEN = 2
};

struct grid_node {
    struct grid_node *neighbours[5]; /* index 0 unused */
    struct grid_node *path; /* path to last caller of populate_distances */
    size_t distance; /* distance to last caller of populate_distances */
    int x;
    int y;
};

struct maze {
    struct grid_node *robot;
    struct grid_node *oxygen;
};

struct explore_node {
    struct grid_node *location;
    struct explore_node *next;
};

struct array_list {
    struct {
        int x;
        int y;
    } position[MAX_LOCATIONS];
    size_t length;
};

/* Exploration queue functions */

void push_queue(struct explore_node **q, struct grid_node *x) {
    struct explore_node *new_node = calloc(1, sizeof(struct explore_node));
    new_node->location = x;
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

struct grid_node *pop_queue(struct explore_node **q) {
    struct explore_node *head = (*q)->next;
    if (head->next == head) {
        *q = (void *) 0;
    } else {
        (*q)->next = head->next;
    }
    struct grid_node *tmp = head->location;
    free(head);
    return tmp;
}

void push_stack(struct explore_node **s, struct grid_node *x) {
    struct explore_node *new_node = calloc(1, sizeof(struct explore_node));
    new_node->location = x;
    new_node->next = *s;
    *s = new_node;
}

struct grid_node *pop_stack(struct explore_node **s) {
    struct explore_node *top = *s;
    *s = top->next;
    struct grid_node *tmp = top->location;
    free(top);
    return tmp;
}

/* Array list functions */

void list_append(struct array_list *l, int x, int y) {
    l->position[l->length].x = x;
    l->position[l->length].y = y;
    (l->length)++;
    return;
}

int list_contains(struct array_list *l, int x, int y) {
    for (size_t i = 0; i < l->length; i++) {
        if (l->position[i].x == x && l->position[i].y == y)
            return 1;
    }
    return 0;
}

/* Maze solving functions */

void write_path(struct grid_node *node, enum direction *buffer) {
    buffer[node->distance] = END_PATH;
    while (node->path) {
        /* pointers not permitted as switch/case values */
        buffer[node->distance - 1] =
            (node->path == node->neighbours[D_NORTH] ? D_SOUTH :
            (node->path == node->neighbours[D_SOUTH] ? D_NORTH :
            (node->path == node->neighbours[D_WEST]  ? D_EAST  :
            (node->path == node->neighbours[D_EAST]  ? D_WEST  :
            /* Damn! The rest of the path doesn't connect! */
            END_PATH))));
        if (buffer[node->distance - 1] == END_PATH)
            printf("Oh no! Path disconnected!\n");
        node = node->path;
    }
    return;
}

size_t find_path(
    struct grid_node *start,
    int target_x,
    int target_y,
    enum direction *buffer
) {
    struct explore_node *xq = (void *) 0;
    struct array_list seen;
    seen.length = 0;
    size_t largest_distance = 0;

    start->path = (void *) 0;
    start->distance = 0;
    push_queue(&xq, start);

    while (xq) {
        struct grid_node *current = pop_queue(&xq);
        if (list_contains(&seen, current->x, current->y))
            continue;
        if (current->distance > largest_distance)
            largest_distance = current->distance;
        if (current->x == target_x && current->y == target_y) {
            if (buffer)
                write_path(current, buffer);
            while (xq)
                pop_queue(&xq);
            return current->distance;
        }
        list_append(&seen, current->x, current->y);
        for (enum direction i = 1; i <= 4; i++) {
            struct grid_node *n = current->neighbours[i];
            if (n && !(list_contains(&seen, n->x, n->y))) {
                n->path = current;
                n->distance = current->distance + 1;
                push_queue(&xq, n);
            }
        }
    }
    /* Exhausted the queue; no path found */
    if (buffer)
        *buffer = END_PATH;
    return largest_distance;
}

/* Robot manipulation functions */

void walk_path(struct intcode_vm *vm, enum direction *path) {
    while (*path != END_PATH) {
        push_input(vm, (num_t) *path);
        run_vm(vm, 0);
        num_t tmp = pop_output(vm);
        if (tmp == R_WALL) { /* Damn! We hit a wall! */
            printf("Oh no! Something went horribly wrong!\n");
            return;
        }
        path++;
    }
    return;
}

void explore_maze(struct intcode_vm *vm, struct maze *m) {
    enum direction path_buffer[MAX_LOCATIONS];
    struct explore_node *xs = (void *) 0;
    struct grid_node *start = calloc(1, sizeof(struct grid_node));
    struct array_list seen;

    seen.length = 0;
    m->robot = start;
    struct grid_node *robot = start;
    push_stack(&xs, start);

    while (xs) {
        struct grid_node *current = pop_stack(&xs);
        if (list_contains(&seen, current->x, current->y))
            continue;
        /*printf("== exploring x=%d, y=%d ==\n", current->x, current->y);*/
        list_append(&seen, current->x, current->y);
        find_path(robot, current->x, current->y, path_buffer);
        /*printf("path buffer %d %d ...\n", path_buffer[0], path_buffer[1]);*/
        walk_path(vm, path_buffer);
        robot = current;

        /* Test moving to each unknown neighbour */
        for (enum direction dir = 1; dir <= 4; dir++) {
            if (robot->neighbours[dir]) {
                /*printf("skipping dir=%d\n", dir);*/
                continue;
            }
            /*printf("attempting to move in dir=%d\n", dir);*/
            push_input(vm, dir);
            run_vm(vm, 0);
            enum response res = pop_output(vm);
            /*printf("response=%d\n", res);*/
            /* If res == R_WALL, robot did not move */
            if (res != R_WALL) {
                struct grid_node *node = calloc(1, sizeof(struct grid_node));
                robot->neighbours[dir] = node;
                switch (dir) {
                case D_NORTH:
                    node->neighbours[D_SOUTH] = robot;
                    node->x = current->x;
                    node->y = current->y - 1;
                    push_input(vm, D_SOUTH);
                    break;
                case D_SOUTH:
                    node->neighbours[D_NORTH] = robot;
                    node->x = current->x;
                    node->y = current->y + 1;
                    push_input(vm, D_NORTH);
                    break;
                case D_WEST:
                    node->neighbours[D_EAST] = robot;
                    node->x = current->x - 1;
                    node->y = current->y;
                    push_input(vm, D_EAST);
                    break;
                case D_EAST:
                    node->neighbours[D_WEST] = robot;
                    node->x = current->x + 1;
                    node->y = current->y;
                    push_input(vm, D_WEST);
                    break;
                case END_PATH:
                default:
                    /* unreachable */
                    break;
                }
                if (res == R_OXYGEN)
                    m->oxygen = node;
                /*printf("pushing x=%d, y=%d to stack\n", node->x, node->y);*/
                push_stack(&xs, node);
                run_vm(vm, 0); /* move back */
                pop_output(vm); /* discard feedback */
            }
        }
    }
    return;
}

/* Main function */

int main(void) {
    num_t file_buffer[8192];
    size_t n = load_file("input15.txt", file_buffer);
    struct intcode_vm *vm = vm_from_buffer(file_buffer, n);

    size_t res;
    struct maze m = {0};
    explore_maze(vm, &m);
    free_vm(vm);

    /* Part 1 */
    if (m.oxygen) {
        res = find_path(m.robot, m.oxygen->x, m.oxygen->y, (void *) 0);
        printf("%lu\n", res);
    } else {
        printf("oxygen not found\n");
        return 0;
    }
    /* Part 2 */
    const int invalid_coord = ~0;
    res = find_path(m.oxygen, invalid_coord, invalid_coord, (void *) 0);
    printf("%lu\n", res);
    /* Maze not freed */
    return 0;
}
