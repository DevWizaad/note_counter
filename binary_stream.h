#ifndef BINARY_STREAM_H_
#define BINARY_STREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#ifndef BINARY_STREAM_ASSERT
  #define BINARY_STREAM_ASSERT(x) assert(x)
#endif

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
float bs_read_f32(binary_stream *bs);
double bs_read_f64(binary_stream *bs);
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

#ifdef BINARY_STREAM_DEFINITIONS

#include <assert.h>
#include <string.h>

struct binary_stream_s
{
  uint8_t *data;
  uint32_t size;

  uint32_t offset;
  endianness endian;
};

binary_stream *bs_open(void *data, uint32_t size)
{
  BINARY_STREAM_ASSERT(data != NULL && size > 0);
  
  binary_stream *ret = (binary_stream*) malloc(sizeof(binary_stream));
  BINARY_STREAM_ASSERT(ret);
  ret->data = (uint8_t*) data;
  ret->size = size;
  ret->offset = 0;
  ret->endian = LITTLE_ENDIAN;

  return ret;
}

binary_stream *bs_duplicate(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  
  binary_stream *ret = (binary_stream*) malloc(sizeof(binary_stream));
  ret->data = bs->data;
  ret->size = bs->size;
  ret->offset = bs->offset;
  ret->endian = bs->endian;

  return ret;
}

void bs_close(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  free(bs);
}

uint32_t bs_get_offset(const binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  return bs->offset;
}

endianness bs_get_endianness(const binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  return bs->endian;
}

int bs_at_end(const binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  return bs->offset >= bs->size;
}

void bs_set_offset(binary_stream *bs, uint32_t offset)
{
  BINARY_STREAM_ASSERT(bs);
  bs->offset = min(offset, bs->size);
}

void bs_set_endianness(binary_stream *bs, endianness endian)
{
  BINARY_STREAM_ASSERT(bs);
  bs->endian = endian;
}

void bs_add_offset(binary_stream *bs, uint32_t offset)
{
  BINARY_STREAM_ASSERT(bs);
  bs->offset = min(bs->offset + offset, bs->size);
}

uint8_t bs_peek_u8(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(bs->offset + 1 <= bs->size);

  return bs->data[bs->offset];
}

uint8_t bs_read_u8(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(bs->offset + 1 <= bs->size);

  uint8_t ret = bs->data[bs->offset];
  bs->offset += 1;
  return ret;
}

uint16_t bs_read_u16(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(bs->offset + 2 <= bs->size);

  uint16_t ret = *(uint16_t*) (bs->data + bs->offset);
  bs->offset += 2;
  return bs->endian == BIG_ENDIAN ? byte_swap16(ret) : ret;
}

uint32_t bs_read_u32(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(bs->offset + 4 <= bs->size);

  uint32_t ret = *(uint32_t*) (bs->data + bs->offset);
  bs->offset += 4;
  return bs->endian == BIG_ENDIAN ? byte_swap32(ret) : ret;
}

uint64_t bs_read_u64(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(bs->offset + 8 <= bs->size);

  uint64_t ret = *(uint64_t*) (bs->data + bs->offset);
  bs->offset += 8;
  return bs->endian == BIG_ENDIAN ? byte_swap64(ret) : ret;
}

float bs_read_f32(binary_stream *bs)
{
  uint32_t val = bs_read_u32(bs);
  return *(float*) &val;
}

double bs_read_f64(binary_stream *bs)
{
  uint64_t val = bs_read_u64(bs);
  return *(double*) &val;
}

uint32_t bs_read_bytes(binary_stream *bs, void *out, uint32_t size)
{
  BINARY_STREAM_ASSERT(bs);
  BINARY_STREAM_ASSERT(out && size > 0);

  size = min(size, bs->size - bs->offset);
  memcpy(out, bs->data + bs->offset, size);
  bs->offset += size;

  return size;
}

void bs_realign32(binary_stream *bs)
{
  BINARY_STREAM_ASSERT(bs);

  uint32_t mod = bs->offset % 4;
  switch (mod)
  {
    case 0:
      break;
    case 1:
      bs->offset += 3;
      break;
    case 2:
      bs->offset += 2;
      break;
    case 3:
      bs->offset += 1;
      break;
  }
}

#endif // BINARY_STREAM_DEFINITIONS

#ifdef __cplusplus
}
#endif

#endif // BINARY_STREAM_H_
