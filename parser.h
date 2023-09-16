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

presult_t parse_line(buffer_t *);
presult_t parse_buffer(buffer_t *);

#endif
