#include <stdio.h>

#define LAYER_WIDTH  25
#define LAYER_HEIGHT  6
#define NUM_LAYERS  100
#define S_BLACK     ' '
#define S_WHITE     '@'

int read_layer(FILE *fp, char *buffer) {
    int n_zeros = 0;
    for (int i = 0; i < (LAYER_WIDTH * LAYER_HEIGHT); i++) {
        buffer[i] = fgetc(fp);
        if (buffer[i] == '0')
            n_zeros++;
    }
    return n_zeros;
}

int score_layer(char *layer) {
    int num_ones = 0;
    int num_twos = 0;
    for (int i = 0; i < (LAYER_WIDTH * LAYER_HEIGHT); i++) {
        switch (layer[i]) {
        case '1':
            num_ones++;
            break;
        case '2':
            num_twos++;
            break;
        }
    }
    return num_ones * num_twos;
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
    int zero_layer = 0;
    for (int i = 0; i < NUM_LAYERS; i++) {
        int tmp = read_layer(fp, layers[i]);
        if (tmp <= min_zeros) {
            min_zeros = tmp;
            zero_layer = i;
        }
    }
    fclose(fp);
    printf("%d\n", score_layer(layers[zero_layer]));
    /* Part 2 */
    char display[LAYER_WIDTH * LAYER_HEIGHT] = {0};
    for (int i = 0; i < NUM_LAYERS; i++)
        apply_layer(layers[NUM_LAYERS - i - 1], display);
    display_buffer(display);
    return 0;
}
