#include <stddef.h>

int process_array(int *data, size_t len)
{
  int total = 0;

  // Simple non-loop code to start
  if (len == 0 || data == NULL)
    return -1;

  // First loop: accumulate even-indexed elements
  for (size_t i = 0; i < len; ++i)
  {
    if ((i & 1u) == 0)
      total += data[i];
  }

  // Second loop with a nested loop: add pairwise products
  for (size_t outer = 0; outer < len; ++outer)
  {
    int pair_sum = 0;
    for (size_t inner = outer + 1; inner < len; ++inner)
      pair_sum += data[outer] * data[inner];

    total += pair_sum;
  }

  return total;
}
