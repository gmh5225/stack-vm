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

  buffer_t buf = buffer_read_cstr("*test-perr*", data, data_size);
  free(data);

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
    ASSERT(test_ith_perr_cstr, memcmp(expected, output, strlen(expected)) == 0);
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
           memcmp(expected, output, strlen(expected)) == 0);
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
         memcmp(expected_output_test_filename, output,
                strlen(expected_output_test_filename)) == 0);
  free(output);

  free(buf.data);

  return test_perr_cstr && test_perr_cursor && test_perr_filename;
}

bool test_parse_line(void)
{
  // Test if each opcode can be parsed
  const char *test_ops[] = {"noop",  "halt",    "push 1", "plus",  "dup 1",
                            "print", "label 1", "jmp 1",  "jmp l1"};

  const op_t expected_ops[] = {{0},
                               OP_CREATE_HALT,
                               OP_CREATE_PUSH(1),
                               OP_CREATE_PLUS,
                               OP_CREATE_DUP(1),
                               OP_CREATE_PRINT,
                               OP_CREATE_LABEL(1),
                               OP_CREATE_JMP_REL(1),
                               OP_CREATE_JMP_LBL(1)};

  assert(ARR_SIZE(test_ops) == NUMBER_OF_OPERATORS &&
         ARR_SIZE(test_ops) == ARR_SIZE(expected_ops) &&
         "opcode_tests is outdated");

  bool test_parsed_completely = true, test_parsed_operator = true,
       test_parsed_operand = true, test_parsed_perr = true;
  for (size_t i = 0; i < ARR_SIZE(test_ops); ++i)
  {
    const char *test_data = test_ops[i];
    buffer_t push_buffer =
        buffer_read_cstr("*test-parse-line*", test_data, strlen(test_data));
    op_t ret                = {0};
    perr_t perr             = parse_line(&push_buffer, &ret);
    const op_t expected_out = expected_ops[i];

    printf("\t");
    // Check that buffer is pushed to the end (completely parsed buffer)
    ASSERT(test_ith_parsed_completely, buffer_at_end(push_buffer));
    test_parsed_completely =
        test_parsed_completely && test_ith_parsed_completely;

    printf("\t");
    // Check that operator is correct
    ASSERT(test_ith_parsed_operator, ret.opcode == expected_out.opcode);
    test_parsed_operator = test_parsed_operator && test_ith_parsed_operator;

    printf("\t");
    // Check that operand is correct
    ASSERT(test_ith_parsed_operand, ret.operand == expected_out.operand);
    test_parsed_operand = test_parsed_operand && test_ith_parsed_operand;

    printf("\t");
    // Check that perr = PERR_OK
    ASSERT(test_ith_parsed_perr, perr == PERR_OK);
    test_parsed_perr = test_parsed_perr && test_ith_parsed_perr;

    free(push_buffer.data);
  }

  LOG_TEST_STATUS(test_parsed_completely,
                  reduce(test_ith_parsed_completely, &));
  LOG_TEST_STATUS(test_parsed_operator, reduce(test_ith_parsed_operator, &));
  LOG_TEST_STATUS(test_parsed_operand, reduce(test_ith_parsed_operand, &));
  LOG_TEST_STATUS(test_parsed_perr, reduce(test_ith_parsed_perr, &));

  return test_parsed_completely && test_parsed_operator && test_parsed_operand;
}

bool test_parse_buffer(void);
