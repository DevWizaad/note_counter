/*
  This is a partial conversion of kbinxml from Python to C.
  View the original source here: https://github.com/mon/kbinxml
 */
#ifndef KBINXML_H_
#define KBINXML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <mxml/mxml.h>

mxml_node_t *kbinxml_from_binary(uint8_t *binary, uint32_t binary_length);

#ifdef __cplusplus
}
#endif

#endif // KBINXML_H_
