#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

#define NETWORK_SIZE 50

struct packet {
    num_t x;
    num_t y;
    struct packet *next;
};

struct network_manager {
    struct intcode_vm *vms[NETWORK_SIZE];
    struct packet *input_queues[NETWORK_SIZE];
    struct packet *nat;
};

struct packet *new_packet(num_t x, num_t y) {
    struct packet *p = calloc(1, sizeof(struct packet));
    p->x = x;
    p->y = y;
    return p;
}

void push_queue(struct packet **q, struct packet *p) {
    if (!(*q)) {
        p->next = p;
        *q = p;
    } else {
        p->next = (*q)->next;
        (*q)->next = p;
        *q = p;
    }
    return;
}

struct packet *pop_queue(struct packet **q) {
    struct packet *head = (*q)->next;
    if (head->next == head) {
        *q = (void *) 0;
    } else {
        (*q)->next = head->next;
    }
    return head;
}

int main(void) {
    num_t file_buffer[8192];
    size_t n;
    n = load_file("input23.txt", file_buffer);

    struct network_manager network = {0};
    for (size_t i = 0; i < NETWORK_SIZE; i++) {
        network.vms[i] = vm_from_buffer(file_buffer, n);
        push_input(network.vms[i], i);
    }

    num_t last_y_delivered = -1;
    size_t idle_timer = 0;
    while (1) {
        for (size_t i = 0; i < NETWORK_SIZE; i++) {
            struct intcode_vm *vm = network.vms[i];
            run_vm(vm, 1);
            if (vm->status & F_REQUIRE_INPUT) {
                if (network.input_queues[i]) {
                    idle_timer = 0;
                    struct packet *p = pop_queue(&(network.input_queues[i]));
                    push_input(vm, p->x);
                    push_input(vm, p->y);
                    free(p);
                } else {
                    push_input(vm, -1);
                    idle_timer++;
                }
            } else if (vm->status & F_PUSHED_OUTPUT) {
                idle_timer = 0;
                num_t address = pop_output(vm);
                run_vm(vm, 1);
                num_t x = pop_output(vm);
                run_vm(vm, 1);
                num_t y = pop_output(vm);
                struct packet *p = new_packet(x, y);
                if (address == 255) { /* NAT address */
                    /* Part 1 */
                    if (!(network.nat))
                        printf(NUM_T_FORMAT "\n", y);
                    free(network.nat);
                    network.nat = p;
                } else {
                    push_queue(&(network.input_queues[address]), p);
                }
            }
        }
        if (idle_timer > NETWORK_SIZE) {
            if (!network.nat) {
                printf("Network became idle before anything sent to NAT!\n");
                goto free_all;
            } else {
                struct packet *p = new_packet(network.nat->x, network.nat->y);
                /* Part 2 */
                if (p->y == last_y_delivered) {
                    printf(NUM_T_FORMAT "\n", p->y);
                    free(p);
                    goto free_all;
                } else {
                    last_y_delivered = p->y;
                }
                push_queue(&(network.input_queues[0]), p);
            }
        }
    }
free_all:
    free(network.nat);
    for (size_t i = 0; i < NETWORK_SIZE; i++) {
        free_vm(network.vms[i]);
        while (network.input_queues[i]) {
            free(pop_queue(&(network.input_queues[i])));
        }
    }
    return 0;
}
