#include <stddef.h>
#include <stdlib.h>

typedef struct Entry
{
    int hot;
    int warm;
    int cold;
} Entry;

int main()
{
    Entry *data = (Entry *)malloc(sizeof(Entry) * 10);
    if (!data)
        return -1;

    for (size_t i = 0; i < 10; ++i)
    {
        data[i].hot = (int)i;
        data[i].warm = (int)i;
        data[i].cold = (int)i;
    }

    int total = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }

    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
        total += data[i].warm;
    }

    free(data);
    return total;
}