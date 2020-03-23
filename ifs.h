/*
  This is a conversion of ifstools' loader from Python to C.
  View the original source here: https://github.com/mon/ifstools
 */
#ifndef IFS_H_
#define IFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <mxml/mxml.h>

typedef enum
{
  IFS_NO_ERROR = 0,
  IFS_INVALID_PARAM = 1,
  IFS_FILE_FAILED = 2,
  IFS_MEM_FAILED = 3,
  IFS_INVALID_FILE = 4,
  IFS_MANIFEST_PARSE_ERROR = 5,
} ifs_error;

ifs_error ifs_extract(const char *path, mxml_node_t **out_manifest, uint32_t *out_manifest_end);

#ifdef __cplusplus
}
#endif

#endif // IFS_H_
