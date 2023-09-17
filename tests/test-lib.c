/* test-lib.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for lib.h
 */

#include "./test-lib.h"
#include "./test.h"

#include "../src/lib.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
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
  LOG_TEST_INFO(test_2, "test_ints=%lu\n", test_ints_size);
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
  FILE *fp = fopen(filepath, "wb");
  if (!fp)
  {
    LOG_TEST_INFO(__func__, "Could not open file (to write) `%s`: %s\n",
                  filepath, strerror(errno));
    return false;
  }
  fwrite(text, 1, text_size, fp);
  fclose(fp);

  fp = fopen(filepath, "r");
  if (!fp)
  {
    LOG_TEST_INFO(__func__, "Could not open file (to read) `%s`: %s\n",
                  filepath, strerror(errno));
    return false;
  }
  buffer_t buf = buffer_read_file(filepath, fp);
  fclose(fp);

  printf("\t");
  ASSERT(test_1, buf.available == text_size);
  printf("\t");
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_file(void)
{
  LOG_TEST_START(test_small);
  LOG_TEST_INFO(test_small, "Testing data size %lu\n", 1LU << 10);
  bool test_small = test_lib_buffer_read_file_fixed(1 << 10);
  LOG_TEST_STATUS(test_small, test_1 & test_2);

  LOG_TEST_START(test_medium);
  LOG_TEST_INFO(test_medium, "Testing data size %lu\n", 1LU << 20);
  bool test_medium = test_lib_buffer_read_file_fixed(1 << 20);
  LOG_TEST_STATUS(test_medium, test_1 & test_2);

  LOG_TEST_START(test_large);
  LOG_TEST_INFO(test_large, "Testing data size %lu\n", 1LU << 25);
  bool test_large = test_lib_buffer_read_file_fixed(1 << 25);
  LOG_TEST_STATUS(test_large, test_1 & test_2);

  return test_small && test_medium && test_large;
}

bool test_lib_buffer_read_cstr_fixed(size_t text_size)
{
  char *text   = generate_random_data(text_size);
  buffer_t buf = buffer_read_cstr("*test-cstr*", text, text_size);

  printf("\t");
  ASSERT(test_1, buf.available == text_size);
  printf("\t");
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_cstr(void)
{
  LOG_TEST_START(test_small);
  LOG_TEST_INFO(test_small, "Testing data size %lu\n", 1LU << 10);
  bool test_small = test_lib_buffer_read_cstr_fixed(1 << 10);
  LOG_TEST_STATUS(test_small, test_1 & test_2);

  LOG_TEST_START(test_medium);
  LOG_TEST_INFO(test_medium, "Testing data size %lu\n", 1LU << 20);
  bool test_medium = test_lib_buffer_read_cstr_fixed(1 << 20);
  LOG_TEST_STATUS(test_medium, test_1 & test_2);

  LOG_TEST_START(test_large);
  LOG_TEST_INFO(test_large, "Testing data size %lu\n", 1LU << 25);
  bool test_large = test_lib_buffer_read_cstr_fixed(1 << 25);
  LOG_TEST_STATUS(test_large, test_1 & test_2);

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

bool test_lib_darr_ensure_capacity(void)
{
  darr_t darr      = {0};
  darr.member_size = sizeof(int); // You need SOME member_size

  // If I've got no space i.e. I'm a fresh dynamic array, then to
  // ensure capacity means to allocate
  size_t prev_size = 0;
  darr_ensure_capacity(&darr, 1);
  ASSERT(test_empty_causes_trigger, prev_size < darr.available);
  darr_free(&darr);

  // Dynamic array of integers
  darr_init(&darr, 64, sizeof(int));

  // Sufficient capacity => No changes
  prev_size = darr.available;
  darr_ensure_capacity(&darr, 1);
  ASSERT(test_sufficient_no_change, prev_size == darr.available);

  // "filling" up dynamic array till bounds, then requesting any more
  // triggers a reallocation
  darr.used = darr.available - 1;
  darr_ensure_capacity(&darr, 1);
  ASSERT(test_filled_triggers_change, prev_size < darr.available);

  darr_free(&darr);

  return test_empty_causes_trigger && test_sufficient_no_change &&
         test_filled_triggers_change;
}

bool test_lib_darr_tighten(void)
{
  darr_t darr      = {0};
  darr.member_size = sizeof(int);

  // No space => No change
  size_t prev = darr.available;
  darr_tighten(&darr);
  ASSERT(test_empty_no_trigger, prev == darr.available);

  // If I've got way more space than I need, then tighten should work
  darr_init(&darr, 64, sizeof(int));
  prev      = darr.available;
  darr.used = 32;
  darr_tighten(&darr);
  ASSERT(test_more_space_tightens, darr.used == darr.available && darr.data);

  // If I use no space and I tighten, I should free the data I have
  darr.used = 0;
  prev      = darr.available;
  darr_tighten(&darr);
  ASSERT(test_no_used_frees, darr.available == 0 && darr.data == NULL);

  darr_free(&darr);

  return test_empty_no_trigger && test_more_space_tightens &&
         test_no_used_frees;
}

bool test_lib_darr_mem_append(void)
{
  darr_t darr = {0};

  // Appending on no buffer means allocation and usage
  darr_init(&darr, 0, sizeof(int));
  int i = 1;
  darr_mem_append(&darr, &i, 1);
  ASSERT(test_empty_append_allocates, darr.available > 0 && darr.used > 0);
  ASSERT(test_empty_append_only_adds_1, darr.used == 1);

  darr_free(&darr);

  // Appending n variably sized pieces of memory should be fine
  size_t max_size = 1024;
  size_t n        = 16;

  size_t acc_size = 0;
  size_t sizes[n];
  char *chunks[n];

  LOG_TEST_START(test_chunks_appended);
  for (size_t i = 0; i < n; ++i)
  {
    sizes[i] = rand() % max_size;
    LOG_TEST_INFO(test_chunks_appended, "Chunk %lu is of size %lu\n", i,
                  sizes[i]);
    chunks[i] = generate_random_data(sizes[i]);
    acc_size += sizes[i];
  }
  LOG_TEST_INFO(test_chunks_appended, "Total data size %lu\n", acc_size);

  darr_init(&darr, 1, sizeof(*chunks[0]));
  for (size_t i = 0; i < n; ++i)
    darr_mem_append(&darr, chunks[i], sizes[i]);

  ASSERT(test_chunks_expected_usage, darr.used == acc_size);

  bool test_chunks_appended = true;
  size_t acc                = 0;
  for (size_t i = 0; i < n && test_chunks_appended; ++i)
  {
    char *chunk          = chunks[i];
    size_t size_of_chunk = sizes[i];
    printf("\t");
    ASSERT(test_chunks_ith_chunk_is_appended,
           strncmp(((char *)darr.data) + acc, chunk, size_of_chunk) == 0);
    test_chunks_appended =
        test_chunks_appended && test_chunks_ith_chunk_is_appended;
    acc += size_of_chunk;
  }

  LOG_TEST_STATUS(test_chunks_appended,
                  reduce(test_chunks_ith_chunk_is_appended, &));

  darr_free(&darr);
  for (size_t i = 0; i < n; ++i)
    free(chunks[i]);

  // Word-by-word sentence construction test using an initially tiny
  // buffer
  darr_init(&darr, 1, sizeof(char));
#define DARR_APP_STR(STR) (darr_mem_append(&darr, STR, strlen(STR)))

  DARR_APP_STR("This ");
  DARR_APP_STR("is ");
  DARR_APP_STR("a ");
  DARR_APP_STR("sentence!");

  ASSERT(test_word_by_word_correct_size, darr.used == 19);
  ASSERT(test_word_by_word_correct_string,
         strncmp(darr.data, "This is a sentence!", 19) == 0);

  darr_free(&darr);

  return test_empty_append_allocates && test_empty_append_only_adds_1 &&
         test_chunks_expected_usage && test_chunks_appended &&
         test_word_by_word_correct_size && test_word_by_word_correct_string;
}

bool test_lib_darr_mem_insert(void)
{
  darr_t darr = {0};

  // Inserting one element in an empty darr allocates
  int i = 1;
  darr_init(&darr, 0, sizeof(i));
  darr_mem_insert(&darr, &i, 1, 0);
  ASSERT(test_empty_insert_once_allocates,
         darr.available == 1 && darr.used == 1);
  darr_free(&darr);

  // Insertion of multiple elements on an empty darr allocates the
  // right size
  size_t data_size = 256;
  char *data       = generate_random_data(data_size);

  darr_init(&darr, 0, sizeof(*data));
  darr_mem_insert(&darr, data, 256, 0);
  free(data);

  LOG_TEST_INFO(test_empty_insert_mult_allocates, "available data %lu\n",
                darr.available);
  ASSERT(test_empty_insert_mult_allocates, darr.used == data_size);

  // Inserting in the middle of the darr changes state but does not
  // allocate
  char sentence[] = "I am inserting this sentence in the middle of the data!";
  darr_mem_insert(&darr, sentence, ARR_SIZE(sentence), data_size / 2);
  // Test for no allocation
  ASSERT(test_middle_insert_does_not_allocate, darr.used == data_size);
  // Check that state has been changed
  ASSERT(test_middle_insert_works, strncmp(((char *)darr.data) + data_size / 2,
                                           sentence, ARR_SIZE(sentence)) == 0);

  // Inserting another random data buffer of the same size but half
  // way must force allocation
  data = generate_random_data(data_size);
  darr_mem_insert(&darr, data, data_size, data_size / 2);

  // `size` bytes so far, inserting another `size` bytes at `size/2`
  // means we need to allocate `size/2` extra bytes right?
  ASSERT(test_large_mid_insert_reallocates_correctly,
         darr.used == (data_size + data_size / 2));
  ASSERT(test_large_mid_insert_works,
         strncmp(((char *)darr.data) + data_size / 2, data, data_size) == 0);
  free(data);

  darr_free(&darr);

  return test_empty_insert_once_allocates && test_empty_insert_mult_allocates &&
         test_middle_insert_does_not_allocate && test_middle_insert_works &&
         test_large_mid_insert_reallocates_correctly &&
         test_large_mid_insert_works;
}

bool test_lib_DARR_APP(void)
{
  // Empty darr => a reallocation
  darr_t darr      = {0};
  darr.member_size = sizeof(char);
  DARR_APP(&darr, char, 'a');
  ASSERT(test_empty_insert_reallocates, darr.used == 1 && darr.available > 0);
  darr_free(&darr);

  // Appending a sentence character by character
  darr_init(&darr, 0, sizeof(char));
  char sentence[] = "This is a sentence!\n";
  for (size_t i = 0; i < ARR_SIZE(sentence); ++i)
    DARR_APP(&darr, char, sentence[i]);
  ASSERT(test_sentence_uses_right_space, darr.used == ARR_SIZE(sentence));
  ASSERT(test_sentence_works,
         strncmp(((char *)darr.data), sentence, ARR_SIZE(sentence)) == 0);
  // Testing if amortized constant big O space is true (by allocating
  // more space than necessary)
  ASSERT(test_sentence_allocated_space,
         darr.available == (1LU << ((size_t)ceil(log2(ARR_SIZE(sentence))))));
  darr_free(&darr);

  // Appending random data character by character works as well
  darr_init(&darr, 0, sizeof(char));
  size_t data_size = 250;
  LOG_TEST_INFO(test_rand_data *, "data_size=%lu\n", data_size);
  char *data = generate_random_data(data_size);
  for (size_t i = 0; i < data_size; ++i)
    DARR_APP(&darr, char, data[i]);

  ASSERT(test_rand_data_uses_right_space, darr.used == data_size);
  ASSERT(test_rand_data_works,
         strncmp(((char *)darr.data), data, data_size) == 0);
  // Testing if amortized constant big O space is true (by allocating
  // more space than necessary)
  ASSERT(test_rand_data_allocated_space,
         darr.available == (1LU << ((size_t)ceil(log2(data_size)))));
  free(data);
  darr_free(&darr);

  return test_empty_insert_reallocates && test_sentence_uses_right_space &&
         test_sentence_works && test_sentence_allocated_space &&
         test_rand_data_uses_right_space && test_rand_data_works &&
         test_rand_data_allocated_space;
}
