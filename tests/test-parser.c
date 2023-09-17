/* test-parser.c
 * Created: 2023-09-17
 * Author: Aryadev Chavali
 * Description: Unit tests for parser.h
 */

#include "./test-parser.h"

#include "../src/parser.h"

#include <assert.h>
#include <string.h>

bool test_perr_generate(void)
{
  // Creating mock data
  size_t data_size = 256;
  char *data       = generate_random_text(data_size, data_size / 4);

  buffer_t buf  = {0};
  buf.name      = "*test-perr*";
  buf.cur       = 0;
  buf.available = data_size;
  buf.data      = data;

  // This is to make sure we get an error if tests become outdated
  static_assert(NUMBER_OF_PERRORS == 5, "test_perr_generate: Outdated!");

  // Current schema is something like "<name>:<pos> <err-cstr>" so
  // let's test that with some expected outputs.
  const char *expected_outputs_test_perr_cstr[] = {
      "*test-perr*:0: PERR_OK",
      "*test-perr*:0: PERR_EXPECTED_INTEGER",
      "*test-perr*:0: PERR_UNEXPECTED_OPERATOR",
      "*test-perr*:0: PERR_UNEXPECTED_OPERAND",
      "*test-perr*:0: PERR_EOF",
  };

  bool test_perr_cstr = true;
  LOG_TEST_START(test_perr_cstr);
  for (size_t i = 0; i < NUMBER_OF_PERRORS; ++i)
  {
    perr_t perr          = (PERR_OK + i);
    char *output         = perr_generate(perr, &buf);
    const char *expected = expected_outputs_test_perr_cstr[i];
    printf("\t");
    ASSERT(test_ith_perr_cstr,
           strncmp(expected, output, strlen(expected)) == 0);
    free(output);
    test_perr_cstr = test_perr_cstr && test_ith_perr_cstr;
  }
  LOG_TEST_STATUS(test_perr_cstr, reduce(test_ith_perr_cstr, &));

  // Have to do this by hand
  const size_t number_of_tests_perr_cursor         = 4;
  const size_t inputs_test_perr_cursor[4]          = {0, 127, 200, 255};
  const char *expected_outputs_test_perr_cursor[4] = {
      "*test-perr*:0: PERR_OK",
      "*test-perr*:127: PERR_OK",
      "*test-perr*:200: PERR_OK",
      "*test-perr*:255: PERR_OK",
  };

  // Another check to ensure up to date
  assert((number_of_tests_perr_cursor == ARR_SIZE(inputs_test_perr_cursor)) &&
         (number_of_tests_perr_cursor ==
          ARR_SIZE(expected_outputs_test_perr_cursor)) &&
         "test_perr_generate: number of tests for cursor outdated!");

  bool test_perr_cursor = true;
  LOG_TEST_START(test_perr_cursor);
  for (size_t i = 0; i < number_of_tests_perr_cursor; ++i)
  {
    perr_t perr          = PERR_OK;
    buf.cur              = inputs_test_perr_cursor[i];
    char *output         = perr_generate(perr, &buf);
    const char *expected = expected_outputs_test_perr_cursor[i];
    printf("\t");
    ASSERT(test_ith_perr_cursor,
           strncmp(expected, output, strlen(expected)) == 0);
    free(output);
    test_perr_cursor = test_perr_cursor && test_ith_perr_cursor;
  }
  LOG_TEST_STATUS(test_perr_cursor, reduce(test_ith_perr_cursor, &));

  // Finally, just a test of the "file name" property
  buf.name = "Another name";
  buf.cur  = 128;
  const char expected_output_test_filename[] =
      "Another name:128: PERR_UNEXPECTED_OPERATOR";
  perr_t perr  = PERR_UNEXPECTED_OPERATOR;
  char *output = perr_generate(perr, &buf);
  ASSERT(test_perr_filename,
         strncmp(expected_output_test_filename, output,
                 strlen(expected_output_test_filename)) == 0);
  free(output);

  free(data);

  return test_perr_cstr && test_perr_cursor && test_perr_filename;
}

bool test_perr_generate(void);
bool test_parse_line(void);
bool test_parse_buffer(void);
