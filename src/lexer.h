#ifndef LEXER_H
#define LEXER_H

#include "./lib.h"

typedef enum
{
  LERR_CHAR_UNRECOGNISED_ESCAPE,
  LERR_CHAR_WRONG_SIZE,
  LERR_UNRECOGNISED_TOKEN,
  LERR_OK
} lerr_t;

typedef enum
{
  TOKEN_EOF = 0,
  TOKEN_DOT,
  TOKEN_DASH,
  TOKEN_WHITESPACE,
  TOKEN_COMMENT,

  // Useful for parsing
  TOKEN_SYMBOL,
  // Data types
  TOKEN_CHARACTER,
  TOKEN_NUMBER,

  // Catch all
  TOKEN_OTHER,
} token_type_t;

typedef struct
{
  token_type_t type;
  char *content;
  size_t size, column, line;
} token_t;

#define LEXER_SYMBOL_ACCEPTED \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"

token_t token_create(token_type_t, size_t, size_t, char *, size_t);
void token_print(token_t, FILE *);

typedef struct
{
  const char *name;
  token_t *tokens;
  size_t cursor, size;
} stream_t;

lerr_t tokenise_buffer(stream_t *stream, buffer_t *buffer);
void stream_print(stream_t *, FILE *);
void stream_free(stream_t *);

#endif
