#include <stddef.h>
#include <stdlib.h>

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int value;
    int weight;
    int flags;
} Entry;

int mixed_process(size_t len) {
    Point *pts = (Point *)malloc(sizeof(Point) * len);
    Entry *entries = (Entry *)malloc(sizeof(Entry) * len);
    if (!pts || !entries) {
        free(pts);
        free(entries);
        return -1;
    }

    for (size_t i = 0; i < len; ++i) {
        pts[i].x = (int)i;
        pts[i].y = (int)(len - i);
        entries[i].value = (int)(i * 2);
        entries[i].weight = (int)(len - i * 2);
        entries[i].flags = (int)(i & 1u);
    }

    int total = 0;

    for (size_t i = 0; i < len; ++i) {
        pts[i].x += pts[i].y;
        total += pts[i].x;
    }

    for (size_t outer = 0; outer < len; ++outer) {
        int pair = 0;
        for (size_t inner = outer + 1; inner < len; ++inner) {
            if ((entries[inner].flags & 1) == 0)
                pair += entries[outer].value * entries[inner].weight;
            else
                pair += entries[outer].weight * entries[inner].value;
        }
        total += pair;
    }

    for (size_t i = 0; i < len; ++i) {
        pts[i].y = entries[i].flags;
    }

    free(pts);
    free(entries);
    return total;
}
