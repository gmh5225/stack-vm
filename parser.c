/* parser.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Parser for assembly
 */

#include "./parser.h"

#include <string.h>

presult_t parse_line(buffer_t *buf)
{
  // Assume we are at an operand?
  presult_t res  = {0};
  size_t segment = strcspn(buf->data, " ");
  if (strncmp(buf->data, "push", segment) == 0)
  {
    res.operands          = calloc(1, sizeof(res.operands[0]));
    res.operands_size     = 1;
    res.operands->opcode  = OP_PUSH;
    res.operands->operand = atoi(buf->data + segment);
    res.ok                = true;
  }
  return res;
}

presult_t parse_buffer(buffer_t *buf)
{
  while (!buffer_is_end(*buf))
  {
    parse_line(buf);
    buffer_seek_next(buf);
  }
}
