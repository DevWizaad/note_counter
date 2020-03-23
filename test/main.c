#include <stdio.h>

#include "../iidx_note_count.h"

int main(void)
{
  iidx_1_note_counts note_counts;
  get_music_note_counts("01000", &note_counts);
  for (int i = 0; i < 12; ++i)
    printf("[%d] %d\n", i, note_counts.charts[i]);

  return 0;
}
