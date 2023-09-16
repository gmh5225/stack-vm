#ifndef LIB_H
#define LIB_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG     0
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

typedef uint64_t u64;
typedef int64_t i64;

typedef u64 word;

/* Expandable text buffers */
typedef struct
{
  const char *name;
  char *data;
  size_t cur, available;
} buffer_t;

buffer_t buffer_read_file(const char *, FILE *fp);
buffer_t buffer_read_cstr(const char *, const char *str, size_t);
char buffer_peek(buffer_t);
void buffer_seek_next(buffer_t *);
void buffer_seek_nextline(buffer_t *);
bool end_of_buffer(buffer_t);
size_t buffer_space_left(buffer_t);

/* Expandable generic buffers */
#define DARR_REALLOC_MULT 1.5
#define DARR_INITAL_SIZE  64
typedef struct
{
  void *data;
  size_t member_size, used, available;
} darr_t;

void darr_init(darr_t *, size_t initial_size, size_t member_size);
void darr_free(darr_t *);
void darr_ensure_capacity(darr_t *, size_t);
void darr_tighten(darr_t *);

#define DARR_APP(DARR, TYPE, OBJ)                 \
  do                                              \
  {                                               \
    darr_ensure_capacity(DARR, 1);                \
    (((TYPE)(DARR)->data)[(DARR)->used++] = OBJ); \
  } while (0);

#endif