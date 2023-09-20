/* data.c
 * Created: 2023-09-19
 * Author: Aryadev Chavali
 * Description: Internal data representation
 */

#include "./data.h"

#include <assert.h>
#include <string.h>

data_t data_nil(void)
{
  return (data_t){0};
}

data_t data_bool(bool b)
{
  return (data_t){.type = DATA_BOOLEAN, .payload = {.as_bool = b}};
}

data_t data_char(char c)
{
  return (data_t){.type = DATA_CHARACTER, .payload = {.as_char = c}};
}

data_t data_int(i64 i)
{
  return (data_t){.type = DATA_INT, .payload = {.as_int = i}};
}

data_t data_uint(u64 u)
{
  return (data_t){.type = DATA_UINT, .payload = {.as_uint = u}};
}

data_t data_double(double d)
{
  return (data_t){.type = DATA_DOUBLE, .payload = {.as_double = d}};
}

size_t data_write(data_t datum, byte *bytes)
{
  if (!bytes)
  {
    // Return bytes written if datum was written
    switch (datum.type)
    {
    case DATA_NIL:
    case DATA_BOOLEAN:
    case DATA_CHARACTER:
      return 2;
    case DATA_INT:
    case DATA_UINT:
    case DATA_DOUBLE:
      return 9;
    case NUMBER_OF_DATATYPES:
    default:
      return 0;
    }
  }
  bytes[0] = datum.type;
  switch (datum.type)
  {
  case DATA_NIL:
    bytes[1] = 0;
    return 2;
    break;
  case DATA_BOOLEAN:
  case DATA_CHARACTER:
    bytes[1] = datum.payload.as_char;
    return 2;
    break;
  case DATA_INT:
  case DATA_UINT:
  case DATA_DOUBLE:
#if IS_BIG_ENDIAN
    for (size_t i = 1, shift = 56; i < 9; ++i, shift -= 8)
      bytes[i] = ((datum.payload.as_uint >> shift) & 0xff);
#else
    for (size_t i = 1, shift = 0; i < 9; ++i, shift += 8)
      bytes[i] = ((datum.payload.as_uint >> shift) & 0xff);
#endif
    return 9;
    break;
  case NUMBER_OF_DATATYPES:
  default:
    return 0;
  }
}

void data_print(data_t d)
{
  switch (d.type)
  {
  case DATA_NIL:
    printf("NIL");
    break;
  case DATA_BOOLEAN:
    printf("%s", d.payload.as_bool ? "True" : "False");
    break;
  case DATA_CHARACTER:
    printf("`%c`", d.payload.as_char);
    break;
  case DATA_INT:
    printf("%" PRId64 "d", d.payload.as_int);
    break;
  case DATA_UINT:
    printf("%" PRIu64 "u", d.payload.as_uint);
    break;
  case DATA_DOUBLE:
    printf("%ff", d.payload.as_double);
    break;
  case NUMBER_OF_DATATYPES:
  default:
    return;
  }
}

uint64_t read_u64_from_bytes(byte *bytes)
{
  u64 acc = 0;
#if IS_BIG_ENDIAN
  for (size_t i = 0, shift = 56; i < 8; ++i, shift -= 8)
    acc += (((u64)bytes[i]) << shift);
#else
  for (size_t i = 0, shift = 0; i < 8; ++i, shift += 8)
    acc += (((u64)bytes[i]) << shift);
#endif
  return acc;
}

data_t data_read(data_type_t tag, byte *bytes)
{
  switch (tag)
  {
  case DATA_NIL:
    return data_nil();
  case DATA_BOOLEAN:
    return data_bool(bytes[1]);
  case DATA_CHARACTER:
    return data_char(bytes[1]);
  case DATA_UINT: {
    u64 u = read_u64_from_bytes(bytes);
    return data_uint(u);
  }
  case DATA_INT: {
    u64 u = read_u64_from_bytes(bytes);
    i64 i = 0;
    memcpy(&i, &u, sizeof(i));
    return data_int(i);
  }
  case DATA_DOUBLE: {
    u64 u    = read_u64_from_bytes(bytes);
    double d = 0;
    memcpy(&d, &u, sizeof(d));
    return data_double(d);
  }
  case NUMBER_OF_DATATYPES:
  default:
    return data_nil();
  }
}

static_assert(DATA_DOUBLE - DATA_INT == 2,
              "data_numeric_cast: Doesn't cover all numeric types");

data_t data_numeric_cast(data_t d, data_type_t t)
{
  if (d.type < DATA_INT || t < DATA_INT)
    return data_nil();

  if (t == DATA_INT)
  {
    if (d.type == DATA_INT || d.type == DATA_UINT)
      return data_int(d.payload.as_int);
    else
      return data_int((i64)d.payload.as_double);
  }
  else if (t == DATA_UINT)
  {
    if (d.type == DATA_INT || d.type == DATA_UINT)
      return data_uint(d.payload.as_uint);
    else
      return data_uint((u64)d.payload.as_double);
  }
  else if (t == DATA_DOUBLE)
  {
    if (d.type == DATA_INT)
      return data_double((double)d.payload.as_int);
    else if (d.type == DATA_UINT)
      return data_double((double)d.payload.as_uint);
    else
      return d;
  }

  return data_nil();
}
