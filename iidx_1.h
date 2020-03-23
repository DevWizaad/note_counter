#ifndef IIDX_1_H_
#define IIDX_1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
  IIDX_1_SPH = 0,
  IIDX_1_SPN = 1,
  IIDX_1_SPA = 2,
  IIDX_1_SPB = 3,

  IIDX_1_DPH = 6,
  IIDX_1_DPN = 7,
  IIDX_1_DPA = 8,

  IIDX_1_MAX_CHART_COUNT = 12
} iidx_1_chart;

typedef struct
{
  int charts[IIDX_1_MAX_CHART_COUNT];
} iidx_1_note_counts;

int iidx_1_get_note_counts(uint8_t *file, uint32_t file_length, iidx_1_note_counts *out_note_counts);
int iidx_1_get_note_count(uint8_t *file, uint32_t file_length, iidx_1_chart chart);

#ifdef __cplusplus
}
#endif

#endif // IIDX_1_H_
