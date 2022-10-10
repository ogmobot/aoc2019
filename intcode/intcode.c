#include <malloc.h>
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

static void stretch_mem_to(struct intcode_memory *mem, size_t min_size) {
    /* Doubles the size of memory until it's at least as large as necessary */
    while (mem->size < min_size) {
        size_t tmp = mem->size;
        mem->size *= 2;
        mem->data = realloc(mem->data, sizeof(num_t) * mem->size);
        /* Assumes realloc won't fail */
        /* New memory must be zeroed */
        while (tmp < mem->size)
            (mem->data)[tmp++] = 0;
    }
    return;
}

num_t get_memory_direct(struct intcode_vm *vm, size_t addr) {
    stretch_mem_to(&(vm->mem), addr + 1);
    return (vm->mem.data)[addr];
}

void set_memory_direct(struct intcode_vm *vm, size_t addr, num_t val) {
    stretch_mem_to(&(vm->mem), addr + 1);
    (vm->mem.data)[addr] = val;
    return;
}

void initialise_vm(struct intcode_vm *vm) {
    /* This function mangles all pointer values! All pointers MUST be freed
     * before calling this function if they were previously malloc'd! */
    vm->mem.size = INITIAL_MEMORY_SIZE;
    vm->mem.data = calloc(vm->mem.size, sizeof(num_t));
    vm->input.reader = 0;
    vm->input.writer = 0;
    for (size_t i = 0; i < IO_BUFFER_LENGTH; i++)
        vm->input.buffer[i] = 0;
    vm->output.reader = 0;
    vm->output.writer = 0;
    for (size_t i = 0; i < IO_BUFFER_LENGTH; i++)
        vm->output.buffer[i] = 0;
    vm->pc = 0;
    vm->relbase = 0;
    return;
}

void free_vm(struct intcode_vm *vm) {
    free(vm->mem.data);
    free(vm);
}

void initialise_vm_from_file(struct intcode_vm *vm, char *filename) {
    initialise_vm(vm);
    size_t mem_index = 0;
    num_t num_reader;
    char char_reader;
    FILE *fp = fopen(filename, "r");
    while (fscanf(fp, "%d%c", &num_reader, &char_reader) != EOF) {
        set_memory_direct(vm, mem_index++, num_reader);
        if (char_reader != ',')
            break;
    }
    fclose(fp);
    return;
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

static int digit_at(num_t value, size_t place) {
    /* e.g. digit_at(3456, 100) => 4 */
    return (value / place) % 10;
}

static void increment_pc(struct intcode_vm *vm) {
    num_t opcode = get_memory_direct(vm, vm->pc);
    switch (opcode % 100) {
    case 1: /* add */
    case 2: /* multiply */
        (vm->pc) += 4;
        return;
    case 3: /* input */
    case 4: /* output */
        (vm->pc) += 2;
        return;
    case 5: /* input */
    case 6: /* output */
        (vm->pc) += 3;
        return;
    case 7: /* less than */
    case 8: /* equals */
        (vm->pc) += 4;
        return;
    case 9: /* adjust offset */
        (vm->pc) += 2;
        return;
    case 99: /* halt */
        return;
    default:
        printf("Unrecognised opcode %d\n", opcode);
        (vm->pc) += 1;
        return;
    }
}
