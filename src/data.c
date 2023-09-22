/* data.c
 * Created: 2023-09-19
 * Author: Aryadev Chavali
 * Description: Internal data representation
 */

#include "./data.h"
#include "./lib.h"

#include <assert.h>
#include <float.h>
#include <string.h>

data_t *data_nil(void)
{
  return (data_t *)TAG(0LU, MASK_NIL, TAG_NIL);
}

data_t *data_bool(bool b)
{
  // Copy bits
  word w = b;
  // Reserve space for tag
  w <<= BITS_BOOLEAN;
  return (data_t *)TAG((word)w, MASK_BOOLEAN, TAG_BOOLEAN);
}

data_t *data_char(char c)
{
  // Copy bits
  word bits = 0;
  memcpy(&bits, &c, sizeof(c));
  // Reserve space for tag
  bits <<= BITS_CHARACTER;
  return (data_t *)TAG(bits, MASK_CHARACTER, TAG_CHARACTER);
}

data_t *data_float(float f)
{
  word bits = 0;
  memcpy(&bits, &f, sizeof(f));
  bits <<= BITS_FLOAT;
  return (data_t *)TAG(bits, MASK_FLOAT, TAG_FLOAT);
}

data_t *data_int(i64 i)
{
  assert(i <= INT60_MAX && i >= INT60_MIN && "data_int: i is not 60 bits");
  return (data_t *)TAG(i << BITS_INT, MASK_INT, TAG_INT);
}

data_t *data_uint(u64 u)
{
  assert(u <= UINT60_MAX && "data_uint: u is not 60 bits");
  return (data_t *)TAG(u << BITS_UINT, MASK_UINT, TAG_UINT);
}

bool data_as_bool(data_t *d)
{
  return ((word)d) >> BITS_BOOLEAN;
}

char data_as_char(data_t *d)
{
  return ((word)d) >> BITS_CHARACTER;
}

float data_as_float(data_t *d)
{
  return ((word)d) >> BITS_FLOAT;
}

i64 data_as_int(data_t *d)
{
  return (((i64)d) >> BITS_INT);
}

u64 data_as_uint(data_t *d)
{
  return (((word)d) >> BITS_UINT);
}

data_type_t data_type(data_t *d)
{
  if (TAGGED((word)d, MASK_NIL, TAG_NIL))
    return DATA_NIL;
  else if (TAGGED((word)d, MASK_BOOLEAN, TAG_BOOLEAN))
    return DATA_BOOLEAN;
  else if (TAGGED((word)d, MASK_CHARACTER, TAG_CHARACTER))
    return DATA_CHARACTER;
  else if (TAGGED((word)d, MASK_INT, TAG_INT))
    return DATA_INT;
  else if (TAGGED((word)d, MASK_UINT, TAG_UINT))
    return DATA_UINT;
  else if (TAGGED((word)d, MASK_FLOAT, TAG_FLOAT))
    return DATA_FLOAT;
  assert(false && "data_type: d is not tagged?!");
  return DATA_NIL;
}

void data_print(data_t *d, FILE *fp)
{
  data_type_t type = data_type(d);
  switch (type)
  {
  case DATA_NIL:
    fprintf(fp, "NIL");
    break;
  case DATA_BOOLEAN:
    fprintf(fp, "bool(%s)", data_as_bool(d) ? "True" : "False");
    break;
  case DATA_CHARACTER:
    fprintf(fp, "char(%c)", data_as_char(d));
    break;
  case DATA_INT:
    fprintf(fp, "int(%" PRId64 ")", data_as_int(d));
    break;
  case DATA_UINT:
    fprintf(fp, "uint(%" PRIu64 ")", data_as_uint(d));
    break;
  case DATA_FLOAT:
    fprintf(fp, "float(%f)", data_as_float(d));
    break;
  case NUMBER_OF_DATATYPES:
  default:
    fprintf(fp, "<UNKNOWN:%" PRIu64 ">", (word)d);
    break;
  }
}

data_t *data_numeric_cast(data_t *d, data_type_t t)
{
  data_type_t type = data_type(d);
  if (t < DATA_INT || type < DATA_INT)
    return data_nil();
  else if (type == t)
    return d;

  // TODO: Make this better?
  if (t == DATA_INT)
  {
    // d must be either uint or float
    if (type == DATA_UINT)
    {
      u64 u = data_as_uint(d);
      if (u > INT64_MAX)
        return data_nil();
      else
        return data_int(u);
    }
    else if (type == DATA_FLOAT)
      // Must truncate :(
      return data_int(data_as_float(d));
  }
  else if (t == DATA_UINT)
  {
    // d must be either an int or float
    if (type == DATA_INT)
      return data_uint(data_as_int(d));
    else if (type == DATA_FLOAT)
      // Must truncate :(
      return data_uint(data_as_float(d));
  }
  else if (t == DATA_FLOAT)
  {
    // d must be either an int or uint, both will truncate :(
    if (type == DATA_INT)
      return data_float(data_as_int(d));
    else if (type == DATA_UINT)
      // Must truncate :(
      return data_float(data_as_uint(d));
  }
  // Should never happen
  assert(false && "data_numeric_cast: Supposedly unreachable case occured!");
  return data_nil();
}

size_t data_type_bytecode_size(data_type_t type)
{
  switch (type)
  {
  case DATA_NIL:
    return 1;
  case DATA_BOOLEAN:
  case DATA_CHARACTER:
    return 2;
  case DATA_FLOAT:
    return sizeof(float) + 1;
  case DATA_INT:
  case DATA_UINT:
    return sizeof(i64) + 1;
  case NUMBER_OF_DATATYPES:
  default:
    return 0;
  }
}

size_t data_write(data_t *d, byte *bytes)
{
  data_type_t type = data_type(d);
  bytes[0]         = type;
  switch (type)
  {
  case DATA_NIL:
    // Just one byte for "being a nil"
    return 1;
  case DATA_BOOLEAN:
    bytes[1] = data_as_bool(d);
    return 2;
  case DATA_CHARACTER:
    bytes[1] = data_as_char(d);
    return 2;
  case DATA_FLOAT: {
    float f = data_as_float(d);
    memcpy(bytes + 1, &f, sizeof(f));
    return sizeof(f) + 1;
  }
  case DATA_INT: {
    i64 i = data_as_int(d);
    memcpy(bytes + 1, &i, sizeof(i));
    return sizeof(i) + 1;
  }
  case DATA_UINT: {
    u64 i = data_as_uint(d);
    memcpy(bytes + 1, &i, sizeof(i));
    return sizeof(i) + 1;
  }
  case NUMBER_OF_DATATYPES:
  default:
    assert(false && "data_write: Type of data is not valid");
    return 0;
  }
}

data_t *data_read(data_type_t type, byte *bytes)
{
  switch (type)
  {
  case DATA_NIL:
    // Don't need to read that really lol
    return data_nil();
  case DATA_BOOLEAN:
    return data_bool(bytes[0]);
  case DATA_CHARACTER:
    return data_char(bytes[0]);
  case DATA_FLOAT: {
    float f = 0;
    memcpy(&f, bytes, sizeof(f));
    return data_float(f);
  }
  case DATA_INT: {
    i64 i = 0;
    memcpy(&i, bytes, sizeof(i));
    return data_int(i);
  }
  case DATA_UINT: {
    u64 u = 0;
    memcpy(&u, bytes, sizeof(u));
    return data_uint(u);
  }
  case NUMBER_OF_DATATYPES:
  default:
    // unreachable
    assert(false && "data_read: Type is not valid");
    return data_nil();
  }
}
