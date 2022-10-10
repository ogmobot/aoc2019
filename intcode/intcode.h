#ifndef INTCODE_H

#include <stdint.h>
#define num_t               int32_t
#define INITIAL_MEMORY_SIZE 1024
#define IO_BUFFER_LENGTH    1024

/* Enums and structs */

enum param {
    P_INDIRECT  = 0,
    P_IMMEDIATE = 1,
    P_OFFSET    = 2
};

struct intcode_memory {
    num_t *data;
    size_t size;
};

struct intcode_io {
    num_t buffer[IO_BUFFER_LENGTH];
    size_t writer; /* index at which buffer is written */
    size_t reader; /* index from which buffer is read */
};

struct intcode_vm {
    struct intcode_memory mem;
    struct intcode_io input;    /* Input buffer */
    struct intcode_io output;   /* Output buffer */
    size_t pc;                  /* Program counter */
    num_t relbase;              /* Base offset for relative memory access */
};

/* Function signatures */

extern void initialise_vm(struct intcode_vm *);
extern void initialise_vm_from_file(struct intcode_vm *, char *);
extern void free_vm(struct intcode_vm *);

extern num_t get_memory_direct(struct intcode_vm *, size_t);
extern void set_memory_direct(struct intcode_vm *, size_t, num_t);
extern num_t get_vm_value(struct intcode_vm *, enum param, num_t);
extern void set_vm_value(struct intcode_vm *, enum param, num_t, num_t);

extern void push_input(struct intcode_vm *, num_t);
extern num_t pop_output(struct intcode_vm *);

#define INTCODE_H
#endif
