#include <stdio.h>

#define LAYER_WIDTH  25
#define LAYER_HEIGHT  6
#define NUM_LAYERS  100
#define S_BLACK     ' '
#define S_WHITE     '@'

struct counter {
    int freqs[3]; /* Assume only 0, 1, 2 can appear */
};

struct counter read_layer(FILE *fp, char *buffer) {
    struct counter res = {0};
    for (int i = 0; i < (LAYER_WIDTH * LAYER_HEIGHT); i++) {
        buffer[i] = fgetc(fp);
        if ('0' <= buffer[i] && buffer[i] <= '2')
            res.freqs[buffer[i] - '0']++;
    }
    return res;
}

void apply_layer(char *layer, char *buffer) {
    for (int i = 0; i < LAYER_WIDTH * LAYER_HEIGHT; i++) {
        switch (layer[i]) {
        case '0':
            buffer[i] = S_BLACK;
            break;
        case '1':
            buffer[i] = S_WHITE;
            break;
        }
    }
    return;
}

void display_buffer(char *buffer) {
    for (int row = 0; row < LAYER_HEIGHT; row++) {
        for (int col = 0; col < LAYER_WIDTH; col++) {
            printf("%c ", buffer[(LAYER_WIDTH * row) + col]);
        }
        printf("\n");
    }
    return;
}

int main(void) {
    char layers[NUM_LAYERS][LAYER_WIDTH * LAYER_HEIGHT];
    FILE *fp = fopen("input08.txt", "r");
    /* Part 1 */
    int min_zeros = LAYER_WIDTH * LAYER_HEIGHT;
    int score = 0;
    for (int i = 0; i < NUM_LAYERS; i++) {
        struct counter tmp = read_layer(fp, layers[i]);
        if (tmp.freqs[0] <= min_zeros) {
            min_zeros = tmp.freqs[0];
            score = tmp.freqs[1] * tmp.freqs[2];
        }
    }
    fclose(fp);
    printf("%d\n", score);
    /* Part 2 */
    char display[LAYER_WIDTH * LAYER_HEIGHT] = {0};
    for (int i = 0; i < NUM_LAYERS; i++)
        apply_layer(layers[NUM_LAYERS - i - 1], display);
    display_buffer(display);
    return 0;
}
