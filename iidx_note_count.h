#ifndef IIDX_NOTE_COUNT_H_
#define IIDX_NOTE_COUNT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "iidx_1.h"

int read_chart(const char *music_id, iidx_1_chart chart);
int read_charts(const char *music_id, iidx_1_note_counts *out_note_counts);

#ifdef __cplusplus
}
#endif

#endif // IIDX_NOTE_COUNT_H_
