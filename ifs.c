#include "ifs.h"

#include <stdio.h>

#include "binary_stream.h"
#include "kbinxml.h"

#define SIGNATURE 0x6CAD8F89

typedef struct
{
  uint32_t signature;
  uint16_t version;
  uint16_t not_version;
  uint32_t time;
  uint32_t tree_size;
  uint32_t manifest_end;
} ifs_header;

ifs_error ifs_extract_manifest(const char *path, mxml_node_t **out_manifest, uint32_t *out_manifest_end)
{
  if (path == NULL || out_manifest == NULL || out_manifest_end == NULL)
    return IFS_INVALID_PARAM;

  // Read the entire file into a buffer.
  FILE *file = fopen(path, "rb");
  if (file == NULL)
    return IFS_FILE_FAILED;

  // read in the header.
  ifs_header header;
  size_t elements_read = fread(&header, sizeof(header), 1, file);

  // swap endianness as the header is stored in big endian.
  header.signature = byte_swap32(header.signature);
  header.version = byte_swap16(header.version);
  header.not_version = byte_swap16(header.not_version);
  header.time = byte_swap32(header.time);
  header.tree_size = byte_swap32(header.tree_size);
  header.manifest_end = byte_swap32(header.manifest_end);

  // verify header.
  if (elements_read < 1 ||
      header.signature != SIGNATURE ||
      (header.version ^ header.not_version) != 0xffff)
  {
    fclose(file);
    return IFS_INVALID_FILE;
  }

  // skip manifest md5.
  char md5[16];
  if (header.version > 1)
    fread(md5, sizeof(char), sizeof(md5), file);
  
  // allocate a buffer for the manifest.
  uint32_t manifest_size = header.manifest_end - ftell(file);
  uint8_t *manifest_buffer = (uint8_t*) malloc(manifest_size);
  if (manifest_buffer == NULL)
  {
    fclose(file);
    return IFS_MEM_FAILED;
  }

  // read in the manifest.
  elements_read = fread(manifest_buffer, sizeof(uint8_t), manifest_size, file);
  if (elements_read < manifest_size)
  {
    free(manifest_buffer);
    fclose(file);
    return IFS_INVALID_FILE;
  }

  // convert from binary to xml.
  mxml_node_t *manifest = kbinxml_from_binary(manifest_buffer, manifest_size);

  // cleanup, check for manifest parse error.
  free(manifest_buffer);
  fclose(file);
  if (manifest == NULL)
    return IFS_MANIFEST_PARSE_ERROR;

  // set return params.
  *out_manifest = manifest;
  *out_manifest_end = header.manifest_end;

  return IFS_NO_ERROR;
}
