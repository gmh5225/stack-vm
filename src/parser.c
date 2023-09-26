/* parser.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Parser for assembly
 */

#include "./parser.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

perr_t parse_nil(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_pop(stream);
  if (!(token.type == TOKEN_SYMBOL && token.size == 3 &&
        strncmp(token.content, "nil", 3) == 0))
  {
    stream->cursor--;
    return PERR_EXPECTED_NIL;
  }
  *datum = data_nil();
  return PERR_OK;
}

perr_t parse_bool(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type == TOKEN_NUMBER)
  {
    perr_t perr = parse_u64(stream, datum);
    if (perr != PERR_OK)
      return PERR_EXPECTED_BOOL;
    stream_pop(stream);
    *datum = data_bool(data_as_uint(*datum));
    return PERR_OK;
  }
  else if (token.type == TOKEN_SYMBOL)
  {
    if (token.size == 4 && strncmp(token.content, "true", 4) == 0)
    {
      stream_pop(stream);
      *datum = data_bool(true);
      return PERR_OK;
    }
    else if (token.size == 5 && strncmp(token.content, "false", 5) == 0)
    {
      stream_pop(stream);
      *datum = data_bool(false);
      return PERR_OK;
    }
  }
  return PERR_EXPECTED_BOOL;
}

perr_t parse_char(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type == TOKEN_NUMBER)
  {
    perr_t perr = parse_i64(stream, datum);
    if (perr != PERR_OK)
      return PERR_EXPECTED_CHAR;
    else if (data_as_int(*datum) > CHAR_MAX)
      return PERR_CHAR_OVERFLOW;
    else if (data_as_int(*datum) < CHAR_MIN)
      return PERR_CHAR_UNDERFLOW;
    stream_pop(stream);
    *datum = data_char(data_as_int(*datum));
    return PERR_OK;
  }
  else if (token.type == TOKEN_CHARACTER)
  {
    stream_pop(stream);
    *datum = data_char(token.content[0]);
    return PERR_OK;
  }
  return PERR_EXPECTED_CHAR;
}

perr_t parse_i64(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type != TOKEN_NUMBER)
    return PERR_EXPECTED_INTEGER;

  char *end  = NULL;
  i64 parsed = strtoll(token.content, &end, 10);

  if (((u64)(end - token.content)) < token.size)
    return PERR_EXPECTED_INTEGER;
  else if (parsed < INT60_MIN)
    return PERR_INTEGER_UNDERFLOW;
  else if (parsed > INT60_MAX)
    return PERR_INTEGER_OVERFLOW;

  stream_pop(stream);
  *datum = data_int(parsed);
  return PERR_OK;
}

perr_t parse_u64(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type != TOKEN_NUMBER || token.content[0] == '-')
    return PERR_EXPECTED_UINTEGER;

  char *end  = NULL;
  u64 parsed = strtoul(token.content, &end, 10);

  if (((u64)(end - token.content)) < token.size)
    return PERR_EXPECTED_UINTEGER;
  else if (errno == ERANGE)
    return PERR_UINTEGER_OVERFLOW;

  stream_pop(stream);
  *datum = data_uint(parsed);
  return PERR_OK;
}

perr_t parse_float(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type != TOKEN_NUMBER)
    return PERR_EXPECTED_FLOAT;

  char *end    = NULL;
  float parsed = strtof(token.content, &end);

  if (((u64)(end - token.content)) < token.size)
    return PERR_EXPECTED_FLOAT;
  else if (errno == ERANGE)
  {
    if (parsed == HUGE_VALF)
      return PERR_FLOAT_OVERFLOW;
    else
      return PERR_FLOAT_UNDERFLOW;
  }

  stream_pop(stream);
  *datum = data_float(parsed);
  return PERR_OK;
}

perr_t parse_number(stream_t *stream, data_t **datum)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);
  if (token.type != TOKEN_NUMBER)
    return PERR_EXPECTED_NUMBER;

  if (strchr(token.content, '.'))
    return parse_float(stream, datum);
  perr_t perr = parse_i64(stream, datum);
  if (perr == PERR_INTEGER_OVERFLOW)
    return parse_u64(stream, datum);
  return perr;
}

perr_t parse_push(stream_t *stream, pres_t *res)
{
  // check eof
  if (stream->cursor >= stream->size)
    return PERR_EOF;
  // Assume we're at an operand
  token_t token = stream_peek(stream);

  res->type             = PRES_IMMEDIATE;
  res->immediate.opcode = OP_PUSH;
  if (token.type == TOKEN_NUMBER)
    return parse_number(stream, &res->immediate.operand);
  else if (token.type == TOKEN_CHARACTER)
    return parse_char(stream, &res->immediate.operand);
  else if (token.type == TOKEN_SYMBOL)
  {
    if (token.content[0] == 't' || token.content[0] == 'f')
      return parse_bool(stream, &res->immediate.operand);
    return parse_nil(stream, &res->immediate.operand);
  }
  else if (token.type == TOKEN_STAR)
  {
    res->type              = PRES_IPTR;
    res->immediate.operand = data_nil();
    stream_pop(stream);
    if (stream_peek(stream).type == TOKEN_NUMBER)
      return parse_u64(stream, &res->immediate.operand);
    return PERR_OK;
  }
  return PERR_EXPECTED_OPERAND;
}

perr_t parse_dup(stream_t *stream, pres_t *res)
{
  // check eof
  if (stream->cursor >= stream->size)
    return PERR_EOF;
  // Assume we're at an operand
  token_t token = stream_peek(stream);
  if (token.type != TOKEN_NUMBER)
    return PERR_EXPECTED_NUMBER;
  res->type             = PRES_IMMEDIATE;
  res->immediate.opcode = OP_DUP;
  return parse_u64(stream, &res->immediate.operand);
}

perr_t parse_label(stream_t *stream, pres_t *res)
{
  // check eof
  if (stream->cursor >= stream->size)
    return PERR_EOF;
  // Assume we're at an operand
  token_t token = stream_peek(stream);
  if (token.type != TOKEN_SYMBOL)
    return PERR_EXPECTED_LABEL;
  stream_pop(stream);
  res->type       = PRES_LABEL;
  res->label_name = token.content;
  return PERR_OK;
}

perr_t parse_jmp(stream_t *stream, pres_t *res)
{
  // check eof
  if (stream->cursor >= stream->size)
    return PERR_EOF;
  // Assume we're at an operand
  token_t token = stream_peek(stream);

  // There are 4 cases: absolute, relative, stack and label
  if (token.type == TOKEN_NUMBER)
  {
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_JUMP;
    return parse_u64(stream, &res->immediate.operand);
  }
  else if (token.type == TOKEN_DOT && stream_peek(stream).type == TOKEN_NUMBER)
  {
    res->type = PRES_JUMP_RELATIVE;
    stream_pop(stream);
    return parse_i64(stream, &res->operand);
  }
  else if (token.type == TOKEN_STAR)
  {
    // Stack based -> Using a nil for the operand
    res->type = PRES_IMMEDIATE;
    stream_pop(stream);
    res->immediate.opcode  = OP_JUMP;
    res->immediate.operand = data_nil();
    return PERR_OK;
  }
  else if (token.type == TOKEN_SYMBOL)
  {
    stream_pop(stream);
    res->type       = PRES_JUMP_LABEL;
    res->label_name = token.content;
    return PERR_OK;
  }

  return PERR_UNEXPECTED_OPERAND;
}

perr_t parse_line(stream_t *stream, pres_t *res)
{
  res->stream_cursor = stream->cursor;

#if DEBUG == 1
  enum BufferState state = buffer_at_end(*buf);
  printf("[" TERM_GREEN "parse_line" TERM_RESET "]: Parsing ");
  token_print(stream->tokens[stream->cursor], stdout);
  puts("");
#endif

  // If at end, fail
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  token_t token = stream_peek(stream);

  if (token.type != TOKEN_SYMBOL)
    return PERR_ILLEGAL_OPERATOR;
  else if (token.size >= 4 && memcmp(token.content, "noop", 4) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_NONE;
    goto NO_OPERAND;
  }
  else if (token.size >= 4 && memcmp(token.content, "halt", 4) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_HALT;
    goto NO_OPERAND;
  }
  // Type based pushes
  else if (token.size >= 4 && memcmp(token.content, "push", 4) == 0)
  {
    stream_pop(stream);
    stream_seek_next(stream);
    return parse_push(stream, res);
  }
  else if (token.size >= 3 && memcmp(token.content, "pop", 3) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_POP;
    goto NO_OPERAND;
  }
  else if (token.size >= 4 && memcmp(token.content, "plus", 4) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_PLUS;
    goto NO_OPERAND;
  }
  else if (token.size >= 4 && memcmp(token.content, "mult", 4) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_MULT;
    goto NO_OPERAND;
  }
  else if (token.size >= 3 && memcmp(token.content, "dup", 3) == 0)
  {
    stream_pop(stream);
    stream_seek_next(stream);
    return parse_dup(stream, res);
  }
  else if (token.size >= 5 && memcmp(token.content, "print", 5) == 0)
  {
    stream_pop(stream);
    res->type             = PRES_IMMEDIATE;
    res->immediate.opcode = OP_PRINT;
    goto NO_OPERAND;
  }
  else if (token.size >= 5 && memcmp(token.content, "label", 5) == 0)
  {
    stream_pop(stream);
    stream_seek_next(stream);
    return parse_label(stream, res);
  }
  else if (token.size >= 3 && memcmp(token.content, "jmp", 3) == 0)
  {
    stream_pop(stream);
    stream_seek_next(stream);
    return parse_jmp(stream, res);
  }
  return PERR_ILLEGAL_OPERATOR;
NO_OPERAND:
  res->immediate.operand = data_nil();
  token                  = stream_peek(stream);
  if (stream->cursor < stream->size &&
      !(token.type == TOKEN_WHITESPACE || token.type == TOKEN_COMMENT))
    return PERR_UNEXPECTED_OPERAND;
  return PERR_OK;
}

bool complete_cmp_string(char *a, char *b)
{
  return strlen(a) == strlen(b) && strncmp(a, b, strlen(a)) == 0;
}

perr_t process_presults(pres_t *results, size_t results_size, stream_t *stream,
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
    // Not processing immediates or labels (yet) so just increment
    // program_size
    if (res.type == PRES_IMMEDIATE || res.type == PRES_JUMP_LABEL)
      ++program_size;
    else if (res.type == PRES_JUMP_RELATIVE)
    {
      // Get absolute program address
      i64 addr     = data_as_int(res.operand);
      i64 abs_addr = program_size + addr;
      if (abs_addr < 0)
      {
        darr_free(&labels);
        return PERR_ILLEGAL_INST_ADDRESS;
      }
      (results + i)->type              = PRES_IMMEDIATE;
      (results + i)->immediate.opcode  = OP_JUMP;
      (results + i)->immediate.operand = data_int(abs_addr);
      // Then register in program
      ++program_size;
    }
    else if (res.type == PRES_LABEL)
    {
      struct LabelPair pair = {res.label_name, program_size};
      DARR_APP(&labels, struct LabelPair, pair);
    }
    else if (res.type == PRES_IPTR)
    {
      // This means it wants the operand of res.immediate to be the
      // next instruction pointer (as a uint)
      u64 iptr         = program_size + 1;
      data_type_t type = data_type(res.immediate.operand);
      if (type == DATA_UINT)
        iptr += data_as_uint(res.immediate.operand);
      (results + i)->immediate.operand = data_uint(iptr);
      ++program_size;
    }
  }

  darr_init(output, program_size, sizeof(op_t));
  // Fixup all label jumps
  for (size_t i = 0; i < results_size; ++i)
  {
    pres_t res = results[i];
    if (res.type == PRES_LABEL)
      continue;
    else if (res.type == PRES_IMMEDIATE || res.type == PRES_IPTR)
    {
      DARR_APP(output, op_t, res.immediate);
    }
    else if (res.type == PRES_JUMP_LABEL)
    {
      op_t op  = {0};
      size_t j = 0;
      for (; j < labels.used; ++j)
      {
        struct LabelPair pair = ((struct LabelPair *)labels.data)[j];
        if (complete_cmp_string(pair.name, res.label_name))
        {
          op = OP_CREATE_JMP(data_uint(pair.iptr));
          break;
        }
      }

      // If no corresponding label found
      if (j == labels.used)
      {
        darr_free(output);
        darr_free(&labels);
        stream->cursor = res.stream_cursor;
        return PERR_UNKNOWN_LABEL;
      }
      DARR_APP(output, op_t, op);
    }
  }

  darr_free(&labels);

  return PERR_OK;
}

perr_t parse_stream(stream_t *stream, op_t **instructions,
                    u64 *instructions_parsed)
{
  if (stream->cursor >= stream->size)
    return PERR_EOF;

  darr_t presults = {0};
  darr_init(&presults, DARR_INITAL_SIZE, sizeof(pres_t));

  stream_seek_next(stream);
  while (stream->cursor < stream->size && stream_peek(stream).type != TOKEN_EOF)
  {
    pres_t pres = {0};
    perr_t perr = parse_line(stream, &pres);

    if (perr != PERR_OK)
    {
      darr_free(&presults);
      return perr;
    }

    DARR_APP(&presults, pres_t, pres);
    // Bring us to the next token
    stream_seek_next(stream);
  }

  darr_t processed = {0};
  perr_t process_error =
      process_presults(presults.data, presults.used, stream, &processed);

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
  case PERR_CHAR_UNDERFLOW:
    return "PERR_CHAR_UNDERFLOW";
  case PERR_CHAR_OVERFLOW:
    return "PERR_CHAR_OVERFLOW";
  case PERR_INTEGER_UNDERFLOW:
    return "PERR_INTEGER_UNDERFLOW";
  case PERR_INTEGER_OVERFLOW:
    return "PERR_INTEGER_OVERFLOW";
  case PERR_UINTEGER_OVERFLOW:
    return "PERR_UINTEGER_OVERFLOW";
  case PERR_FLOAT_UNDERFLOW:
    return "PERR_FLOAT_UNDERFLOW";
  case PERR_FLOAT_OVERFLOW:
    return "PERR_FLOAT_OVERFLOW";
  case PERR_UNEXPECTED_OPERAND:
    return "PERR_UNEXPECTED_OPERAND";
  case PERR_EXPECTED_NIL:
    return "PERR_EXPECTED_NIL";
  case PERR_EXPECTED_BOOL:
    return "PERR_EXPECTED_BOOL";
  case PERR_EXPECTED_CHAR:
    return "PERR_EXPECTED_CHAR";
  case PERR_EXPECTED_INTEGER:
    return "PERR_EXPECTED_INTEGER";
  case PERR_EXPECTED_UINTEGER:
    return "PERR_EXPECTED_UINTEGER";
  case PERR_EXPECTED_FLOAT:
    return "PERR_EXPECTED_FLOAT";
  case PERR_EXPECTED_NUMBER:
    return "PERR_EXPECTED_NUMBER";
  case PERR_EXPECTED_OPERAND:
    return "PERR_EXPECTED_OPERAND";
  case PERR_EXPECTED_LABEL:
    return "PERR_EXPECTED_LABEL";
  case PERR_UNKNOWN_LABEL:
    return "PERR_UNKNOWN_LABEL";
  case PERR_ILLEGAL_OPERATOR:
    return "PERR_ILLEGAL_OPERATOR";
  case PERR_ILLEGAL_INST_ADDRESS:
    return "PERR_ILLEGAL_INST_ADDRESS";
  case PERR_EOF:
    return "PERR_EOF";
  case NUMBER_OF_PERRORS:
    // This really shouldn't happen
  default:
    return "";
  }
}

char *perr_generate(perr_t err, stream_t *stream)
{
  const char *err_cstr = perr_as_cstr(err);
  token_t token        = stream_peek(stream);
  int char_num_size    = snprintf(NULL, 0, "%s:%zu:%zu: %s", stream->name,
                                  token.line, token.column, err_cstr);
  char *message        = calloc(char_num_size + 1, sizeof(*message));
  sprintf(message, "%s:%zu:%zu: %s", stream->name, token.line, token.column,
          err_cstr);
  message[char_num_size] = '\0';
  return message;
}
