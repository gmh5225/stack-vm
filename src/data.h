#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

#include "./lib.h"

typedef struct
{
  enum DataType
  {
    DATA_NIL = 0,
    DATA_BOOLEAN,
    DATA_CHARACTER,
    DATA_INT,
    DATA_UINT,
    DATA_DOUBLE,

    NUMBER_OF_DATATYPES
  } type;

  union
  {
    bool as_bool;
    char as_char;
    i64 as_int;
    u64 as_uint;
    double as_double;
  } payload;
} data_t;

typedef enum DataType data_type_t;

data_t data_nil(void);
data_t data_bool(bool b);
data_t data_char(char c);
data_t data_int(i64 i);
data_t data_uint(u64 u);
data_t data_double(double d);

void data_print(data_t);

data_t data_numeric_cast(data_t, data_type_t);

// Writes datum in bytes.  Assume bytes has enough space for datum.
size_t data_write(data_t, byte *);

// Read bytes as data with a certain type.  We assume you've asserted
// that *actually have* enough bytes to do this operation (use
// data_write with NULL to figure it out).
data_t data_read(data_type_t, byte *);

#endif
