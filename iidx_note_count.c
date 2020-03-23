#include "iidx_note_count.h"

#include <stdio.h>

#include "ifs.h"

static int load_iidx_1(const char *music_id, uint8_t **out_file_buffer, int *out_file_length)
{
  uint8_t *file_buffer;
  int file_length;

  // check if .1 file already exists.
  char filename[128];
  snprintf(filename, sizeof(filename), "data/sound/%s/%s.1", music_id, music_id);
  FILE *file = fopen(filename, "rb");
  if (file != NULL)
  {
    // ifs already extracted, read the charts file.
    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_buffer = (uint8_t*) malloc(file_length);
    fread(file_buffer, sizeof(uint8_t), file_length, file);
    fclose(file);
  }
  else
  {
    // no extracted folder exists already, have to read the ifs.
    snprintf(filename, sizeof(filename), "data/sound/%s.ifs", music_id);

    mxml_node_t *manifest = NULL;
    uint32_t manifest_end = 0;
    ifs_error e = ifs_extract(filename, &manifest, &manifest_end);
    if (e != IFS_NO_ERROR)
      return e;

    // get the text of our .1 file and parse it.
    char manifest_path[128];
    snprintf(manifest_path, sizeof(manifest_path), "imgfs/_%s/_%s_E1", music_id, music_id);
    const char *iidx_1_manifest = mxmlGetText(mxmlFindPath(manifest, manifest_path), NULL);
    char *endptr = NULL;
    int file_offset = strtol(iidx_1_manifest, &endptr, 10);
    file_length = strtol(endptr, NULL, 10);
    int iidx_1_offset = manifest_end + file_offset;
    mxmlDelete(manifest);

    // read the iidx_1 file from the ifs.
    file = fopen(filename, "rb");
    if (file == NULL)
      return 1;
    file_buffer = (uint8_t*) malloc(file_length);
    fseek(file, iidx_1_offset, SEEK_SET);
    fread(file_buffer, sizeof(uint8_t), file_length, file);
    fclose(file);
  }

  *out_file_buffer = file_buffer;
  *out_file_length = file_length;
  return 0;
}

int read_chart(const char *music_id, iidx_1_chart chart)
{
  if (music_id == NULL || (uint32_t)chart >= IIDX_1_MAX_CHART_COUNT)
    return -1;

  // read the iidx_1 file.
  uint8_t *file_buffer = NULL;
  uint32_t file_length = 0;
  if (load_iidx_1(music_id, &file_buffer, &file_length))
    return -1;

  // get the note counts from the file.
  int ret = iidx_1_get_note_count(file_buffer, file_length, chart);

  free(file_buffer);
  return ret;
}

int read_charts(const char *music_id, iidx_1_note_counts *out_note_counts)
{
  if (music_id == NULL || out_note_counts == NULL)
    return -1;

  // read the iidx_1 file.
  uint8_t *file_buffer = NULL;
  uint32_t file_length = 0;
  if (load_iidx_1(music_id, &file_buffer, &file_length))
    return -1;

  // get the note counts from the file.
  int ret = iidx_1_get_note_counts(file_buffer, file_length, out_note_counts);

  free(file_buffer);
  return ret;
}
