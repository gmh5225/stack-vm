/* data.c
 * Created: 2023-09-19
 * Author: Aryadev Chavali
 * Description: Internal data representation
 */

#include "./data.h"

#include <assert.h>
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
