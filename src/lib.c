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
  buffer.data      = calloc(size, data_size + 1);
  buffer.available = size;
  buffer.cur       = 0;
  memcpy(buffer.data, str, data_size * size);
  buffer.data[data_size * size] = '\0';
  return buffer;
}

char buffer_peek(buffer_t buf)
{
  if (buffer_at_end(buf) == BUFFER_OK)
    return buf.data[buf.cur];
  return 0;
}

bool is_newline(char c)
{
  return c == '\r' || c == '\n';
}

void buffer_seek_nextline(buffer_t *buffer)
{
  for (;
       buffer_at_end(*buffer) == BUFFER_OK && is_newline(buffer_peek(*buffer));
       ++buffer->cur)
    continue;
}

void buffer_seek_next(buffer_t *buffer)
{
  for (; buffer_at_end(*buffer) == BUFFER_OK && isblank(buffer_peek(*buffer));
       ++buffer->cur)
    continue;
}

enum BufferState buffer_at_end(buffer_t buf)
{
  if (buf.available == 0 || buf.cur > buf.available)
    return BUFFER_PAST_END;
  else if (buf.cur == buf.available)
    return BUFFER_AT_END;
  else
    return BUFFER_OK;
}

size_t buffer_space_left(buffer_t buf)
{
  if (buffer_at_end(buf) != BUFFER_OK)
    return 0;
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
  if (darr->data)
    free(darr->data);
}

void darr_ensure_capacity(darr_t *darr, size_t wanted)
{
  if (darr->used + wanted >= darr->available)
  {
    darr->available =
        MAX(darr->used + wanted, darr->available * DARR_REALLOC_MULT);
    darr->data = reallocarray(darr->data, darr->available, darr->member_size);
  }
}

void darr_tighten(darr_t *darr)
{
  if (darr->used == 0)
  {
    free(darr->data);
    darr->data = NULL;
  }
  else
    darr->data = reallocarray(darr->data, darr->used, darr->member_size);
  darr->available = darr->used;
}

void darr_mem_append(darr_t *darr, void *ptr, size_t size)
{
  darr_ensure_capacity(darr, size);
  // NOTE: We need this cast to move byte-by-byte.  Of course GNU C
  // allows (void *) to have byte-like pointer arithemtic but we gotta
  // be good.
  memcpy(((int8_t *)darr->data) + (darr->member_size * darr->used), ptr,
         size * darr->member_size);
  darr->used += size;
}

void darr_mem_insert(darr_t *darr, void *ptr, size_t size, size_t where)
{
  // This is illegal
  if (where > darr->available)
    return;
  else if (where + size > darr->available)
  {
    // memcpy will later add these directly, so we can increase used
    darr_ensure_capacity(darr, where + size - darr->available);
    darr->used = where + size;
  }

  // NOTE: We need this cast to move byte-by-byte.  Of course GNU C
  // allows (void *) to have byte-like pointer arithemtic but we gotta
  // be good.
  memcpy(((int8_t *)darr->data) + (darr->member_size * where), ptr,
         size * darr->member_size);
}
