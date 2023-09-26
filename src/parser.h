#ifndef PARSER_H
#define PARSER_H

#include "./lexer.h"
#include "./lib.h"
#include "./op.h"

#include <stdbool.h>

typedef enum
{
  PERR_OK = 0,
  PERR_CHAR_UNDERFLOW,
  PERR_CHAR_OVERFLOW,
  PERR_INTEGER_UNDERFLOW,
  PERR_INTEGER_OVERFLOW,
  PERR_UINTEGER_OVERFLOW,
  PERR_FLOAT_UNDERFLOW,
  PERR_FLOAT_OVERFLOW,
  PERR_UNEXPECTED_OPERAND,

  PERR_EXPECTED_NIL,
  PERR_EXPECTED_BOOL,
  PERR_EXPECTED_CHAR,
  PERR_EXPECTED_INTEGER,
  PERR_EXPECTED_UINTEGER,
  PERR_EXPECTED_FLOAT,
  PERR_EXPECTED_NUMBER,
  PERR_EXPECTED_OPERAND,

  PERR_EXPECTED_LABEL,

  PERR_UNKNOWN_LABEL,
  PERR_ILLEGAL_OPERATOR,
  PERR_ILLEGAL_INST_ADDRESS,
  PERR_EOF,

  NUMBER_OF_PERRORS,
} perr_t;

const char *perr_as_cstr(perr_t err);
char *perr_generate(perr_t err, stream_t *stream);

typedef struct
{
  size_t stream_cursor;

  enum
  {
    PRES_IMMEDIATE,
    PRES_JUMP_RELATIVE,
    PRES_JUMP_LABEL,
    PRES_LABEL,
    PRES_IPTR,
  } type;

  union
  {
    op_t immediate;
    char *label_name;
    data_t *operand;
  };
} pres_t;

perr_t parse_nil(stream_t *, data_t **);
perr_t parse_bool(stream_t *, data_t **);
perr_t parse_char(stream_t *, data_t **);

perr_t parse_i64(stream_t *, data_t **);
perr_t parse_u64(stream_t *, data_t **);
perr_t parse_float(stream_t *, data_t **);
perr_t parse_number(stream_t *, data_t **);

perr_t parse_push(stream_t *, pres_t *);
perr_t parse_dup(stream_t *, pres_t *);
perr_t parse_label(stream_t *, pres_t *);
perr_t parse_jmp(stream_t *, pres_t *);

perr_t parse_line(stream_t *, pres_t *);
perr_t process_presults(pres_t *, size_t, stream_t *, darr_t *);
perr_t parse_stream(stream_t *, op_t **, u64 *);

#endif
