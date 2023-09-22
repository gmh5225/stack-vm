#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

#include "./lib.h"

typedef enum DataType
{
  DATA_NIL = 0,
  DATA_BOOLEAN,
  DATA_CHARACTER,
  DATA_INT,
  DATA_UINT,
  DATA_FLOAT,

  NUMBER_OF_DATATYPES
} data_type_t;

typedef enum
{
  // The "immediate types"
  TAG_INT       = 0x0, // 0b0000
  TAG_UINT      = 0x2, // 0b0010
  TAG_CHARACTER = 0x4, // 0b0100
  TAG_BOOLEAN   = 0x6, // 0b0110
  TAG_FLOAT     = 0x8, // 0b1000
  TAG_NIL       = 0xa, // 0b1010

  // Any "allocated" types
  // TODO: Create some allocated types
} tags_t;

typedef enum
{
  MASK_INT       = 15,
  MASK_UINT      = 15,
  MASK_CHARACTER = 15,
  MASK_BOOLEAN   = 15,
  MASK_NIL       = 15,
  MASK_FLOAT     = 15,
} mask_t;

typedef enum
{
  BITS_INT       = 4,
  BITS_UINT      = 4,
  BITS_CHARACTER = 4,
  BITS_BOOLEAN   = 4,
  BITS_NIL       = 4,
  BITS_FLOAT     = 4,
} bits_t;

// Type level separation of tagged bits
struct Data;
typedef struct Data data_t;

// Macro to tag some bits using a mask
#define TAG(BITS, MASK, TAG) (((BITS) & ~(MASK)) | (TAG))

// Macro to check if some set of bits are tagged
#define TAGGED(BITS, MASK, TAG) (((BITS) & (MASK)) == (TAG))

// Bounds of a 60 bit (u)int
// No sign bit => 60 bits of space
#define UINT60_MAX ((1LU << 60) - 1)
#define INT60_MAX  ((1LL << 59) - 1)
#define INT60_MIN  (-(1LL << 59))

/* Constructors */
data_t *data_nil(void);
data_t *data_bool(bool b);
data_t *data_char(char c);
data_t *data_float(float f);
data_t *data_int(i64 i);
data_t *data_uint(u64 u);

/* Destructors */
bool data_as_bool(data_t *);
char data_as_char(data_t *);
float data_as_float(data_t *);
i64 data_as_int(data_t *);
u64 data_as_uint(data_t *);

void data_print(data_t *, FILE *);

/* Type programming */
data_type_t data_type(data_t *);
bool data_type_is_numeric(data_type_t);
data_t *data_numeric_cast(data_t *, data_type_t);
void data_numerics_promote_on_float(data_t **, data_type_t *, data_t **,
                                    data_type_t *);

/* Dealing with bytecode */

size_t data_type_bytecode_size(data_type_t);

// Writes datum in bytes.  Assume bytes has enough space for datum.
size_t data_write(data_t *, byte *);

// Read bytes as data with a certain type.  We assume you've asserted
// that the buffer *actually has* enough bytes to do this operation
// (use data_type_bytecode_size to figure it out).
data_t *data_read(data_type_t, byte *);

#endif
