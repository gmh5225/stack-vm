#ifndef PARSER_H
#define PARSER_H

#include "./lib.h"
#include "./op.h"

#include <stdbool.h>

typedef struct
{
  bool ok;
  op_t *operands;
  size_t operands_size;
} presult_t;

typedef enum
{
  PERR_OK = 0,
  PERR_UNEXPECTED_OPERATOR,
  PERR_UNEXPECTED_OPERAND,
  PERR_EOF,
} perr_t;

const char *perr_as_cstr(perr_t err);
char *perr_generate(perr_t err, buffer_t *buf);

perr_t parse_line(buffer_t *, op_t *);
perr_t parse_buffer(buffer_t *, op_t **, u64 *);

#endif
