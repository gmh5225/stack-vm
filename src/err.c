/* err.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Error implementation
 */

#include "./err.h"

const char *err_as_cstr(err_t e)
{
  switch (e)
  {
  case ERR_OK:
    return "ERR_OK";
    break;
  case ERR_STACK_OVERFLOW:
    return "ERR_STACK_OVERFLOW";
    break;
  case ERR_STACK_UNDERFLOW:
    return "ERR_STACK_UNDERFLOW";
    break;
  case ERR_BYTECODE_EOF:
    return "ERR_BYTECODE_EOF";
  case ERR_ILLEGAL_JUMP:
    return "ERR_ILLEGAL_JUMP";
  case ERR_ILLEGAL_TYPE:
    return "ERR_ILLEGAL_TYPE";
  case ERR_ILLEGAL_INSTRUCTION:
    return "ERR_ILLEGAL_INSTRUCTION";
    break;
  case ERR_INTEGER_OVERFLOW:
    return "ERR_INTEGER_OVERFLOW";
    break;
  case ERR_INTEGER_UNDERFLOW:
    return "ERR_INTEGER_UNDERFLOW";
    break;
  case NUMBER_OF_ERRORS:
  default:
    return "";
    break;
  }
}
