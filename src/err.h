#ifndef ERR_H
#define ERR_H

#include "./lib.h"

typedef enum
{
  ERR_OK = 0,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,

  ERR_BYTECODE_EOF,
  ERR_ILLEGAL_TYPE,
  ERR_ILLEGAL_JUMP,
  ERR_ILLEGAL_INSTRUCTION,

  ERR_INTEGER_OVERFLOW,
  ERR_INTEGER_UNDERFLOW,

  NUMBER_OF_ERRORS,
} err_t;

const char *err_as_cstr(err_t);
char *err_generate(err_t err, buffer_t *buffer);

#endif
