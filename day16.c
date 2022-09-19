#include <stdio.h>
#include <stdint.h>

#define MAX_DIGITS 1000

void populate_buffer(char *filename, int32_t *buffer) {
    FILE *fp = fopen(filename, "r");
    while (1) {
        char c = fgetc(fp);
        if ('0' <= c && c <= '9') {
            *buffer++ = c - '0';
        } else {
            break;
        }
    }
    *buffer = -1;
    fclose(fp);
    return;
}

int32_t pattern_at(size_t input_index, size_t output_pos, int32_t offset) {
    if (output_pos > input_index) {
        return 0;
    } else if (output_pos * 2 > input_index) {
        return 1;
    } else {
        int32_t pattern[4] = {0, 1, 0, -1};
        if (output_pos == 0)
            return pattern[(input_index + offset) % 4];
        size_t pattern_ptr = 0;
        size_t j = 1;
        for (int32_t i = 0; i < input_index; i++) {
            j++;
            if (j == output_pos + offset) {
                j = 0;
                pattern_ptr = (pattern_ptr + 1) % 4;
            }
        }
        return pattern[pattern_ptr];
    }
}

void apply_transform(int32_t *buffer) {
    for (int i = 0; buffer[i] != -1; i++) {
        int32_t total = 0;
        for (int j = 0; buffer[j] != -1; j++) {
            /*
            printf("%2d*%2d   ", buffer[j], pattern_at(j, i, 1));
            */
            total += (buffer[j] * pattern_at(j, i, 1));
        }
        if (total < 0) total = -total;
        buffer[i] = total % 10;
        /*
        printf(" = %d\n", total % 10);
        */
    }
    return;
}

int main(void) {
    int32_t buffer[MAX_DIGITS] = {0};
    populate_buffer("input16.txt", buffer);

    for (int i = 0; i < 100; i++) {
        apply_transform(buffer);
    }
    printf("%d%d%d%d%d%d%d%d\n",
        buffer[0], buffer[1], buffer[2], buffer[3],
        buffer[4], buffer[5], buffer[6], buffer[7]);
    return 0;
}

/* We'll need to go bigger for part 2... */
