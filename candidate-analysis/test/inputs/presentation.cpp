#include <stddef.h>
#include <stdlib.h>

typedef struct Type1
{
    int hot;
    int warm;
    int cold;
} Type1;

typedef struct Type2
{
    int cold_1;
    int cold_2;
} Type2;

int main()
{
    Type1 *data = (Type1 *)malloc(sizeof(Type1) * 10);
    Type2 *not_used = (Type2 *)malloc(sizeof(Type2) * 10);
    if (!data)
        return -1;

    if (!not_used)
        return -1;

    for (size_t i = 0; i < 10; ++i)
    {
        data[i].hot = (int)i;
        data[i].warm = (int)i;
        data[i].cold = (int)i;
        not_used[i].cold_1 = (int)i;
        not_used[i].cold_2 = (int)i;
    }

    int total = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }

    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }

    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    for (size_t i = 0; i < 10; ++i)
    {
        total += data[i].hot;
    }
    free(data);
    return total;
}