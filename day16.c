#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Struct handling */

struct message {
    size_t size;
    int32_t *data;
    int32_t *next;
};

struct message *new_message(size_t size) {
    /* size should never be zero */
    struct message *m = calloc(1, sizeof(struct message));
    m->size = size;
    m->data = calloc(size, sizeof(int32_t));
    m->next = calloc(size, sizeof(int32_t));
    return m;
}

void free_message(struct message *m) {
    free(m->data);
    free(m);
}

void swap_buffer(struct message *m) {
    /* Swaps the values of the data and next pointers, so that it seems like
       their contents have been swapped. (Will the compiler optimise it?) */
    void *tmp = m->data;
    m->data = m->next;
    m->next = tmp;
    return;
}

void multiply_length(struct message *m, size_t factor) {
    size_t orig_size = m->size;
    (m->size) *= factor;
    m->data = realloc(m->data, sizeof(int32_t) * m->size);
    m->next = realloc(m->next, sizeof(int32_t) * m->size);
    for (size_t i = 1; i < factor; i++) {
        memcpy(m->data + (i * orig_size), m->data, orig_size * sizeof(int32_t));
    }
    return;
}

/* IO handling */

void populate_buffer(char *filename, struct message *m) {
    FILE *fp = fopen(filename, "r");
    size_t index = 0;
    while (1) {
        char c = fgetc(fp);
        if ('0' <= c && c <= '9') {
            if (index >= m->size - 1) {
                m->size += 1024;
                m->data = realloc(m->data, sizeof(int32_t) * m->size);
                /* doesn't initialise new memory */
            }
            (m->data)[index++] = c - '0';
        } else {
            break;
        }
    }
    m->size = index;
    m->data = realloc(m->data, sizeof(int32_t) * m->size);
    m->next = realloc(m->next, sizeof(int32_t) * m->size);
    /* doesn't initialise new memory */
    fclose(fp);
    return;
}

void output_array_i32(int32_t *buffer, size_t n) {
    for (size_t i = 0; i < n; i++)
        printf("%d", *buffer++);
    printf("\n");
    return;
}

/* Program logic */

int32_t pattern_at(size_t input_index, size_t output_pos) {
    if (output_pos > input_index) {
        return 0;
    } else if (output_pos * 2 > input_index) {
        return 1;
    } else {
        int32_t pattern[4] = {0, 1, 0, -1};
        if (output_pos == 0)
            return pattern[(input_index + 1) % 4];
        size_t pattern_ptr = 0;
        size_t j = 1;
        for (size_t i = 0; i < input_index; i++) {
            j++;
            if (j == output_pos + 1) {
                j = 0;
                pattern_ptr = (pattern_ptr + 1) % 4;
            }
        }
        return pattern[pattern_ptr];
    }
}

void apply_transform(struct message *m) {
    /* Part 2:
       My first instinct was to construct a size * size matrix A, then find
       eigenvectors and eigenvalues of A, use them to compute A^100, then
       find (A^100)m.
       Fortunately, we can exploit the structure of A to do this without
       needing to allocate terabytes of memory.
    */
    const int32_t deltas[4] = {1, -1, -1, 1};
    size_t delta_index = 0;
    size_t step_size = 1;
    for (size_t i = 0; i < m->size; i++) {
        (m->next)[i] = 0;
    }
    while (step_size <= m->size) {
        /* Step down to the bottommost index of the "matrix"... */
        int32_t read_head = step_size - 1;
        int32_t write_head = (m->size / step_size) - 1;
        while (read_head < m->size)
            read_head += step_size;
        /*
        printf("step size %lu\n", step_size);
        */
        /* Then step back up, adding as we go */
        size_t timer = 0;
        int32_t subtotal = 0;
        while (read_head >= 0) {
            read_head--;
            timer++;
            if (read_head >= 0 && read_head < m->size) {
                subtotal += (m->data)[read_head];
            }
            if (write_head >= 0
                && write_head < m->size
                && timer % step_size == 0) {
                (m->next)[write_head--] += deltas[delta_index] * subtotal;
                /*
                printf("subtotal = %d\n", subtotal);
                printf("%d %d %d %d %d %d %d %d\n",
                    m->next[0], m->next[1], m->next[2], m->next[3],
                    m->next[4], m->next[5], m->next[6], m->next[7]);
                */
            }
        }
        delta_index = (delta_index + 1) % 4;
        step_size++;
    }
    for (size_t i = 0; i < m->size; i++) {
        (m->next)[i] %= 10;
        if ((m->next)[i] < 0)
            m->next[i] *= -1;
    }
    swap_buffer(m);
    return;
    /* Hm... This approach is ~6 times faster, but still too slow for part 2 */
}

void apply_first_pass(struct message *m, size_t offset) {
    /* Quick 'n' dirty version of part 2; solve a simplified problem by relying
       on the fact that we don't need any of the numbers in the first half of
       the result. (Skips more than half of the computation!)
       We can even skip filling in *everything* lower than the offset.
    */
    int32_t index = m->size - 1;
    int32_t subtotal = 0;
    while (index >= offset) {
        subtotal += (m->data)[index];
        (m->next)[index--] = subtotal % 10; /* Can't even be negative */
    }
    swap_buffer(m);
    return;
}

size_t get_offset(struct message *m) {
    size_t offset = 0;
    for (size_t i = 0; i < 7; i++) {
        offset *= 10;
        offset += (m->data)[i];
    }
    return offset;
}

/* Main function */

int main(void) {
    struct message *m = new_message(1024);
    populate_buffer("input16.txt", m);

    for (size_t i = 0; i < 100; i++) {
        apply_transform(m);
    }
    output_array_i32(m->data, 8);

    populate_buffer("input16.txt", m);
    size_t offset = get_offset(m);
    multiply_length(m, 10000);

    for (size_t i = 0; i < 100; i++) {
        apply_first_pass(m, offset); /* Assume offset > (size / 2) */
    }
    output_array_i32(m->data + offset, 8);

    free_message(m);
    return 0;
}
