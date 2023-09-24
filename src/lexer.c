/* lexer.c
 * Created: 2023-09-24
 * Author: Aryadev Chavali
 * Description: Lexer/Tokeniser for parser, interpreter, etc
 */

#include "./lexer.h"

#include <ctype.h>
#include <string.h>

void token_print(token_t t, FILE *fp)
{
  const char *type_cstr = "";
  switch (t.type)
  {
  case TOKEN_EOF:
    type_cstr = "TOKEN_EOF";
    break;
  case TOKEN_DOT:
    type_cstr = "TOKEN_DOT";
    break;
  case TOKEN_DASH:
    type_cstr = "TOKEN_DASH";
    break;
    break;
  case TOKEN_COMMENT:
    type_cstr = "TOKEN_COMMENT";
    break;
  case TOKEN_WHITESPACE:
    type_cstr = "TOKEN_WHITESPACE";
    break;
  case TOKEN_SYMBOL:
    type_cstr = "TOKEN_SYMBOL";
    break;
  case TOKEN_CHARACTER:
    type_cstr = "TOKEN_CHARACTER";
    break;
  case TOKEN_NUMBER:
    type_cstr = "TOKEN_NUMBER";
    break;
  case TOKEN_OTHER:
    type_cstr = "TOKEN_OTHER";
    break;
  default:
    type_cstr = "TOKEN_UNKNOWN";
    break;
  }

  fprintf(fp, "%s@(%lu,%lu)[size=%lu]: `%s`", type_cstr, t.column, t.line,
          t.size, t.content);
}

void stream_print(stream_t *stream, FILE *fp)
{
  fprintf(fp, "STREAM=%s (cursor=%lu, number=%lu)\n", stream->name,
          stream->cursor, stream->size);
  for (size_t i = 0; i < stream->size; ++i)
  {
    token_print(stream->tokens[i], fp);
    fprintf(fp, "\n");
  }
}

token_t token_create(token_type_t type, size_t col, size_t line, char *str,
                     size_t size)
{
  token_t token = {.type    = type,
                   .size    = size,
                   .column  = col,
                   .line    = line,
                   .content = calloc(size + 1, sizeof(*str))};
  if (str)
  {
    memcpy(token.content, str, size);
    token.content[size] = '\0';
  }
  else
    memset(token.content, 0, size + 1);
  return token;
}

lerr_t tokenise_buffer(stream_t *stream, buffer_t *buffer)
{
  stream->name   = buffer->name;
  stream->cursor = 0;

  if (buffer_at_end(*buffer) != BUFFER_OK)
  {
    stream->tokens = NULL;
    stream->size   = 0;
    return LERR_OK;
  }

  darr_t tokens = {0};
  darr_init(&tokens, 1, sizeof(token_t));

  size_t column = 0, line = 1;

  while (buffer_at_end(*buffer) != BUFFER_PAST_END)
  {
    token_t token = {0};
    char c        = buffer_pop(buffer);
    switch (c)
    {
    case '\0':
      token = token_create(TOKEN_EOF, column, line, &c, 1);
      ++column;
      break;
    case '.':
      token = token_create(TOKEN_DOT, column, line, &c, 1);
      ++column;
      break;
    case '-':
      token = token_create(TOKEN_DASH, column, line, &c, 1);
      ++column;
      break;
    case '#': {
      // Figure out the size of our comment (until newline or eof)
      size_t comment_size = 0;
      for (char comment_char = buffer->data[buffer->cur + comment_size];
           comment_size < buffer_space_left(*buffer) &&
           !(isspace(comment_char) || comment_char != '\0');
           comment_char = buffer->data[buffer->cur + (++comment_size)])
        continue;
      token = token_create(TOKEN_COMMENT, column, line,
                           buffer->data + buffer->cur - 1, comment_size + 1);
      column += comment_size + 1;
      buffer->cur += comment_size;
      break;
    }
    case '\'': {
      // This may be a character literal
      if (buffer_peek(*buffer) == '\\')
      {
        // Escape sequence parsing
        char escaped = 0;
        switch (buffer->data[buffer->cur + 1])
        {
        case 'n':
          escaped = '\n';
          break;
        default:
          darr_free(&tokens);
          return LERR_CHAR_UNRECOGNISED_ESCAPE;
          break;
        }
        token = token_create(TOKEN_CHARACTER, column, line, &escaped, 1);
        column += 4;
      }
      else
      {
        // NOTE: ASCII specific (no unicode hence one byte checks)
        if (buffer->data[buffer->cur + 1] != '\'')
        {
          darr_free(&tokens);
          return LERR_CHAR_WRONG_SIZE;
        }

        token = token_create(TOKEN_CHARACTER, column, line,
                             buffer->data + buffer->cur - 1, 3);
        buffer->cur += 2;
        column += 3;
      }
      break;
    }
    default:
      if (isspace(c))
      {
        size_t prev_col  = column;
        size_t prev_line = line;

        ++column;
        if (c == '\n')
        {
          column = 0;
          ++line;
        }

        size_t i = 0;
        for (c = buffer_peek(*buffer);
             i < buffer_space_left(*buffer) && isspace(c);
             c = buffer->data[buffer->cur + (++i)])
        {
          ++column;
          if (c == '\n')
          {
            column = 0;
            ++line;
          }
          continue;
        }
        token = token_create(TOKEN_WHITESPACE, prev_col, prev_line, NULL, i);
        buffer->cur += i;
        break;
      }
      // Number parsers
      else if (isdigit(c) || (c == '-' && isdigit(buffer_peek(*buffer))))
      {
        bool decimal_place = false;
        size_t number_size = 0;
        for (char n_char = buffer_peek(*buffer);
             number_size < buffer_space_left(*buffer) &&
             (isdigit(n_char) || (n_char == '.' && !decimal_place));
             n_char = buffer->data[buffer->cur + (++number_size)])
          if (n_char == '.')
            decimal_place = true;
        token = token_create(TOKEN_NUMBER, column, line,
                             buffer->data + buffer->cur - 1, number_size + 1);
        column += number_size + 1;
        buffer->cur += number_size;
      }
      else if (strchr(LEXER_SYMBOL_ACCEPTED, c))
      {
        size_t symbol_size = 0;
        for (char symbol_char = buffer_peek(*buffer);
             symbol_size < buffer_space_left(*buffer) &&
             !(isspace(symbol_char) || symbol_char == '\0');
             symbol_char = buffer->data[buffer->cur + (++symbol_size)])
          continue;
        token = token_create(TOKEN_SYMBOL, column, line,
                             buffer->data + buffer->cur - 1, symbol_size + 1);
        column += symbol_size + 1;
        buffer->cur += symbol_size;
      }
      else
      {
        darr_free(&tokens);
        return LERR_UNRECOGNISED_TOKEN;
      }
      break;
    }
    // Append token to our current set
    DARR_APP(&tokens, token_t, token);
  }

  darr_tighten(&tokens);
  stream->tokens = tokens.data;
  stream->size   = tokens.used;
  return LERR_OK;
}

void stream_free(stream_t *stream)
{
  for (size_t i = 0; i < stream->size; ++i)
  {
    free(stream->tokens[i].content);
  }
  free(stream->tokens);
}
