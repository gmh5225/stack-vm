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
      "*test-perr*:0: PERR_EXPECTED_OPERAND",
      "*test-perr*:0: PERR_UNEXPECTED_OPERAND",
      "*test-perr*:0: PERR_ILLEGAL_OPERATOR",
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
      "Another name:128: PERR_ILLEGAL_OPERATOR";
  perr_t perr  = PERR_ILLEGAL_OPERATOR;
  char *output = perr_generate(perr, &buf);
  ASSERT(test_perr_filename,
         memcmp(expected_output_test_filename, output,
                strlen(expected_output_test_filename)) == 0);
  free(output);

  free(buf.data);

  return test_perr_cstr && test_perr_cursor && test_perr_filename;
}

bool test_parse_i64(void)
{
  // Parsing empty buffers
  buffer_t buf = {0};
  i64 ret      = 0;
  perr_t err   = parse_i64(&buf, &ret);
  ASSERT(test_empty_buffer, err == PERR_EOF);
  // Parsing positive integers
  const char *test_positive_inputs[] = {
      "1", "10", "1000", "4294967296", "3141", "4611686018427387904"};
  const i64 expected_positive_outputs[] = {
      1, 10, 1000, 4294967296, 3141, 4611686018427387904};

  bool test_positive_correct = true, test_positive_no_error = true;
  LOG_TEST_START(test_positive_correct);
  LOG_TEST_START(test_positive_no_error);

  for (size_t i = 0; i < ARR_SIZE(test_positive_inputs); ++i)
  {
    const char *test = test_positive_inputs[i];
    buf              = buffer_read_cstr("*test-parse-i64*", test, strlen(test));
    err              = parse_i64(&buf, &ret);
    const i64 expected = expected_positive_outputs[i];

    LOG_TEST_INFO(test_ith_positive_, "Expected=%lu\n", expected);

    ASSERT(test_ith_positive_correct, ret == expected);
    test_positive_correct = test_positive_correct && test_ith_positive_correct;
    ASSERT(test_ith_positive_no_error, err == PERR_OK);
    test_positive_no_error =
        test_positive_no_error && test_ith_positive_no_error;

    free(buf.data);
  }

  LOG_TEST_STATUS(test_positive_correct, reduce(test_ith_positive_correct, &));
  LOG_TEST_STATUS(test_positive_no_error,
                  reduce(test_ith_positive_no_error, &));

  // Parsing negative integers
  const char *test_negative_inputs[] = {
      "-1", "-10", "-1000", "-4294967296", "-3141", "-4611686018427387904"};
  const i64 expected_negative_outputs[] = {
      -1, -10, -1000, -4294967296, -3141, -4611686018427387904};

  bool test_negative_correct = true, test_negative_no_error = true;
  LOG_TEST_START(test_negative_correct);
  LOG_TEST_START(test_negative_no_error);

  for (size_t i = 0; i < ARR_SIZE(test_negative_inputs); ++i)
  {
    const char *test = test_negative_inputs[i];
    buf              = buffer_read_cstr("*test-parse-i64*", test, strlen(test));
    err              = parse_i64(&buf, &ret);
    const i64 expected = expected_negative_outputs[i];

    LOG_TEST_INFO(test_ith_negative_, "Expected=%ld\n", expected);

    ASSERT(test_ith_negative_correct, ret == expected);
    test_negative_correct = test_negative_correct && test_ith_negative_correct;
    ASSERT(test_ith_negative_no_error, err == PERR_OK);
    test_negative_no_error =
        test_negative_no_error && test_ith_negative_no_error;

    free(buf.data);
  }

  LOG_TEST_STATUS(test_negative_correct, reduce(test_ith_negative_correct, &));
  LOG_TEST_STATUS(test_negative_no_error,
                  reduce(test_ith_negative_no_error, &));

  // Parsing bad integers (should cause errors)
  const char *test_bad_inputs[] = {"This is certainly not an integer",
                                   "Why are you looking for an integer?",
                                   "<<<3"};

  bool test_bad_got_error = true;
  LOG_TEST_START(test_bad_got_error);

  for (size_t i = 0; i < ARR_SIZE(test_bad_inputs); ++i)
  {
    const char *test = test_bad_inputs[i];
    buf              = buffer_read_cstr("*test-parse-i64*", test, strlen(test));
    err              = parse_i64(&buf, &ret);

    ASSERT(test_ith_bad_got_error, err == PERR_EXPECTED_OPERAND);
    test_bad_got_error = test_bad_got_error && test_ith_bad_got_error;

    free(buf.data);
  }

  LOG_TEST_STATUS(test_bad_got_error, reduce(test_ith_bad_got_error, &));

  return test_empty_buffer && test_positive_correct && test_positive_no_error &&
         test_negative_correct && test_negative_no_error && test_bad_got_error;
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

  LOG_TEST_START(test_parsed_completely);
  LOG_TEST_START(test_parsed_operator);
  LOG_TEST_START(test_parsed_operand);
  LOG_TEST_START(test_parsed_perr);

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
    ASSERT(test_ith_parsed_completely, buffer_at_end(push_buffer) != BUFFER_OK);
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

  // Test if weird whitespace is fine
  const char *test_whitespace_inputs[]     = {"push 1", "push       20",
                                              "   push 5", "   push     20"};
  const op_t expected_whitespace_outputs[] = {
      OP_CREATE_PUSH(1), OP_CREATE_PUSH(20), OP_CREATE_PUSH(5),
      OP_CREATE_PUSH(20)};

  assert(ARR_SIZE(test_whitespace_inputs) ==
             ARR_SIZE(expected_whitespace_outputs) &&
         "whitespace_tests is outdated");

  bool test_whitespace_parsed_completely = true,
       test_whitespace_parsed_operator   = true,
       test_whitespace_parsed_operand    = true,
       test_whitespace_parsed_perr       = true;

  LOG_TEST_START(test_whitespace_completely);
  LOG_TEST_START(test_whitespace_operator);
  LOG_TEST_START(test_whitespace_operand);
  LOG_TEST_START(test_whitespace_perr);

  for (size_t i = 0; i < ARR_SIZE(test_whitespace_inputs); ++i)
  {
    const char *test_data = test_whitespace_inputs[i];
    buffer_t push_buffer =
        buffer_read_cstr("*test-parse-line*", test_data, strlen(test_data));
    op_t ret                = {0};
    perr_t perr             = parse_line(&push_buffer, &ret);
    const op_t expected_out = expected_whitespace_outputs[i];

    printf("\t");
    // Check that buffer is pushed to the end (completely parsed buffer)
    ASSERT(test_ith_whitespace_parsed_completely,
           buffer_at_end(push_buffer) != BUFFER_OK);
    test_parsed_completely =
        test_parsed_completely && test_ith_whitespace_parsed_completely;

    printf("\t");
    // Check that operator is correct
    ASSERT(test_ith_whitespace_parsed_operator,
           ret.opcode == expected_out.opcode);
    test_parsed_operator =
        test_parsed_operator && test_ith_whitespace_parsed_operator;

    printf("\t");
    // Check that operand is correct
    ASSERT(test_ith_whitespace_parsed_operand,
           ret.operand == expected_out.operand);
    test_parsed_operand =
        test_parsed_operand && test_ith_whitespace_parsed_operand;

    printf("\t");
    // Check that perr = PERR_OK
    ASSERT(test_ith_whitespace_parsed_perr, perr == PERR_OK);
    test_whitespace_parsed_perr =
        test_whitespace_parsed_perr && test_ith_whitespace_parsed_perr;

    free(push_buffer.data);
  }

  LOG_TEST_STATUS(test_whitespace_parsed_completely,
                  reduce(test_ith_whitespace_parsed_completely, &));
  LOG_TEST_STATUS(test_whitespace_parsed_operator,
                  reduce(test_ith_whitespace_parsed_operator, &));
  LOG_TEST_STATUS(test_whitespace_parsed_operand,
                  reduce(test_ith_whitespace_parsed_operand, &));
  LOG_TEST_STATUS(test_whitespace_parsed_perr,
                  reduce(test_ith_whitespace_parsed_perr, &));

  // Test if I can parse line by line
  const char test_line_by_line_input[]      = "push 10\n"
                                              "push 20\n"
                                              "dup 10\n";
  const op_t expected_line_by_line_output[] = {
      OP_CREATE_PUSH(10), OP_CREATE_PUSH(20), OP_CREATE_DUP(10)};

  buffer_t buffer = buffer_read_cstr("*test-perr*", test_line_by_line_input,
                                     strlen(test_line_by_line_input));

  bool test_line_by_line_operator = true, test_line_by_line_operand = true,
       test_line_by_line_perr = true;

  LOG_TEST_START(test_line_by_line_operator);
  LOG_TEST_START(test_line_by_line_operand);
  LOG_TEST_START(test_line_by_line_perr);

  for (size_t i = 0; buffer_at_end(buffer) == BUFFER_OK; ++i)
  {
    op_t ret          = {0};
    perr_t perr       = parse_line(&buffer, &ret);
    op_t expected_out = expected_line_by_line_output[i];

    printf("\t");
    // Check that operator is correct
    ASSERT(test_ith_line_by_line_operator, ret.opcode == expected_out.opcode);
    test_line_by_line_operator =
        test_line_by_line_operator && test_ith_line_by_line_operator;

    printf("\t");
    // Check that operand is correct
    ASSERT(test_ith_line_by_line_operand, ret.operand == expected_out.operand);
    test_line_by_line_operand =
        test_line_by_line_operand && test_ith_line_by_line_operand;

    printf("\t");
    // Check that perr = PERR_OK
    ASSERT(test_ith_line_by_line_perr, perr == PERR_OK);
    test_line_by_line_perr =
        test_line_by_line_perr && test_ith_line_by_line_perr;

    buffer_seek_nextline(&buffer);
  }

  free(buffer.data);

  LOG_TEST_STATUS(test_line_by_line_operator,
                  reduce(test_ith_line_by_line_operator, &));
  LOG_TEST_STATUS(test_line_by_line_operand,
                  reduce(test_ith_line_by_line_operand, &));
  LOG_TEST_STATUS(test_line_by_line_perr,
                  reduce(test_ith_line_by_line_perr, &));

  // Now test for error states

  // No operand
  const char *test_perr_no_operand_inputs[] = {"push", "dup", "label", "jmp"};

  bool test_perr_no_operand = true;
  LOG_TEST_START(test_perr_no_operand);
  for (size_t i = 0; i < ARR_SIZE(test_perr_no_operand_inputs); ++i)
  {
    buffer_t buffer =
        buffer_read_cstr("*test-parse-line*", test_perr_no_operand_inputs[i],
                         strlen(test_perr_no_operand_inputs[i]));

    op_t op  = {0};
    perr_t p = parse_line(&buffer, &op);
    printf("%s\n", perr_as_cstr(p));
    printf("\t");
    ASSERT(test_ith_perr_no_operand, p == PERR_EXPECTED_OPERAND);
    test_perr_no_operand = test_perr_no_operand && test_ith_perr_no_operand;

    free(buffer.data);
  }
  LOG_TEST_STATUS(test_perr_no_operand, reduce(test_ith_perr_no_operand, &));

  // Unexpected operand
  const char *test_perr_unexpected_operand_inputs[] = {"halt 1", "plus 2",
                                                       "print 3"};

  bool test_perr_unexpected_operand = true;
  LOG_TEST_START(test_perr_unexpected_operand);
  for (size_t i = 0; i < ARR_SIZE(test_perr_unexpected_operand_inputs); ++i)
  {
    buffer_t buffer = buffer_read_cstr(
        "*test-parse-line*", test_perr_unexpected_operand_inputs[i],
        strlen(test_perr_unexpected_operand_inputs[i]));

    op_t op  = {0};
    perr_t p = parse_line(&buffer, &op);

    printf("\t");
    ASSERT(test_ith_perr_unexpected_operand, p == PERR_UNEXPECTED_OPERAND);
    test_perr_unexpected_operand =
        test_perr_unexpected_operand && test_ith_perr_unexpected_operand;

    free(buffer.data);
  }
  LOG_TEST_STATUS(test_perr_unexpected_operand,
                  reduce(test_ith_perr_unexpected_operand, &));

  // Not an operator
  const char *test_perr_not_an_operator_inputs[] = {
      "this is certainly not an operator", "18238192", "8====D"};
  bool test_perr_not_an_operator = true;
  LOG_TEST_START(test_perr_not_an_operator);
  for (size_t i = 0; i < ARR_SIZE(test_perr_not_an_operator_inputs); ++i)
  {
    buffer_t buffer = buffer_read_cstr(
        "*test-parse-line*", test_perr_not_an_operator_inputs[i],
        strlen(test_perr_not_an_operator_inputs[i]));

    op_t op  = {0};
    perr_t p = parse_line(&buffer, &op);

    printf("\t");
    ASSERT(test_ith_perr_not_an_operator, p == PERR_ILLEGAL_OPERATOR);
    test_perr_not_an_operator =
        test_perr_not_an_operator && test_ith_perr_not_an_operator;

    free(buffer.data);
  }
  LOG_TEST_STATUS(test_perr_not_an_operator,
                  reduce(test_ith_perr_not_an_operator, &));

  // End of file
  // Simple case
  buffer   = (buffer_t){0};
  op_t op  = {0};
  perr_t p = parse_line(&buffer, &op);
  ASSERT(test_perr_simple_eof, p == PERR_EOF);

  size_t data_size = 256;
  char *data       = generate_random_data(data_size);
  buffer           = buffer_read_cstr("*test-parse-line*", data, data_size);
  free(data);

  buffer.cur = buffer.available;
  p          = parse_line(&buffer, &op);
  ASSERT(test_perr_complex_eof, p == PERR_EOF);

  free(buffer.data);

  return test_parsed_completely && test_parsed_operator &&
         test_parsed_operand && test_parsed_perr &&
         test_whitespace_parsed_completely && test_whitespace_parsed_operator &&
         test_whitespace_parsed_operand && test_whitespace_parsed_perr &&
         test_line_by_line_operator && test_line_by_line_operand &&
         test_line_by_line_perr && test_perr_no_operand &&
         test_perr_unexpected_operand && test_perr_not_an_operator &&
         test_perr_simple_eof && test_perr_complex_eof;
}

bool test_parse_buffer(void);
