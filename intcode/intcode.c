#include <malloc.h>
#include <stdio.h>
#include "intcode.h"

/* Initialising structs */

static struct intcode_memory *find_chunk(struct intcode_vm *vm, num_t addr) {
    /* Finds the chunk of memory in which the given address resides */
    /* Allocates memory! */
    num_t target_chunk_start = addr / CHUNK_SIZE;
    struct intcode_memory *chunk = &(vm->mem);
    while (chunk->start != target_chunk_start) {
        if (chunk->next == (void *) 0) {
            chunk->next = calloc(1, sizeof(struct intcode_memory));
            if (chunk->next) {
                chunk->next->next = (void *) 0;
                chunk->next->start = target_chunk_start;
            } else {
                vm->status = F_CRASHED;
                vm->error_cause = E_OUT_OF_MEMORY;
                return (void *) 0;
            }
        }
        chunk = chunk->next;
    }
    return chunk;
}

num_t get_memory_direct(struct intcode_vm *vm, num_t addr) {
    struct intcode_memory *chunk = find_chunk(vm, addr);
    if (chunk)
        return (chunk->data)[addr - (chunk->start * CHUNK_SIZE)];
    return 0;
}

void set_memory_direct(struct intcode_vm *vm, num_t addr, num_t val) {
    struct intcode_memory *chunk = find_chunk(vm, addr);
    if (chunk)
        (chunk->data)[addr - (chunk->start * CHUNK_SIZE)] = val;
    return;
}

struct intcode_vm *initialise_vm(void) {
    /* Allocates memory! */
    struct intcode_vm *vm = calloc(1, sizeof(struct intcode_vm));
    /* calloc ensures memory is zeroed */
    return vm;
}

size_t load_file(char *filename, num_t *buffer) {
    num_t *writer = buffer;
    num_t num_reader;
    char char_reader;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Couldn't open file \"%s\"\n", filename);
        return 0;
    }
    while (fscanf(fp, NUM_T_SCAN "%c", &num_reader, &char_reader) != EOF) {
        *writer++ = num_reader;
        if (char_reader != ',')
            break;
    }
    fclose(fp);
    return writer - buffer;
}

struct intcode_vm *vm_from_buffer(num_t *buffer, size_t buffer_len) {
    /* Allocates memory! */
    struct intcode_vm *vm = initialise_vm();
    for (size_t i = 0; i < buffer_len; i++)
        set_memory_direct(vm, i, *(buffer + i));
    return vm;
}

void free_vm(struct intcode_vm *vm) {
    struct intcode_memory *memptr = vm->mem.next;
    while (memptr) {
        struct intcode_memory *tmp = memptr->next;
        free(memptr);
        memptr = tmp;
    }
    free(vm);
}

/* Reading/writing memory addressed via opcode */

num_t get_vm_value(struct intcode_vm *vm, enum param p, num_t arg) {
    switch (p) {
    case P_INDIRECT:
        return get_memory_direct(vm, arg);
    case P_IMMEDIATE:
        return arg;
    case P_OFFSET:
        return get_memory_direct(vm, arg + vm->relbase);
    }
    return 0;
}

void set_vm_value(struct intcode_vm *vm, enum param p, num_t arg, num_t val) {
    switch (p) {
    case P_INDIRECT:
        set_memory_direct(vm, arg, val);
        return;
    case P_IMMEDIATE:
        /* Can't use immediate mode for a destination */
        vm->status = F_CRASHED;
        vm->error_cause = E_SET_IMMEDIATE;
        return;
    case P_OFFSET:
        set_memory_direct(vm, arg + vm->relbase, val);
        return;
    }
    return;
}

/* IO functions */

static void write_io_buffer(struct intcode_io *io, num_t val) {
    io->buffer[io->writer] = val;
    io->writer = (io->writer + 1) % IO_BUFFER_LENGTH;
}

static num_t read_io_buffer(struct intcode_io *io) {
    num_t ret = (io->buffer)[io->reader];
    io->reader = (io->reader + 1) % IO_BUFFER_LENGTH;
    return ret;
}

void push_input(struct intcode_vm *vm, num_t val) {
    write_io_buffer(&(vm->input), val);
    vm->status &= ~F_REQUIRE_INPUT;
    return;
}

num_t pop_output(struct intcode_vm *vm) {
    return read_io_buffer(&(vm->output));
}

/* CPU emulation */

static enum param digit_at(num_t value, size_t place) {
    /* e.g. digit_at(3456, 100) => 4 */
    return (enum param) ((value / place) % 10);
}

static void step_vm(struct intcode_vm *vm) {
    if (vm->status & (F_CRASHED | F_HALTED))
        return;
    vm->status = F_OKAY;
    num_t opcode = get_memory_direct(vm, vm->pc);
    num_t args[3];
    enum param mode[3];
    size_t place = 100;
    for (size_t i = 0; i < 3; i++) {
        args[i] = get_memory_direct(vm, vm->pc + i + 1);
        mode[i] = digit_at(opcode, place);
        place *= 10;
    }
    switch (opcode % 100) {
    case OP_ADD:
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                +
            get_vm_value(vm, mode[1], args[1])
        );
        return;
    case OP_MULTIPLY:
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                *
            get_vm_value(vm, mode[1], args[1])
        );
        return;
    case OP_INPUT:
        if (vm->input.reader == vm->input.writer) {
            vm->status = F_REQUIRE_INPUT;
        } else {
            (vm->pc) += 2;
            set_vm_value(vm, mode[0], args[0], read_io_buffer(&(vm->input)));
        }
        return;
    case OP_OUTPUT:
        (vm->pc) += 2;
        write_io_buffer(&(vm->output), get_vm_value(vm, mode[0], args[0]));
        vm->status = F_PUSHED_OUTPUT;
        return;
    case OP_JUMP_IF_TRUE:
        (vm->pc) += 3;
        if (get_vm_value(vm, mode[0], args[0]))
            vm->pc = get_vm_value(vm, mode[1], args[1]);
        return;
    case OP_JUMP_IF_FALSE:
        (vm->pc) += 3;
        if (!get_vm_value(vm, mode[0], args[0]))
            vm->pc = get_vm_value(vm, mode[1], args[1]);
        return;
    case OP_LESS_THAN:
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                <
            get_vm_value(vm, mode[1], args[1])
        );
        return;
    case OP_EQUALS:
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                ==
            get_vm_value(vm, mode[1], args[1])
        );
        return;
    case OP_ADJUST_OFFSET:
        (vm->pc) += 2;
        vm->relbase += get_vm_value(vm, mode[0], args[0]);
        return;
    case OP_HALT:
        vm->status = F_HALTED;
        return;
    default:
        vm->status = F_CRASHED;
        vm->error_cause = E_UNKNOWN_OPCODE;
        return;
    }
}

void run_vm(struct intcode_vm *vm, int stop_at_output) {
    int stopflags = F_HALTED | F_CRASHED | F_REQUIRE_INPUT;
    if (stop_at_output)
        stopflags |= F_PUSHED_OUTPUT;
    do {
        step_vm(vm);
    } while (!(vm->status & stopflags));
    return;
}

void run_vm_interactive(struct intcode_vm *vm, enum interact_mode mode) {
    size_t print_count;
    int stopflags = F_HALTED | F_CRASHED | F_REQUIRE_INPUT;
    while (!(vm->status & (F_HALTED | F_CRASHED))) {
        print_count = 0;
        while (print_count < IO_BUFFER_LENGTH && (!(vm->status & stopflags))) {
            run_vm(vm, 1);
            if (vm->status & F_PUSHED_OUTPUT)
                print_count++;
        }
        for (size_t i = 0; i < print_count; i++) {
            num_t tmp = pop_output(vm);
            if (mode == I_NUMERIC || tmp < 0 || tmp > UINT8_MAX) {
                printf(NUM_T_FORMAT "\n", tmp);
            } else {
                if (mode == I_ASCII)
                    putchar((char) tmp);
            }
        }
        if (vm->status & F_REQUIRE_INPUT) {
            int received_input = 0;
            if (mode == I_NUMERIC) {
                num_t gotnum;
                int res = scanf(NUM_T_SCAN, &gotnum);
                if (res != EOF) {
                    received_input = 1;
                    if (res)
                        push_input(vm, gotnum);
                }
            } else {
                int gotch = getchar();
                while (gotch != EOF) {
                    received_input = 1;
                    push_input(vm, (num_t) gotch);
                    if ((char) gotch == '\n')
                        break;
                    gotch = getchar();
                }
            }
            if (!received_input) {
                vm->status = F_CRASHED;
                vm->error_cause = E_EXPECTED_INPUT;
            }
        }
    }
    return;
}
