#include "kbinxml.h"

#include "binary_stream.h"

#define SIGNATURE 0xA0
#define SIG_COMPRESSED 0x42
#define SIG_UNCOMPRESSED 0x45
#define XML_TYPE_ARRAY_BIT 0x40

typedef struct
{
  const char *name;
  int8_t count;
  char type;
} xml_format;

static xml_format xml_formats[] = {
  {NULL},
  {"void"},
  {"s8",  1,  'b'},
  {"u8",  1,  'B'},
  {"s16", 1,  'h'},
  {"u16", 1,  'H'},
  {"s32", 1,  'i'},
  {"u32", 1,  'I'},
  {"s64", 1,  'q'},
  {"u64", 1,  'Q'},
  {"bin", -1, 'B'},
  {"str", -1, 'B'},
  {"ip4", 1,  'P'},
  {"time", 1, 'I'},
  {"float", 1, 'f'},
  {"double", 1, 'd'},
  {"2s8",  2, 'b'},
  {"2u8",  2, 'B'},
  {"2s16", 2, 'h'},
  {"2u16", 2, 'H'},
  {"2s32", 2, 'i'},
  {"2u32", 2, 'I'},
  {"2s64", 2, 'q'},
  {"2u64", 2, 'Q'},
  {"2f",   2, 'f'},
  {"2d",   2, 'd'},
  {"3s8",  3, 'b'},
  {"3u8",  3, 'B'},
  {"3s16", 3, 'h'},
  {"3u16", 3, 'H'},
  {"3s32", 3, 'i'},
  {"3u32", 3, 'I'},
  {"3s64", 3, 'q'},
  {"3u64", 3, 'Q'},
  {"3f",   3, 'f'},
  {"3d",   3, 'd'},
  {"4s8",  4, 'b'},
  {"4u8",  4, 'B'},
  {"4s16", 4, 'h'},
  {"4u16", 4, 'H'},
  {"4s32", 4, 'i'},
  {"4u32", 4, 'I'},
  {"4s64", 4, 'q'},
  {"4u64", 4, 'Q'},
  {"4f",   4, 'f'},
  {"4d",   4, 'd'},
  {"attr"},
  {"array"},
  {"vs8", 16, 'b'},
  {"vu8", 16, 'B'},
  {"vs16", 8, 'h'},
  {"vu16", 8, 'H'},
  {"bool", 1, 'b'},
  {"2b",   2, 'b'},
  {"3b",   3, 'b'},
  {"4b",   4, 'b'},
  {"vb",  16, 'b'}
};

typedef enum
{
  XML_TYPE_NODE_START = 1,
  XML_TYPE_BINARY = 10,
  XML_TYPE_STRING = 11,
  XML_TYPE_ATTR = 46,
  XML_TYPE_NODE_END = 190,
  XML_TYPE_END_SECTION = 191
} xml_types;

typedef struct
{
  uint8_t signature;
  uint8_t compressed;
  uint8_t encoding_key;
  uint8_t not_encoding_key; // ~encoding_key
  uint32_t section_length; // length of the entire section
} kbinxml_header;

#define SIX_BITS 0x3f
static const char SIXBIT_CHARMAP[] = "0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

static char *unpack_sixbit(binary_stream *stream)
{
  // get the number of sixbit encoded characters.
  uint32_t length = bs_read_u8(stream);
  
  // allocate an array for the decoded characters.
  char *ret = (char*) calloc(length + 1, sizeof(char));
  char *ret_ptr = ret;

  // read in 24 bits at a time, matching up to exactly 4 encoded chars.
  // this is done to remove worrying about bit offsets between loop iterations.
  uint32_t bits;
  for (uint32_t chars_read = 0; chars_read < length; chars_read += 4)
  {
    bits = 0;

    if (length - chars_read >= 4)
    {
      bits |= (bs_read_u8(stream) << 16);
      bits |= (bs_read_u8(stream) << 8);
      bits |= bs_read_u8(stream);

      *(ret_ptr++) = SIXBIT_CHARMAP[bits >> 18];
      *(ret_ptr++) = SIXBIT_CHARMAP[(bits >> 12) & SIX_BITS];
      *(ret_ptr++) = SIXBIT_CHARMAP[(bits >> 6) & SIX_BITS];
      *(ret_ptr++) = SIXBIT_CHARMAP[bits & SIX_BITS];
    }
    else if (length - chars_read == 3)
    {
      bits |= (bs_read_u8(stream) << 16);
      bits |= (bs_read_u8(stream) << 8);
      bits |= bs_read_u8(stream);

      *(ret_ptr++) = SIXBIT_CHARMAP[bits >> 18];
      *(ret_ptr++) = SIXBIT_CHARMAP[(bits >> 12) & SIX_BITS];
      *(ret_ptr++) = SIXBIT_CHARMAP[(bits >> 6) & SIX_BITS];
    }
    else if (length - chars_read == 2)
    {
      bits |= (bs_read_u8(stream) << 16);
      bits |= (bs_read_u8(stream) << 8);

      *(ret_ptr++) = SIXBIT_CHARMAP[bits >> 18];
      *(ret_ptr++) = SIXBIT_CHARMAP[(bits >> 12) & SIX_BITS];
    }
    else
    {
      bits |= (bs_read_u8(stream) << 16);

      *(ret_ptr++) = SIXBIT_CHARMAP[bits >> 18];
    }
  }

  return ret;
}

static char *read_and_format_data(binary_stream *data_bs, const xml_format *format, uint32_t total_count)
{
  // try to calculate a text buffer size.
  uint32_t char_per_element = 1;

  // binary type gets serialized as hex, so 2 chars per byte.
  if (format == &xml_formats[XML_TYPE_BINARY])
    char_per_element = 2;

  if (format->count > 0)
  {
    // get the maximum number of characters needed for every data type (includes tailing whitespace).
    switch (format->type)
    {
      // byte.
      case 'b':
      case 'B':
        char_per_element = 5;
        break;
      // half int.
      case 'h':
      case 'H':
        char_per_element = 7;
        break;
      // int.
      case 'i':
      case 'I':
        char_per_element = 12;
        break;
      // quadword.
      case 'q':
      case 'Q':
        char_per_element = 21;
        break;
      // float/double. (idk about these so overcompensating)
      case 'f':
      case 'd':
        char_per_element = 64;
        break;
      // IPv4 address.
      case 'P':
        char_per_element = 16;
        break;
    }
  }

  // allocate our text buffer.  
  char *ret = (char*) calloc(char_per_element * total_count + 1, sizeof(char));

  // figure out how to format it.
  if (format == &xml_formats[XML_TYPE_STRING])
  {
    // string.
    bs_read_bytes(data_bs, ret, total_count);
  }
  else if (format == &xml_formats[XML_TYPE_BINARY])
  {
    // binary, print hex.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%x", bs_read_u8(data_bs));
  }
  else if (format->type == 'b')
  {
    // signed char.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%hhd ", bs_read_u8(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'B')
  {
    // unsigned char.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%hhu ", bs_read_u8(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'h')
  {
    // signed short.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%hd ", bs_read_u16(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'H')
  {
    // unsigned short.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%hu ", bs_read_u16(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'i')
  {
    // signed int.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%d ", bs_read_u32(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'I')
  {
    // unsigned int.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%u ", bs_read_u32(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'q')
  {
    // signed quad int.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%lld ", bs_read_u64(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'Q')
  {
    // unsigned quad int.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%llu ", bs_read_u64(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'f')
  {
    // float.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%.6f ", bs_read_f32(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'd')
  {
    // double.
    char *cur = ret;
    while (total_count--)
      cur += sprintf(cur, "%.6f ", bs_read_f64(data_bs));
    *(cur - 1) = 0;
  }
  else if (format->type == 'P')
  {
    // TODO: parsing IP addresses not currently enabled.
  }

  bs_realign32(data_bs);
  return ret;
};

mxml_node_t *kbinxml_from_binary(uint8_t *binary, uint32_t binary_length)
{
  if (binary == NULL || binary_length <= sizeof(kbinxml_header))
    return NULL;

  mxml_node_t *ret = NULL;

  // create a binary stream from our parameters.
  binary_stream *bs = bs_open(binary, binary_length);
  bs_set_endianness(bs, BIG_ENDIAN);

  // read the header from our stream.
  kbinxml_header header;
  header.signature = bs_read_u8(bs);
  header.compressed = bs_read_u8(bs);
  header.encoding_key = bs_read_u8(bs);
  header.not_encoding_key = bs_read_u8(bs);
  header.section_length = bs_read_u32(bs);

  // open another binary stream at the data section after the node.
  binary_stream *data_bs = bs_duplicate(bs);
  bs_set_offset(data_bs, header.section_length + sizeof(kbinxml_header));
  uint32_t data_size = bs_read_u32(data_bs);

  // verify the header is valid.
  if (header.signature == SIGNATURE &&
      (header.compressed == SIG_COMPRESSED || header.compressed == SIG_UNCOMPRESSED) &&
      (header.encoding_key ^ header.not_encoding_key) == 0xff &&
      binary_length >= header.section_length + sizeof(kbinxml_header))
  {
    ret = mxmlNewXML("1.0");
    mxml_node_t *node = ret;
    int compressed = (header.compressed == SIG_COMPRESSED);

    int done = 0;
    while (!done && node != NULL)
    {
      // Skip 0x0.
      while (bs_peek_u8(bs) == 0)
        bs_read_u8(bs);

      // read the node's xml_type.
      uint8_t xml_type = bs_read_u8(bs);
      int is_array = xml_type & XML_TYPE_ARRAY_BIT;
      xml_type &= ~XML_TYPE_ARRAY_BIT;

      // check for extra special xml types.
      if (xml_type == XML_TYPE_NODE_END)
      {
        // node's over, go back to parent.
        node = mxmlGetParent(node);
        continue;
      }
      else if (xml_type == XML_TYPE_END_SECTION)
      {
        // section's over, we're done.
        done = 1;
        continue;
      }

      // xml_type MUST be within our known range now.
      if (xml_type >= sizeof(xml_formats) / sizeof(*xml_formats))
      {
        mxmlDelete(ret);
        ret = NULL;
        goto end;
      }
      xml_format *node_format = &xml_formats[xml_type];
      
      // read the node name.
      char *name = NULL;
      if (compressed)
        name = unpack_sixbit(bs);
      else
      {
        uint8_t length = (bs_read_u8(bs) & ~0x40);
        name = (char*) calloc(length + 1, sizeof(char));
        bs_read_bytes(bs, name, length);
      }

      // handle attribute types.
      if (xml_type == XML_TYPE_ATTR)
      {
        // read the attribute data.
        uint32_t length = bs_read_u32(bs);
        char *attr_value = (char*) calloc(length + 1, sizeof(char));
        bs_read_bytes(bs, attr_value, length);
        bs_realign32(bs);
        mxmlElementSetAttr(node, name, attr_value);
        free(attr_value);
        free(name);
      }
      else
      {
        // make a new element.
        node = mxmlNewElement(node, name);
        free(name);
        name = NULL;
        if (xml_type == XML_TYPE_NODE_START)
          continue;

        // set the node's type attribute.
        mxmlElementSetAttr(node, "__type", node_format->name);

        // get the total number of elements for the node's text.
        uint32_t var_count = node_format->count == -1 ? bs_read_u32(data_bs) : node_format->count;
        uint32_t array_count = is_array ? bs_read_u32(data_bs) : 1;
        uint32_t total_count = var_count * array_count;

        // set the array node's count attribute.
        if (is_array)
        {
          array_count = bs_read_u32(data_bs);
          char num_buffer[16];
          sprintf(num_buffer, "%d", array_count);
          mxmlElementSetAttr(node, "__count", num_buffer);
        }

        // set the binary node's size attribute.
        if (xml_type == XML_TYPE_BINARY)
        {
          char num_buffer[16];
          sprintf(num_buffer, "%d", total_count);
          mxmlElementSetAttr(node, "__size", num_buffer);
        }

        // format and set the text for the node.
        char *data = read_and_format_data(data_bs, node_format, total_count);
        mxmlNewText(node, 0, data);
        free(data);
      }
    }
  }

end:
  bs_close(data_bs);
  bs_close(bs);

  return ret;
}
