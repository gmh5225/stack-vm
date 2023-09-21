/* parser.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Parser for assembly
 */

#include "./parser.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

size_t get_size_i64(char *str, size_t max_size)
{
  if (!(str[0] == '-' || isdigit(str[0])))
    return 0;
  size_t i;
  for (i = 1; i < max_size; ++i)
  {
    if (isspace(str[i]) || str[i] == '\0')
      return i; // Stop parsing
    else if (!isdigit(str[i]))
      return 0; // Isn't a number
  }
  return i; // EOF
}

perr_t parse_i64(buffer_t *buf, i64 *ret)
{
  if (buffer_at_end(*buf) == BUFFER_PAST_END)
    return PERR_EOF;
  perr_t err           = PERR_OK;
  char *operand        = buf->data + buf->cur;
  size_t end_of_number = get_size_i64(operand, buf->available - buf->cur);
  if (end_of_number != 0)
    *ret = atoll(operand);
  else
    err = PERR_EXPECTED_OPERAND; // catch all
  buf->cur += end_of_number;
  return err;
}

perr_t parse_line(buffer_t *buf, pres_t *res)
{
  res->buffer_cursor = buf->cur;

  // Bring us to the first "token"
  buffer_seek_next(buf);

  // If at end, fail
  if (buffer_at_end(*buf) != BUFFER_OK)
    return PERR_EOF;

  // Find the end of operator
  size_t end_of_operator = strcspn(buf->data + buf->cur, " \n\0");
  if (memcmp(buf->data + buf->cur, "noop", 4) == 0)
  {
    buf->cur += end_of_operator;
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_NONE;
    goto NO_OPERAND;
  }
  else if (memcmp(buf->data + buf->cur, "halt", 4) == 0)
  {
    buf->cur += end_of_operator;
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_HALT;
    goto NO_OPERAND;
  }
  else if (memcmp(buf->data + buf->cur, "push", 4) == 0)
  {
    // Seek the operand
    buf->cur += end_of_operator;
    buffer_seek_next(buf);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_PUSH;
    return parse_i64(buf, &res->immediate.operand);
  }
  else if (memcmp(buf->data + buf->cur, "plus", 4) == 0)
  {
    buf->cur += end_of_operator;
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_PLUS;
    goto NO_OPERAND;
  }
  else if (memcmp(buf->data + buf->cur, "dup", 3) == 0)
  {
    buf->cur += end_of_operator;
    buffer_seek_next(buf);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_DUP;
    return parse_i64(buf, &res->immediate.operand);
  }
  else if (memcmp(buf->data + buf->cur, "print", 5) == 0)
  {
    buf->cur += end_of_operator;
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_PRINT;
    goto NO_OPERAND;
  }
  else if (memcmp(buf->data + buf->cur, "label", 5) == 0)
  {
    assert(false && "TODO: reimplement parse_line(label)");
  }
  else if (memcmp(buf->data + buf->cur, "jmp", 3) == 0)
  {
    assert(false && "TODO: reimplement parse_line(jmp)");
  }
  else
    return PERR_ILLEGAL_OPERATOR;
NO_OPERAND:
  buffer_seek_next(buf);
  if (buffer_at_end(*buf) == BUFFER_OK && buf->data[buf->cur] != '\n')
    return PERR_UNEXPECTED_OPERAND;
  return PERR_OK;
}

perr_t parse_buffer(buffer_t *buf, op_t **instructions,
                    u64 *instructions_parsed)
{
  if (buffer_at_end(*buf) != BUFFER_OK)
    return PERR_EOF;

  darr_t darr = {0};
  darr_init(&darr, DARR_INITAL_SIZE, sizeof(**instructions));
  size_t parsed;

  buffer_seek_nextline(buf);
  for (parsed = 0; buffer_at_end(*buf) == BUFFER_OK; ++parsed)
  {
    op_t parsed = {0};
    perr_t perr = parse_line(buf, &parsed);
    if (perr != PERR_OK)
    {
      free(darr.data);
      return perr;
    }
    DARR_APP(&darr, op_t, parsed);

    buffer_seek_nextline(buf);
  }
  darr_tighten(&darr);
  *instructions        = darr.data;
  *instructions_parsed = parsed;
  return PERR_OK;
}

const char *perr_as_cstr(perr_t err)
{
  switch (err)
  {
  case PERR_OK:
    return "PERR_OK";
  case PERR_EXPECTED_OPERAND:
    return "PERR_EXPECTED_OPERAND";
  case PERR_UNEXPECTED_OPERAND:
    return "PERR_UNEXPECTED_OPERAND";
  case PERR_ILLEGAL_OPERATOR:
    return "PERR_ILLEGAL_OPERATOR";
  case PERR_EOF:
    return "PERR_EOF";
  case NUMBER_OF_PERRORS:
    // This really shouldn't happen
  default:
    return "";
  }
}

char *perr_generate(perr_t err, buffer_t *buffer)
{
  const char *err_cstr = perr_as_cstr(err);
  int char_num_size    = snprintf(NULL, 0, "%zu", buffer->cur);
  char *message =
      calloc((4 + char_num_size + strlen(err_cstr) + strlen(buffer->name)),
             sizeof(*message));
  sprintf(message, "%s:%lu: %s", buffer->name, buffer->cur, err_cstr);
  return message;
}
