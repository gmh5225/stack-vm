#ifndef LIB_H
#define LIB_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0

typedef uint64_t u64;
typedef int64_t i64;

typedef u64 word;

typedef struct
{
  char *data;
  size_t cur, available;
} buffer_t;

buffer_t buffer_read_file(FILE *fp);
buffer_t buffer_read_cstr(const char *, size_t);
char buffer_peek(buffer_t);
void buffer_seek_next(buffer_t *);
bool buffer_is_end(buffer_t);
size_t buffer_space_left(buffer_t);

#endif
