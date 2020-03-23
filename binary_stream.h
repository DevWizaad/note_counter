#ifndef BINARY_STREAM_H_
#define BINARY_STREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct binary_stream_s binary_stream;

typedef enum
{
  LITTLE_ENDIAN,
  BIG_ENDIAN
} endianness;

// create/destroy functions.
binary_stream *bs_open(void *data, uint32_t size);
binary_stream *bs_duplicate(binary_stream *bs);
void bs_close(binary_stream *bs);

// getters.
uint32_t bs_get_offset(const binary_stream *bs);
endianness bs_get_endianness(const binary_stream *bs);
int bs_at_end(const binary_stream *bs);

// setters.
void bs_set_offset(binary_stream *bs, uint32_t offset);
void bs_set_endianness(binary_stream *bs, endianness endian);
void bs_add_offset(binary_stream *bs, uint32_t offset);

// read from the stream.
uint8_t bs_peek_u8(binary_stream *bs);
uint8_t bs_read_u8(binary_stream *bs);
uint16_t bs_read_u16(binary_stream *bs);
uint32_t bs_read_u32(binary_stream *bs);
uint64_t bs_read_u64(binary_stream *bs);
uint32_t bs_read_bytes(binary_stream *bs, void *out, uint32_t size); // returns the number of bytes read.

// alignment.
void bs_realign32(binary_stream *bs);

inline uint16_t byte_swap16(uint16_t bytes)
{
  return _byteswap_ushort(bytes);
}

inline uint32_t byte_swap32(uint32_t bytes)
{
  return _byteswap_ulong(bytes);
}

inline uint64_t byte_swap64(uint64_t bytes)
{
  return _byteswap_uint64(bytes);
}

#ifdef __cplusplus
}
#endif

#endif // BINARY_STREAM_H_
