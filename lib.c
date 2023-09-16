/* lib.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: General library functions
 */

#include "./lib.h"

#include <ctype.h>
#include <malloc.h>
#include <string.h>

buffer_t buffer_read_file(const char *name, FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  buffer_t buffer  = {0};
  size_t data_size = sizeof(buffer.data[0]);

  buffer.name      = name;
  buffer.data      = calloc(size, data_size);
  buffer.available = size;
  buffer.cur       = 0;

  fread(buffer.data, data_size, size / data_size, fp);

  return buffer;
}

buffer_t buffer_read_cstr(const char *name, const char *str, size_t size)
{
  buffer_t buffer  = {0};
  size_t data_size = sizeof(buffer.data[0]);
  buffer.name      = name;
  buffer.data      = calloc(size, data_size);
  buffer.available = size;
  buffer.cur       = 0;
  memcpy(buffer.data, str, data_size * size);
  return buffer;
}

char buffer_peek(buffer_t buf)
{
  if (!end_of_buffer(buf))
    return buf.data[buf.cur];
  return 0;
}

void buffer_seek_nextline(buffer_t *buffer)
{
  for (; !end_of_buffer(*buffer) && isspace(buffer->data[buffer->cur]);
       ++buffer->cur)
    continue;
}

void buffer_seek_next(buffer_t *buffer)
{
  for (; !end_of_buffer(*buffer) && isblank(buffer->data[buffer->cur]);
       ++buffer->cur)
    continue;
}

bool end_of_buffer(buffer_t buf)
{
  return buf.cur >= buf.available || buf.cur == buf.available - 1;
}

size_t buffer_space_left(buffer_t buf)
{
  return buf.available - buf.cur;
}

void darr_init(darr_t *darr, size_t initial_size, size_t member_size)
{
  darr->member_size = member_size;
  darr->used        = 0;
  darr->available   = initial_size;
  darr->data        = calloc(initial_size, member_size);
}

void darr_free(darr_t *darr)
{
  free(darr->data);
}

void darr_ensure_capacity(darr_t *darr, size_t wanted)
{
  if (darr->used + wanted >= darr->available)
    darr->data = reallocarray(
        darr->data,
        MAX(darr->used + wanted, darr->available * DARR_REALLOC_MULT),
        darr->member_size);
}

void darr_tighten(darr_t *darr)
{
  darr->data = reallocarray(darr->data, darr->used, darr->member_size);
}