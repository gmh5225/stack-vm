/* lib.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: General library functions
 */

#include "./lib.h"

#include <ctype.h>
#include <string.h>

buffer_t buffer_read_file(FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  buffer_t buffer  = {0};
  size_t data_size = sizeof(buffer.data[0]);

  buffer.data      = calloc(size, data_size);
  buffer.available = size;
  buffer.cur       = 0;

  fread(buffer.data, data_size, size / data_size, fp);

  return buffer;
}

buffer_t buffer_read_cstr(const char *str, size_t size)
{
  buffer_t buffer  = {0};
  size_t data_size = sizeof(buffer.data[0]);
  buffer.data      = calloc(size, data_size);
  buffer.available = size;
  buffer.cur       = 0;
  memcpy(buffer.data, str, data_size * size);
  return buffer;
}

char buffer_peek(buffer_t buf)
{
  if (!buffer_is_end(buf))
    return buf.data[buf.cur];
  return 0;
}

void buffer_seek_next(buffer_t *buffer)
{
  for (; isspace(buffer->data[buffer->cur]); ++buffer->cur)
    continue;
}

bool buffer_is_end(buffer_t buf)
{
  return buf.cur >= buf.available;
}

size_t buffer_space_left(buffer_t buf)
{
  return buf.available - buf.cur;
}
