#ifndef PARSER_H
#define PARSER_H

#include "./lib.h"
#include "./op.h"

#include <stdbool.h>

typedef enum
{
  PERR_OK = 0,

  PERR_EXPECTED_OPERAND,
  PERR_EXPECTED_LABEL,
  PERR_UNEXPECTED_OPERAND,

  PERR_UNKNOWN_LABEL,
  PERR_ILLEGAL_OPERATOR,
  PERR_ILLEGAL_INST_ADDRESS,
  PERR_EOF,

  NUMBER_OF_PERRORS,
} perr_t;

const char *perr_as_cstr(perr_t err);
char *perr_generate(perr_t err, buffer_t *buf);

#define PARSER_LABEL_ACCEPTED_CHARS \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"

typedef struct
{
  size_t buffer_cursor;

  enum
  {
    PRES_IMMEDIATE,
    PRES_JUMP_RELATIVE,
    PRES_LABEL,
    PRES_JUMP_LABEL,
  } type;

  union
  {
    op_t immediate;
    char *label_name;
    i64 relative_jump_operand;
  };
} pres_t;

perr_t parse_i64(buffer_t *buf, data_t **ret);
perr_t parse_u64(buffer_t *buf, data_t **ret);

perr_t parse_line(buffer_t *, pres_t *);
perr_t process_presults(pres_t *, size_t, buffer_t *, darr_t *);

perr_t parse_buffer(buffer_t *, op_t **, u64 *);

#endif
