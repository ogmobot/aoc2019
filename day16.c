#include <malloc.h>
#include <stdio.h>
#include <stdint.h>

/* Struct handling */

struct message {
    size_t size;
    int32_t *data;
};

struct message *new_message(size_t size) {
    /* size should never be zero */
    struct message *m = calloc(1, sizeof(struct message));
    m->size = size;
    m->data = calloc(size, sizeof(int32_t));
    return m;
}

void free_message(struct message *m) {
    free(m->data);
    free(m);
}

/* IO handling */

void populate_buffer(char *filename, struct message *m) {
    FILE *fp = fopen(filename, "r");
    size_t index = 0;
    while (1) {
        char c = fgetc(fp);
        if ('0' <= c && c <= '9') {
            if (index >= m->size - 1) {
                m->size *= 2;
                m->data = realloc(m->data, sizeof(int32_t) * m->size);
                /* doesn't initialise new memory */
            }
            (m->data)[index++] = c - '0';
        } else {
            break;
        }
    }
    m->size = index; /* Don't bother reallocating memory */
    fclose(fp);
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
        for (int32_t i = 0; i < input_index; i++) {
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
    for (int i = 0; i < m->size; i++) {
        int32_t total = 0;
        for (int j = 0; j < m->size; j++) {
            /*
            printf("%2d*%2d   ", buffer[j], pattern_at(j, i));
            */
            total += ((m->data)[j] * pattern_at(j, i));
        }
        if (total < 0) total = -total;
        (m->data)[i] = total % 10;
        /*
        printf(" = %d\n", total % 10);
        */
    }
    return;
}

/* Main function */

int main(void) {
    struct message *m = new_message(1024);
    populate_buffer("input16.txt", m);

    for (int i = 0; i < 100; i++) {
        apply_transform(m);
    }
    printf("%d%d%d%d%d%d%d%d\n",
        m->data[0], m->data[1], m->data[2], m->data[3],
        m->data[4], m->data[5], m->data[6], m->data[7]);
    return 0;
}

/* We'll need to go bigger for part 2... */
