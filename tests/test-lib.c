/* test-lib.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for lib.h
 */

#include "./test-lib.h"
#include "./test.h"

#include "../lib.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

bool test_lib_MAX(void)
{
  ASSERT(test_1, MAX(0, 1) == 1);
  ASSERT(test_2, MAX(-2, 2) == 2);
  return test_1 && test_2;
}

bool test_lib_MIN(void)
{
  ASSERT(test_1, (MIN(0, 1) == 0));
  ASSERT(test_2, (MIN(-2, 2) == -2));
  return test_1 && test_2;
}

bool test_lib_ARR_SIZE(void)
{
  const char test_str[] = "This string is 29 characters";
  ASSERT(test_1, ARR_SIZE(test_str) == 29);

  const size_t test_ints_size = rand() % 1024;
  int64_t test_ints[test_ints_size];
  printf("\t\t[INFO]: test_lib_ARR_SIZE: test_ints=%lu\n", test_ints_size);
  ASSERT(test_2, ARR_SIZE(test_ints) == test_ints_size);
  return test_1 && test_2;
}

bool test_lib_buffer_read_file_fixed(size_t text_size)
{
  char *text = generate_random_data(text_size);

  // Create a mock file
  size_t n = snprintf(NULL, 0, "%lu", text_size);
  char filepath[30 + n];
  sprintf(filepath, "tests/TEST_LIB_MOCK_FILE-%lu.txt", text_size);
  FILE *fp = fopen(filepath, "w");
  if (!fp)
  {
    printf("\t\t[INFO]: test_lib_buffer_read_file: Could not open file (to "
           "write) `%s`: "
           "%s\n",
           filepath, strerror(errno));
    return false;
  }
  fwrite(text, 1, text_size, fp);
  fclose(fp);

  fp = fopen(filepath, "r");
  if (!fp)
  {
    printf("\t\t[INFO]: test_lib_buffer_read_file: Could not open file (to "
           "read) `%s`: "
           "%s\n",
           filepath, strerror(errno));
    return false;
  }
  buffer_t buf = buffer_read_file(filepath, fp);
  fclose(fp);

  ASSERT(test_1, buf.available == text_size);
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_file(void)
{
  // Create a mock file (using a random number to ensure we're not
  // conflicting with something in the workspace
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 10);
  bool test_small = test_lib_buffer_read_file_fixed(1 << 10);
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 20);
  bool test_medium = test_lib_buffer_read_file_fixed(1 << 20);
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 25);
  bool test_large = test_lib_buffer_read_file_fixed(1 << 25);
  return test_small && test_medium && test_large;
}

bool test_lib_buffer_read_cstr_fixed(size_t text_size)
{
  char *text   = generate_random_data(text_size);
  buffer_t buf = buffer_read_cstr("*test-cstr*", text, text_size);

  ASSERT(test_1, buf.available == text_size);
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_cstr(void)
{
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 10);
  bool test_small = test_lib_buffer_read_cstr_fixed(1 << 10);
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 20);
  bool test_medium = test_lib_buffer_read_cstr_fixed(1 << 20);
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 25);
  bool test_large = test_lib_buffer_read_cstr_fixed(1 << 25);
  return test_small && test_medium && test_large;
}

bool test_lib_buffer_peek(void)
{
  // Two contexts: Normal context and an EOF or empty context
  size_t text_size = 256;
  char *text       = generate_random_text(text_size, text_size / 4);
  buffer_t buffer  = buffer_read_cstr("*test-cstr*", text, text_size);

  ASSERT(test_top_equality, buffer_peek(buffer) == buffer.data[buffer.cur]);
  buffer_seek_nextline(&buffer);
  ASSERT(test_next_equality, buffer_peek(buffer) == buffer.data[buffer.cur]);
  free(text);
  free(buffer.data);

  char empty_text[] = "";
  buffer = buffer_read_cstr("*test-cstr*", empty_text, ARR_SIZE(empty_text));
  ASSERT(test_empty, buffer_peek(buffer) == 0);
  free(buffer.data);

  return test_top_equality && test_next_equality && test_empty;
}

bool test_lib_buffer_seek_next(void)
{
  size_t text_size = 256;
  char *text       = generate_random_text(text_size, text_size / 4);
  buffer_t buffer  = buffer_read_cstr("*test-cstr*", text, text_size);
  free(text);

  // Check we're actually skipping to a non blank character
  buffer_seek_next(&buffer);
  ASSERT(test_not_at_blank, !isblank(buffer_peek(buffer)));

  // Check behaviour when we're at the end
  buffer.cur = buffer.available;
  buffer_seek_next(&buffer);
  ASSERT(test_eof_behaviour, buffer_at_end(buffer));

  // Reset
  free(buffer.data);
  buffer = (buffer_t){0};

  // Let's test that it actually does what we want with some sample
  // text

  char sample[] = "    1    Word1 Word2     Word3\nShouldn't be skipped to\n";
  size_t sample_text_size = ARR_SIZE(sample);
  buffer = buffer_read_cstr("*test-cstr*", sample, sample_text_size);

  buffer_seek_next(&buffer);
  ASSERT(test_sample_char, buffer_peek(buffer) == '1');

  ++buffer.cur;
  buffer_seek_next(&buffer);
  ASSERT(test_sample_first_word,
         strncmp("Word1", buffer.data + buffer.cur, 5) == 0);

  buffer.cur += 5;
  buffer_seek_next(&buffer);
  ASSERT(test_sample_second_word,
         strncmp("Word2", buffer.data + buffer.cur, 5) == 0);

  buffer.cur += 5;
  buffer_seek_next(&buffer);
  ASSERT(test_sample_third_word,
         strncmp("Word3", buffer.data + buffer.cur, 5) == 0);

  buffer_seek_next(&buffer);
  ASSERT(test_sample_does_not_skip,
         strncmp("Word3", buffer.data + buffer.cur, 5) == 0);

  free(buffer.data);

  return test_not_at_blank && test_eof_behaviour && test_sample_char &&
         test_sample_first_word && test_sample_second_word &&
         test_sample_third_word && test_sample_does_not_skip;
}

bool test_lib_buffer_seek_nextline(void)
{
  size_t text_size = 256;
  char *text       = generate_random_text(text_size, text_size / 4);
  buffer_t buffer  = buffer_read_cstr("*test-cstr*", text, text_size);
  free(text);

  // Check we're actually skipping to a newline (i.e. we've got a
  // newline or space behind us)
  buffer_seek_nextline(&buffer);
  ASSERT(test_not_at_blank, !isspace(buffer_peek(buffer)));

  // Check behaviour when we're at the end
  buffer.cur = buffer.available;
  buffer_seek_nextline(&buffer);
  ASSERT(test_eof_behaviour, buffer_at_end(buffer));

  // Reset
  free(buffer.data);
  buffer = (buffer_t){0};

  // Let's test that it actually does what we want with some sample
  // text
  char sample[] = "I should stay here\n"
                  "I should stop here\n"
                  "\n"
                  "Can I stop here?\n"
                  "\n"
                  "\n"
                  "a\n"
                  "\n"
                  "      stop here";

  size_t sample_text_size = ARR_SIZE(sample);
  buffer = buffer_read_cstr("*test-cstr*", sample, sample_text_size);

  buffer_seek_nextline(&buffer);
  ASSERT(test_sample_first_sentence,
         strncmp(buffer.data + buffer.cur, "I should stay here", 18) == 0);
  buffer.cur += 18;

  buffer_seek_nextline(&buffer);
  ASSERT(test_sample_second_sentence,
         strncmp(buffer.data + buffer.cur, "I should stop here", 18) == 0);
  buffer.cur += 18;

  buffer_seek_nextline(&buffer);
  ASSERT(test_sample_third_sentence,
         strncmp(buffer.data + buffer.cur, "Can I stop here?", 16) == 0);
  buffer.cur += 16;

  buffer_seek_nextline(&buffer);
  ASSERT(test_sample_fourth_sentence,
         strncmp(buffer.data + buffer.cur, "a", 1) == 0);
  buffer.cur += 1;

  buffer_seek_nextline(&buffer);
  ASSERT(test_sample_fifth_sentence,
         strncmp(buffer.data + buffer.cur, "stop here", 9) == 0);

  buffer_seek_next(&buffer);
  ASSERT(test_sample_does_not_skip,
         strncmp(buffer.data + buffer.cur, "stop here", 9) == 0);

  free(buffer.data);

  return test_not_at_blank && test_eof_behaviour &&
         test_sample_first_sentence && test_sample_second_sentence &&
         test_sample_third_sentence && test_sample_fourth_sentence &&
         test_sample_fifth_sentence && test_sample_does_not_skip;
}

bool test_lib_buffer_at_end(void)
{
  // Empty context => empty
  buffer_t buf = {0};
  ASSERT(test_very_easy, buffer_at_end(buf));

  // Read a large text, go right to the end and check when going to
  // the penultimate, ultimate and past the end
  size_t text_size = 256;
  char *text       = generate_random_data(text_size);
  buf              = buffer_read_cstr("*test-cstr*", text, text_size);
  free(text);

  buf.cur = buf.available - 1;
  ASSERT(test_penultimate, buffer_at_end(buf));
  buf.cur = buf.available;
  ASSERT(test_ultimate, buffer_at_end(buf));

  buf.cur = buf.available + 1;
  ASSERT(test_kinda_past, buffer_at_end(buf));

  buf.cur = buf.available + text_size;
  ASSERT(test_way_past, buffer_at_end(buf));

  free(buf.data);

  return test_very_easy && test_penultimate && test_ultimate &&
         test_kinda_past && test_way_past;
}

bool test_lib_buffer_space_left(void)
{
  // Empty context => No space left
  buffer_t buf = {0};
  ASSERT(test_empty_no_space, buffer_space_left(buf) == 0);

  // Fresh context => Full space left
  size_t text_size = 256;
  char *text       = generate_random_data(text_size);
  buf              = buffer_read_cstr("*test-cstr*", text, text_size);
  free(text);
  ASSERT(test_fresh_full_space, buffer_space_left(buf) == buf.available);

  // If I halfway through the file I should get half the space left
  buf.cur = buf.available / 2;
  ASSERT(test_halfway_half_space, buffer_space_left(buf) == buf.available / 2);

  // If I reach the end I should have no space
  buf.cur = buf.available;
  ASSERT(test_end_no_space, buffer_space_left(buf) == 0);

  // If I go past the end, I still have no space
  buf.cur = buf.available + 256;
  ASSERT(test_past_end_no_space, buffer_space_left(buf) == 0);

  free(buf.data);

  return test_empty_no_space && test_fresh_full_space &&
         test_halfway_half_space && test_end_no_space && test_past_end_no_space;
}
