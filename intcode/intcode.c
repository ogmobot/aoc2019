#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "intcode.h"

/*
 * Build with
    gcc -shared -fPIC -lc <this> -o intcode.so
 * Build and run others with
    gcc <other> intcode.so
    export LD_LIBRARY_PATH=`pwd`
    ./a.out
*/

/* Initialising structs */

static struct intcode_memory *find_chunk(struct intcode_vm *vm, num_t addr) {
    /* Finds the chunk of memory in which the given address resides */
    /* Allocates memory! */
    num_t target_chunk_start = addr / CHUNK_SIZE;
    struct intcode_memory *chunk = &(vm->mem);
    while (chunk->start != target_chunk_start) {
        if (chunk->next == (void *) 0) {
            chunk->next = calloc(1, sizeof(struct intcode_memory));
            chunk->next->next = (void *) 0;
            chunk->next->start = target_chunk_start;
        }
        chunk = chunk->next;
    }
    return chunk;
}

num_t get_memory_direct(struct intcode_vm *vm, num_t addr) {
    struct intcode_memory *chunk = find_chunk(vm, addr);
    return (chunk->data)[addr - (chunk->start * CHUNK_SIZE)];
}

void set_memory_direct(struct intcode_vm *vm, num_t addr, num_t val) {
    struct intcode_memory *chunk = find_chunk(vm, addr);
    (chunk->data)[addr - (chunk->start * CHUNK_SIZE)] = val;
    return;
}

struct intcode_vm *initialise_vm(void) {
    /* Allocates memory! */
    struct intcode_vm *vm = calloc(1, sizeof(struct intcode_vm));
    vm->mem.next = (void *) 0;
    vm->mem.start = 0;
    vm->input.reader = 0;
    vm->input.writer = 0;
    for (num_t i = 0; i < IO_BUFFER_LENGTH; i++)
        vm->input.buffer[i] = 0;
    vm->output.reader = 0;
    vm->output.writer = 0;
    for (size_t i = 0; i < IO_BUFFER_LENGTH; i++)
        vm->output.buffer[i] = 0;
    vm->pc = 0;
    vm->relbase = 0;
    return vm;
}

struct intcode_vm *initialise_vm_from_file(char *filename) {
    /* Allocates memory! */
    struct intcode_vm *vm = initialise_vm();
    size_t mem_index = 0;
    num_t num_reader;
    char char_reader;
    FILE *fp = fopen(filename, "r");
    while (fscanf(fp, NUM_T_FORMAT "%c", &num_reader, &char_reader) != EOF) {
        set_memory_direct(vm, mem_index++, num_reader);
        if (char_reader != ',')
            break;
    }
    fclose(fp);
    return vm;
}

/* FIXME */
/*
struct intcode_vm *copy_vm(struct intcode_vm *orig) {
*/
    /* Allocates memory! */
    /*
    struct intcode_vm *vm = calloc(1, sizeof(struct intcode_vm));
    memcpy(vm, orig, sizeof(struct intcode_vm));
    */
    /* vm->mem.data is reassigned without freeing, because the memory is still
     * being pointed at by orig->mem.data
     */
    /*
    vm->mem.size = orig->mem.size;
    vm->mem.data = calloc(vm->mem.size, sizeof(num_t));
    memcpy(vm->mem.data, orig->mem.data, vm->mem.size);
    return vm;
}
*/

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
        return get_memory_direct(vm, (size_t) arg);
    case P_IMMEDIATE:
        return arg;
    case P_OFFSET:
        return get_memory_direct(vm, (size_t) (arg + vm->relbase));
    }
    return 0;
}

void set_vm_value(struct intcode_vm *vm, enum param p, num_t arg, num_t val) {
    switch (p) {
    case P_INDIRECT:
        set_memory_direct(vm, (size_t) arg, val);
        return;
    case P_IMMEDIATE:
        printf("Can't use immediate mode for instruction at %lu\n", vm->pc);
        return;
    case P_OFFSET:
        set_memory_direct(vm, (size_t) (arg + vm->relbase), val);
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

static enum stepcode step_vm(struct intcode_vm *vm) {
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
    case 1: /* add */
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                +
            get_vm_value(vm, mode[1], args[1])
        );
        return F_OKAY;
    case 2: /* multiply */
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                *
            get_vm_value(vm, mode[1], args[1])
        );
        return F_OKAY;
    case 3: /* input */
        if (vm->input.reader == vm->input.writer) {
            return F_REQUIRE_INPUT;
        } else {
            (vm->pc) += 2;
            set_vm_value(vm, mode[0], args[0], read_io_buffer(&(vm->input)));
            return F_OKAY;
        }
    case 4: /* output */
        (vm->pc) += 2;
        write_io_buffer(&(vm->output), get_vm_value(vm, mode[0], args[0]));
        return F_PUSHED_OUTPUT;
    case 5: /* jump-if-true */
        (vm->pc) += 3;
        if (get_vm_value(vm, mode[0], args[0]))
            vm->pc = get_vm_value(vm, mode[1], args[1]);
        return F_OKAY;
    case 6: /* jump-if-false */
        (vm->pc) += 3;
        if (!get_vm_value(vm, mode[0], args[0]))
            vm->pc = get_vm_value(vm, mode[1], args[1]);
        return F_OKAY;
    case 7: /* less-than */
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                <
            get_vm_value(vm, mode[1], args[1])
        );
        return F_OKAY;
    case 8: /* equals */
        (vm->pc) += 4;
        set_vm_value(vm, mode[2], args[2],
            get_vm_value(vm, mode[0], args[0])
                ==
            get_vm_value(vm, mode[1], args[1])
        );
        return F_OKAY;
    case 9: /* adjust offset */
        (vm->pc) += 2;
        vm->relbase += get_vm_value(vm, mode[0], args[0]);
        return F_OKAY;
    case 99: /* halt */
        return F_HALTED;
    default:
        printf("Unhandled opcode " NUM_T_FORMAT
               " at location " NUM_T_FORMAT "\n",
            opcode, vm->pc);
        return F_CRASHED;
    }
}

void run_vm(struct intcode_vm *vm, int stop_at_output) {
    int stopflags = ((int) F_HALTED)
                  | ((int) F_CRASHED)
                  | ((int) F_REQUIRE_INPUT);
    if (stop_at_output)
        stopflags |= F_PUSHED_OUTPUT;
    int res = (int) F_OKAY;
    do {
        res = (int) step_vm(vm);
    } while (!(res & stopflags));
    return;
}
