#ifndef INTCODE_H

#include <inttypes.h>
#define num_t               int64_t
#define NUM_T_FORMAT        "%" PRId64
#define CHUNK_SIZE          1024
#define IO_BUFFER_LENGTH    1024

/* Enums and structs */

enum param {
    P_INDIRECT  = 0,
    P_IMMEDIATE = 1,
    P_OFFSET    = 2
};

enum stepcode {
    F_OKAY          = 0x00,
    F_CRASHED       = 0x01,
    F_HALTED        = 0x02,
    F_PUSHED_OUTPUT = 0x04,
    F_REQUIRE_INPUT = 0x08
};

enum crashcode {
    E_NO_CRASH = 0,
    E_NEGATIVE_ADDRESS,
    E_SET_IMMEDIATE,
    E_UNKNOWN_OPCODE,
    E_OUT_OF_MEMORY,
    E_EXPECTED_INPUT
};

struct intcode_memory {
    struct intcode_memory *next;
    num_t data[CHUNK_SIZE];
    num_t start;
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
    num_t pc;                   /* Program counter */
    num_t relbase;              /* Base offset for relative memory access */
    enum stepcode status;       /* Running? Crashed? Waiting for input? */
    enum crashcode error_cause;
};

/* Function signatures */

extern struct intcode_vm *initialise_vm(void);
extern size_t load_file(char *, num_t *);
extern struct intcode_vm *vm_from_buffer(num_t *, size_t);
extern void free_vm(struct intcode_vm *);

extern num_t get_memory_direct(struct intcode_vm *, num_t);
extern void set_memory_direct(struct intcode_vm *, num_t, num_t);
extern num_t get_vm_value(struct intcode_vm *, enum param, num_t);
extern void set_vm_value(struct intcode_vm *, enum param, num_t, num_t);

extern void push_input(struct intcode_vm *, num_t);
extern num_t pop_output(struct intcode_vm *);

extern void run_vm(struct intcode_vm *, int);
extern void run_vm_ascii(struct intcode_vm *);

#define INTCODE_H
#endif
