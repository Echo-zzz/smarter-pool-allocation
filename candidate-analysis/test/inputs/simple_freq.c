#include <stddef.h>
#include <stdlib.h>

typedef struct Entry {
    int hot;
    int warm;
    int cold;
} Entry;

static volatile int hot_flag = 1;
static volatile int warm_flag = 1;

int main(void) {
    const size_t init_iters = 8;
    const size_t hot_iters = 1000;
    const size_t warm_iters = 64;

    Entry *data = (Entry *)malloc(sizeof(Entry) * init_iters);
    if (!data)
        return -1;

    for (size_t i = 0; i < init_iters; ++i) {
        data[i].hot = (int)i;
        data[i].warm = (int)(i * 2);
        data[i].cold = (int)(i * 3);
    }

    int total = 0;
    if (__builtin_expect_with_probability(hot_flag, 1, 0.99)) {
        for (size_t i = 0; i < hot_iters; ++i) {
            size_t idx = i % init_iters;
            total += data[idx].hot;
        }
    }

    if (__builtin_expect_with_probability(warm_flag, 1, 0.15)) {
        for (size_t i = 0; i < warm_iters; ++i) {
            size_t idx = i % init_iters;
            total += data[idx].warm;
        }
    }

    free(data);
    return total;
}
