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
  case ERR_LABEL_OVERFLOW:
    return "ERR_LABEL_OVERFLOW";
    break;
  case ERR_ILLEGAL_JUMP:
    return "ERR_ILLEGAL_JUMP";
    break;
  case ERR_ILLEGAL_INSTRUCTION:
    return "ERR_ILLEGAL_INSTRUCTION";
    break;
  case ERR_INTEGER_OVERFLOW:
    return "ERR_INTEGER_OVERFLOW";
    break;
  case ERR_INTEGER_UNDERFLOW:
    return "ERR_INTEGER_UNDERFLOW";
    break;
  default:
    return "";
    break;
  }
}
