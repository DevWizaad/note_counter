#include "iidx_1.h"

#include <string.h>

#define BINARY_STREAM_DEFINITIONS
#include "binary_stream.h"

#define CHART_END_SIGNATURE 0x7fffffff

typedef struct
{
  struct
  {
    uint32_t offset;
    uint32_t length;
  } charts[IIDX_1_MAX_CHART_COUNT];
} iidx_1_header;

static int get_note_count(uint8_t *chart, uint32_t length)
{
  // validate parameters, length MUST be a multiple of 8.
  if (chart == NULL || length == 0 || (length & 0x07))
    return 0;
  
  // open chart for as binary stream.
  binary_stream *bs = bs_open(chart, length);
  int note_count = 0;
  while (!bs_at_end(bs))
  {
    // read in 8 bytes.
    uint32_t event_offset = bs_read_u32(bs);
    uint8_t event_type = bs_read_u8(bs);
    uint8_t event_param = bs_read_u8(bs);
    uint16_t event_value = bs_read_u16(bs);

    // check if end of chart.
    if (event_offset == CHART_END_SIGNATURE)
      break;

    // check if it's a note. (0x00 == 1p note, 0x01 == 2p note)
    if (event_type == 0x00 || event_type == 0x01)
    {
      // check if it's a charge note.
      note_count += event_value > 0 ? 2 : 1;
    }
  }

  bs_close(bs);
  return note_count;
}

int iidx_1_get_note_counts(uint8_t *file, uint32_t file_length, iidx_1_note_counts *out_note_counts)
{
  if (file == NULL || file_length < sizeof(iidx_1_header) || out_note_counts == NULL)
    return -1;

  // read file header.
  iidx_1_header header;
  memcpy(&header, file, sizeof(header));
  iidx_1_note_counts note_counts;
  for (int i = 0; i < IIDX_1_MAX_CHART_COUNT; ++i)
  {
    // get note counts for all charts.
    note_counts.charts[i] = get_note_count(file + header.charts[i].offset, header.charts[i].length);
  }

  *out_note_counts = note_counts;
  return 0;
}

int iidx_1_get_note_count(uint8_t *file, uint32_t file_length, iidx_1_chart chart)
{
  if (file == NULL || file_length < sizeof(iidx_1_header) || (uint32_t) chart >= IIDX_1_MAX_CHART_COUNT)
    return -1;
  
  iidx_1_header *header = (iidx_1_header*) file;
  return get_note_count(file + header->charts[chart].offset, header->charts[chart].length);
}
