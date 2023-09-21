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
    buf->cur += end_of_operator;
    buffer_seek_next(buf);
    size_t label_size =
        strspn(buf->data + buf->cur, PARSER_LABEL_ACCEPTED_CHARS);
    if (label_size == 0)
      return PERR_EXPECTED_LABEL;
    res->type = PRES_LABEL;
    // Parse label name
    res->label_name = calloc(label_size + 1, sizeof(*res->label_name));
    memcpy(res->label_name, buf->data + buf->cur, label_size);
    res->label_name[label_size] = '\0';

    buf->cur += label_size;

    return PERR_OK;
  }
  else if (memcmp(buf->data + buf->cur, "jmp", 3) == 0)
  {
    // Jump must have an operand afterwards
    buf->cur += end_of_operator;
    buffer_seek_next(buf);
    size_t operand_size = strcspn(buf->data + buf->cur, "\n\0");
    if (operand_size == 0)
      return PERR_EXPECTED_OPERAND;

    // There are three types of jmp I want to support:
    // Absolute jump (easiest)

    // First an error check for "negative absolute addresses"
    if (buffer_peek(*buf) == '-' && buffer_at_end(*buf) == BUFFER_OK &&
        isdigit(buf->data[buf->cur + 1]))
      return PERR_ILLEGAL_INST_ADDRESS;
    if (isdigit(buffer_peek(*buf)))
    {
      res->type             = PRES_IMMEDIATE;
      res->immediate.opcode = OP_JUMP;
      perr_t err            = parse_i64(buf, &res->immediate.operand);
      if (err != PERR_OK)
        return err;
      else
        return PERR_OK;
    }

    // Relative jump (medium)
    else if (buffer_peek(*buf) == '.')
    {
      // I need digits after this
      if (buffer_at_end(*buf) != BUFFER_OK)
        return PERR_EXPECTED_OPERAND;
      ++buf->cur;
      res->type = PRES_JUMP_RELATIVE;
      return parse_i64(buf, &res->relative_jump_operand);
    }

    // Label jump (hard)
    // First check if it is a valid label name
    size_t label_size =
        strspn(buf->data + buf->cur, PARSER_LABEL_ACCEPTED_CHARS);
    if (label_size == 0)
      return PERR_EXPECTED_OPERAND;
    else if (label_size != operand_size)
      return PERR_EXPECTED_LABEL;

    res->type       = PRES_JUMP_LABEL;
    res->label_name = calloc(label_size + 1, sizeof(*res->label_name));
    memcpy(res->label_name, buf->data + buf->cur, label_size);
    res->label_name[label_size] = '\0';

    buf->cur += label_size;

    return PERR_OK;
  }
  else
    return PERR_ILLEGAL_OPERATOR;
NO_OPERAND:
  buffer_seek_next(buf);
  if (buffer_at_end(*buf) == BUFFER_OK && buf->data[buf->cur] != '\n')
    return PERR_UNEXPECTED_OPERAND;
  return PERR_OK;
}

bool complete_cmp_string(char *a, char *b)
{
  return strlen(a) == strlen(b) && strncmp(a, b, strlen(a)) == 0;
}

perr_t process_presults(pres_t *results, size_t results_size, buffer_t *buffer,
                        darr_t *output)
{
  // Process labels and relative jumps
  struct LabelPair
  {
    char *name;
    u64 iptr;
  };

  darr_t labels       = {0};
  size_t program_size = 0;
  darr_init(&labels, 0, sizeof(struct LabelPair));

  for (size_t i = 0; i < results_size; ++i)
  {
    pres_t res = results[i];
    // Not processing immediates or labels (yet) so just increment program_size
    if (res.type == PRES_IMMEDIATE || res.type == PRES_JUMP_LABEL)
      ++program_size;
    else if (res.type == PRES_JUMP_RELATIVE)
    {
      // Get absolute program address
      i64 abs_addr = program_size + res.relative_jump_operand;
      if (abs_addr < 0)
      {
        darr_free(&labels);
        return PERR_ILLEGAL_INST_ADDRESS;
      }
      (results + i)->type              = PRES_IMMEDIATE;
      (results + i)->immediate.opcode  = OP_JUMP;
      (results + i)->immediate.operand = abs_addr;
      // Then register in program
      ++program_size;
    }
    else if (res.type == PRES_LABEL)
    {
      struct LabelPair pair = {res.label_name, program_size};
      DARR_APP(&labels, struct LabelPair, pair);
    }
  }

  darr_init(output, program_size, sizeof(op_t));
  // Fixup all label jumps
  for (size_t i = 0; i < results_size; ++i)
  {
    pres_t res = results[i];
    if (res.type == PRES_LABEL)
      continue;
    else if (res.type == PRES_IMMEDIATE)
    {
      DARR_APP(output, op_t, res.immediate);
    }
    else if (res.type == PRES_JUMP_LABEL)
    {
      op_t op = {0};
      for (size_t j = 0; j < labels.used; ++j)
      {
        struct LabelPair pair = ((struct LabelPair *)labels.data)[j];
        if (complete_cmp_string(pair.name, res.label_name))
        {
          op = OP_CREATE_JMP(pair.iptr);
          break;
        }
      }
      if (op.opcode != OP_JUMP)
      {
        darr_free(output);
        darr_free(&labels);
        buffer->cur = res.buffer_cursor;
        return PERR_UNKNOWN_LABEL;
      }
      DARR_APP(output, op_t, op);
    }
  }

  darr_free(&labels);

  return PERR_OK;
}

perr_t parse_buffer(buffer_t *buf, op_t **instructions,
                    u64 *instructions_parsed)
{
  if (buffer_at_end(*buf) != BUFFER_OK)
    return PERR_EOF;

  darr_t presults = {0};
  darr_init(&presults, DARR_INITAL_SIZE, sizeof(pres_t));

  buffer_seek_nextline(buf);
  while (buffer_at_end(*buf) == BUFFER_OK)
  {
    pres_t pres = {0};
    perr_t perr = parse_line(buf, &pres);

    if (perr != PERR_OK)
    {
      darr_free(&presults);
      return perr;
    }

    DARR_APP(&presults, pres_t, pres);
    buffer_seek_nextline(buf);
  }

  darr_t processed = {0};
  perr_t process_error =
      process_presults(presults.data, presults.used, buf, &processed);

  // Gotta free those labels
  for (size_t i = 0; i < presults.used; ++i)
    if (((pres_t *)presults.data)[i].type > PRES_JUMP_RELATIVE)
      free(((pres_t *)presults.data)[i].label_name);
  darr_free(&presults);

  if (process_error != PERR_OK)
    return process_error;

  *instructions        = processed.data;
  *instructions_parsed = processed.used;
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
  case PERR_ILLEGAL_INST_ADDRESS:
    return "PERR_ILLEGAL_INST_ADDRESS";
  case PERR_UNKNOWN_LABEL:
    return "PERR_UNKNOWN_LABEL";
  case PERR_EOF:
    return "PERR_EOF";
  case PERR_EXPECTED_LABEL:
    return "PERR_EXPECTED_LABEL";
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
