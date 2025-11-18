#include <stddef.h>
#include <stdlib.h>

typedef struct Entry {
    int value;
    int weight;
    int flags;
} Entry;

int process_structs(size_t len) {
    Entry *data = (Entry *)malloc(sizeof(Entry) * len);
    if (!data)
        return -1;

    for (size_t i = 0; i < len; ++i) {
        data[i].value = (int)i;
        data[i].weight = (int)(len - i);
        data[i].flags = (int)(i & 1u);
    }

    int total = 0;

    for (size_t i = 0; i < len; ++i) {
        if ((i & 1u) == 0)
            total += data[i].value;
        else
            total += data[i].weight;
    }

    for (size_t outer = 0; outer < len; ++outer) {
        int pair_sum = 0;
        for (size_t inner = outer + 1; inner < len; ++inner) {
            if ((data[inner].flags & 1) == 0)
                pair_sum += data[outer].value * data[inner].weight;
            else
                pair_sum += data[outer].weight * data[inner].value;
        }
        total += pair_sum;
    }

    free(data);
    return total;
}

int main(void) {
    return process_structs(1000);
}
