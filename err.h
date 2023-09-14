#ifndef ERR_H
#define ERR_H

typedef enum
{
  ERR_OK = 0,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  ERR_LABEL_OVERFLOW,
  ERR_ILLEGAL_JUMP,
  ERR_ILLEGAL_INSTRUCTION,
  ERR_INTEGER_OVERFLOW,
  ERR_INTEGER_UNDERFLOW,
} err_t;

const char *err_as_cstr(err_t);

#endif
