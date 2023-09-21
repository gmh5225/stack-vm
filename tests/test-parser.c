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
  static_assert(NUMBER_OF_PERRORS == 8, "test_perr_generate: Outdated!");

  // Current schema is something like "<name>:<pos> <err-cstr>" so
  // let's test that with some expected outputs.
  const char *expected_outputs_test_perr_cstr[] = {
      "*test-perr*:0: PERR_OK",
      "*test-perr*:0: PERR_EXPECTED_OPERAND",
      "*test-perr*:0: PERR_EXPECTED_LABEL",
      "*test-perr*:0: PERR_UNEXPECTED_OPERAND",
      "*test-perr*:0: PERR_UNKNOWN_LABEL",
      "*test-perr*:0: PERR_ILLEGAL_OPERATOR",
      "*test-perr*:0: PERR_ILLEGAL_INST_ADDRESS",
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
    LOG_TEST_INFO(test_ith_perr_cursor, "Expected=%s\n", expected);
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
  LOG_TEST_INFO(test_perr_filename, "Expected=%s\n",
                expected_output_test_filename);
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
  bool test_parsed_completely = true, test_parsed_immediate = true,
       test_parsed_operator = true, test_parsed_operand = true,
       test_parsed_perr = true;
  {
    // Test if immediate opcodes can be parsed
    const char *test_cases[] = {
        "noop", "halt", "push 1", "plus", "dup 1", "print",
        /* "label 1", "jmp 1",  "jmp l1" */
    };

    const op_t expected_output[] = {
        {0},
        OP_CREATE_HALT,
        OP_CREATE_PUSH(1),
        OP_CREATE_PLUS,
        OP_CREATE_DUP(1),
        OP_CREATE_PRINT,
    };

    assert(ARR_SIZE(test_cases) == OP_JUMP &&
           ARR_SIZE(test_cases) == ARR_SIZE(expected_output) &&
           "opcode_tests is outdated");

    LOG_TEST_START(test_parsed_completely);
    LOG_TEST_START(test_parsed_immediate);
    LOG_TEST_START(test_parsed_operator);
    LOG_TEST_START(test_parsed_operand);
    LOG_TEST_START(test_parsed_perr);

    for (size_t i = 0; i < ARR_SIZE(test_cases); ++i)
    {
      const char *test_data = test_cases[i];
      buffer_t push_buffer =
          buffer_read_cstr("*test-parse-line*", test_data, strlen(test_data));
      pres_t pres             = {0};
      perr_t perr             = parse_line(&push_buffer, &pres);
      const op_t expected_out = expected_output[i];

      LOG_TEST_INFO(test_ith_parsed_, "Expected=%s", "");
      op_print(expected_out, stdout);
      puts("");

      printf("\t");
      // Check that buffer is pushed to the end (completely parsed buffer)
      ASSERT(test_ith_parsed_completely,
             buffer_at_end(push_buffer) != BUFFER_OK);
      test_parsed_completely =
          test_parsed_completely && test_ith_parsed_completely;

      printf("\t");
      // Check that we got an immediate back
      ASSERT(test_ith_parsed_immediate, pres.type == PRES_IMMEDIATE);
      test_parsed_immediate =
          test_parsed_immediate && test_ith_parsed_immediate;

      if (pres.type != PRES_IMMEDIATE)
      {
        test_parsed_operator = false;
        test_parsed_operand  = false;
        test_parsed_perr     = false;
        free(push_buffer.data);
        continue;
      }

      op_t ret = pres.immediate;

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
    LOG_TEST_STATUS(test_parsed_immediate,
                    reduce(test_ith_parsed_immediate, &));
    LOG_TEST_STATUS(test_parsed_operator, reduce(test_ith_parsed_operator, &));
    LOG_TEST_STATUS(test_parsed_operand, reduce(test_ith_parsed_operand, &));
    LOG_TEST_STATUS(test_parsed_perr, reduce(test_ith_parsed_perr, &));
  }

  // Test if weird whitespace is fine
  bool test_whitespace_parsed_completely = true,
       test_whitespace_parsed_immediate  = true,
       test_whitespace_parsed_operator   = true,
       test_whitespace_parsed_operand    = true,
       test_whitespace_parsed_perr       = true;
  {
    const char *test_whitespace_inputs[]     = {"push 1", "push       20",
                                                "   push 5", "   push     20"};
    const op_t expected_whitespace_outputs[] = {
        OP_CREATE_PUSH(1), OP_CREATE_PUSH(20), OP_CREATE_PUSH(5),
        OP_CREATE_PUSH(20)};

    assert(ARR_SIZE(test_whitespace_inputs) ==
               ARR_SIZE(expected_whitespace_outputs) &&
           "whitespace_tests is outdated");

    LOG_TEST_START(test_whitespace_completely);
    LOG_TEST_START(test_whitespace_immediate);
    LOG_TEST_START(test_whitespace_operator);
    LOG_TEST_START(test_whitespace_operand);
    LOG_TEST_START(test_whitespace_perr);

    for (size_t i = 0; i < ARR_SIZE(test_whitespace_inputs); ++i)
    {
      const char *test_data = test_whitespace_inputs[i];
      buffer_t push_buffer =
          buffer_read_cstr("*test-parse-line*", test_data, strlen(test_data));
      pres_t pres             = {0};
      perr_t perr             = parse_line(&push_buffer, &pres);
      const op_t expected_out = expected_whitespace_outputs[i];

      LOG_TEST_INFO(test_ith_whitespace_parsed_, "Expected=%s", "");
      op_print(expected_out, stdout);
      puts("");

      printf("\t");
      // Check that buffer is pushed to the end (completely parsed buffer)
      ASSERT(test_ith_whitespace_parsed_completely,
             buffer_at_end(push_buffer) != BUFFER_OK);
      test_parsed_completely =
          test_parsed_completely && test_ith_whitespace_parsed_completely;

      printf("\t");
      // Check that we got an immediate back
      ASSERT(test_ith_whitespace_parsed_immediate, pres.type == PRES_IMMEDIATE);
      test_whitespace_parsed_immediate = test_whitespace_parsed_immediate &&
                                         test_ith_whitespace_parsed_immediate;

      if (pres.type != PRES_IMMEDIATE)
      {
        test_whitespace_parsed_operator = false;
        test_whitespace_parsed_operand  = false;
        test_whitespace_parsed_perr     = false;
        free(push_buffer.data);
        continue;
      }

      op_t ret = pres.immediate;

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
    LOG_TEST_STATUS(test_whitespace_parsed_immediate,
                    reduce(test_ith_whitespace_parsed_immediate, &));
    LOG_TEST_STATUS(test_whitespace_parsed_operator,
                    reduce(test_ith_whitespace_parsed_operator, &));
    LOG_TEST_STATUS(test_whitespace_parsed_operand,
                    reduce(test_ith_whitespace_parsed_operand, &));
    LOG_TEST_STATUS(test_whitespace_parsed_perr,
                    reduce(test_ith_whitespace_parsed_perr, &));
  }

  // Test if I can parse line by line
  bool test_line_by_line_immediate = true, test_line_by_line_operator = true,
       test_line_by_line_operand = true, test_line_by_line_perr = true;
  {
    const char test_line_by_line_input[]      = "push 10\n"
                                                "push 20\n"
                                                "dup 10\n";
    const op_t expected_line_by_line_output[] = {
        OP_CREATE_PUSH(10), OP_CREATE_PUSH(20), OP_CREATE_DUP(10)};

    buffer_t buffer = buffer_read_cstr("*test-perr*", test_line_by_line_input,
                                       strlen(test_line_by_line_input));

    LOG_TEST_START(test_line_by_line_immediate);
    LOG_TEST_START(test_line_by_line_operator);
    LOG_TEST_START(test_line_by_line_operand);
    LOG_TEST_START(test_line_by_line_perr);

    for (size_t i = 0; buffer_at_end(buffer) == BUFFER_OK; ++i)
    {
      pres_t pres       = {0};
      perr_t perr       = parse_line(&buffer, &pres);
      op_t expected_out = expected_line_by_line_output[i];

      LOG_TEST_INFO(test_ith_line_by_line_, "Expected=%s", "");
      op_print(expected_out, stdout);
      puts("");

      printf("\t");
      // Check that we got an immediate back
      ASSERT(test_ith_line_by_line_immediate, pres.type == PRES_IMMEDIATE);
      test_line_by_line_immediate =
          test_line_by_line_immediate && test_ith_line_by_line_immediate;

      if (pres.type != PRES_IMMEDIATE)
      {
        test_line_by_line_operator = false;
        test_line_by_line_operand  = false;
        test_line_by_line_perr     = false;
        continue;
      }

      op_t ret = pres.immediate;

      printf("\t");
      // Check that operator is correct
      ASSERT(test_ith_line_by_line_operator, ret.opcode == expected_out.opcode);
      test_line_by_line_operator =
          test_line_by_line_operator && test_ith_line_by_line_operator;

      printf("\t");
      // Check that operand is correct
      ASSERT(test_ith_line_by_line_operand,
             ret.operand == expected_out.operand);
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

    LOG_TEST_STATUS(test_line_by_line_immediate,
                    reduce(test_ith_line_by_line_parsed_immediate, &));
    LOG_TEST_STATUS(test_line_by_line_operator,
                    reduce(test_ith_line_by_line_operator, &));
    LOG_TEST_STATUS(test_line_by_line_operand,
                    reduce(test_ith_line_by_line_operand, &));
    LOG_TEST_STATUS(test_line_by_line_perr,
                    reduce(test_ith_line_by_line_perr, &));
  }

  // Test label construction
  bool test_label_no_perr = true, test_label_is_label = true,
       test_label_correct_label = true;
  {
    const char *test_label_inputs[] = {"label abc", "label my_favourite_label",
                                       "label This-Is-Also-A-Label"};
    const pres_t expected_test_label_outputs[] = {
        {.type = PRES_LABEL, .label_name = "abc"},
        {.type = PRES_LABEL, .label_name = "my_favourite_label"},
        {.type = PRES_LABEL, .label_name = "This-Is-Also-A-Label"},
    };

    LOG_TEST_START(test_label_no_perr);
    LOG_TEST_START(test_label_is_label);
    LOG_TEST_START(test_label_correct_label);
    for (size_t i = 0; i < ARR_SIZE(test_label_inputs); ++i)
    {
      const char *test      = test_label_inputs[i];
      const pres_t expected = expected_test_label_outputs[i];

      buffer_t buffer =
          buffer_read_cstr("*test-parse-line*", test, strlen(test));
      pres_t res = {0};
      perr_t p   = parse_line(&buffer, &res);

      printf("\t");
      ASSERT(test_ith_label_no_perr, p == PERR_OK);
      test_label_no_perr &= test_ith_label_no_perr;
      printf("\t");
      ASSERT(test_ith_label_is_label, res.type == PRES_LABEL);
      test_label_is_label &= test_ith_label_is_label;
      printf("\t");
      ASSERT(test_ith_label_correct_label,
             strncmp(res.label_name, expected.label_name,
                     strlen(expected.label_name)) == 0);
      test_label_correct_label &= test_ith_label_correct_label;

      free(buffer.data);
      free(res.label_name);
    }

    LOG_TEST_STATUS(test_label_no_perr, reduce(test_ith_label_no_perr, &));
    LOG_TEST_STATUS(test_label_is_label, reduce(test_ith_label_is_label, &));
    LOG_TEST_STATUS(test_label_correct_label,
                    reduce(test_ith_label_correct_label, &));
  }

  // TODO: Write test for the 3 types of jmp

  // Test absolute jumps
  bool test_jmp_absolute_no_perr = true, test_jmp_absolute_is_immediate = true,
       test_jmp_absolute_correct_address = true;
  {
    const char *inputs[]         = {"jmp 200", "jmp 150", "jmp 2"};
    const i64 expected_outputs[] = {200, 150, 2};

    LOG_TEST_START(test_jmp_absolute_no_perr);
    LOG_TEST_START(test_jmp_absolute_is_immediate);
    LOG_TEST_START(test_jmp_absolute_correct_address);

    for (size_t i = 0; i < ARR_SIZE(inputs); ++i)
    {
      const char *input  = inputs[i];
      const i64 expected = expected_outputs[i];
      buffer_t buf =
          buffer_read_cstr("*test-parse-line*", input, strlen(input));
      pres_t pres = {0};
      perr_t perr = parse_line(&buf, &pres);

      printf("\t");
      ASSERT(test_ith_no_perr, perr == PERR_OK);
      test_jmp_absolute_no_perr &= test_ith_no_perr;

      printf("\t");
      ASSERT(test_ith_is_immediate, pres.type == PRES_IMMEDIATE);
      test_jmp_absolute_is_immediate &= test_ith_is_immediate;

      printf("\t");
      ASSERT(test_ith_correct_addr, pres.immediate.operand == expected);
      test_jmp_absolute_correct_address &= test_ith_correct_addr;

      free(buf.data);
    }

    LOG_TEST_STATUS(test_jmp_absolute_no_perr, reduce(test_ith_no_perr, &));
    LOG_TEST_STATUS(test_jmp_absolute_is_immediate,
                    reduce(test_ith_is_immediate, &));
    LOG_TEST_STATUS(test_jmp_absolute_correct_address,
                    reduce(test_ith_correct_address, &));
  }

  // Test relative jumps
  bool test_jmp_relative_no_perr = true, test_jmp_relative_is_relative = true,
       test_jmp_relative_correct_address = true;
  {
    const char *test_inputs[]    = {"jmp .200", "jmp .-150", "jmp .2"};
    const i64 expected_outputs[] = {200, -150, 2};

    LOG_TEST_START(test_jmp_relative_no_perr);
    LOG_TEST_START(test_jmp_relative_is_relative);
    LOG_TEST_START(test_jmp_relative_correct_address);

    for (size_t i = 0; i < ARR_SIZE(test_inputs); ++i)
    {
      const char *input  = test_inputs[i];
      const i64 expected = expected_outputs[i];
      buffer_t buf =
          buffer_read_cstr("*test-parse-line*", input, strlen(input));
      pres_t pres = {0};
      perr_t perr = parse_line(&buf, &pres);

      printf("\t");
      ASSERT(test_ith_no_perr, perr == PERR_OK);
      test_jmp_relative_no_perr &= test_ith_no_perr;

      printf("\t");
      ASSERT(test_ith_is_relative, pres.type == PRES_JUMP_RELATIVE);
      test_jmp_relative_is_relative &= test_ith_is_relative;

      printf("\t");
      ASSERT(test_ith_correct_address, pres.relative_jump_operand == expected);
      test_jmp_relative_correct_address &= test_ith_correct_address;

      free(buf.data);
    }

    LOG_TEST_STATUS(test_jmp_relative_no_perr, reduce(test_ith_no_perr, &));
    LOG_TEST_STATUS(test_jmp_relative_is_relative,
                    reduce(test_ith_is_relative, &));
    LOG_TEST_STATUS(test_jmp_relative_correct_address,
                    reduce(test_ith_correct_address, &));
  }

  // Test label jumps

  bool test_jmp_label_no_perr = true, test_jmp_label_is_label = true,
       test_jmp_label_correct_label = true;
  {
    const char *test_inputs[]      = {"jmp label", "jmp another-label",
                                      "jmp this_IS-also_A-label"};
    const char *expected_outputs[] = {"label", "another-label",
                                      "this_IS-also_A-label"};

    LOG_TEST_START(test_jmp_label_no_perr);
    LOG_TEST_START(test_jmp_label_is_label);
    LOG_TEST_START(test_jmp_label_correct_address);

    for (size_t i = 0; i < ARR_SIZE(test_inputs); ++i)
    {
      const char *input    = test_inputs[i];
      const char *expected = expected_outputs[i];
      buffer_t buf =
          buffer_read_cstr("*test-parse-line*", input, strlen(input));
      pres_t pres = {0};
      perr_t perr = parse_line(&buf, &pres);

      printf("\t");
      ASSERT(test_ith_no_perr, perr == PERR_OK);
      test_jmp_relative_no_perr &= test_ith_no_perr;

      printf("\t");
      ASSERT(test_ith_is_label, pres.type == PRES_JUMP_LABEL);
      test_jmp_relative_is_relative &= test_ith_is_label;

      printf("\t");
      ASSERT(test_ith_correct_label,
             strncmp(pres.label_name, expected, strlen(expected)) == 0);
      test_jmp_label_correct_label &= test_ith_correct_label;

      free(buf.data);
      free(pres.label_name);
    }

    LOG_TEST_STATUS(test_jmp_label_no_perr, reduce(test_ith_no_perr, &));
    LOG_TEST_STATUS(test_jmp_label_is_label, reduce(test_ith_is_label, &));
    LOG_TEST_STATUS(test_jmp_label_correct_label,
                    reduce(test_ith_correct_address, &));
  }

  // Now test for error states

  // No operand
  bool test_perr_no_operand = true;
  {
    const char *test_perr_no_operand_inputs[] = {"push", "dup"};
    LOG_TEST_START(test_perr_no_operand);
    for (size_t i = 0; i < ARR_SIZE(test_perr_no_operand_inputs); ++i)
    {
      buffer_t buffer =
          buffer_read_cstr("*test-parse-line*", test_perr_no_operand_inputs[i],
                           strlen(test_perr_no_operand_inputs[i]));

      pres_t res = {0};
      perr_t p   = parse_line(&buffer, &res);

      LOG_TEST_INFO(test_ith_line_by_line_, "Expected=%s\n", perr_as_cstr(p));
      printf("\t");
      ASSERT(test_ith_perr_no_operand, p == PERR_EXPECTED_OPERAND);
      test_perr_no_operand = test_perr_no_operand && test_ith_perr_no_operand;

      free(buffer.data);
    }
    LOG_TEST_STATUS(test_perr_no_operand, reduce(test_ith_perr_no_operand, &));
  }

  // Unexpected operand
  bool test_perr_unexpected_operand = true;
  {
    const char *test_perr_unexpected_operand_inputs[] = {"halt 1", "plus 2",
                                                         "print 3"};

    LOG_TEST_START(test_perr_unexpected_operand);
    for (size_t i = 0; i < ARR_SIZE(test_perr_unexpected_operand_inputs); ++i)
    {
      buffer_t buffer = buffer_read_cstr(
          "*test-parse-line*", test_perr_unexpected_operand_inputs[i],
          strlen(test_perr_unexpected_operand_inputs[i]));

      pres_t pres = {0};
      perr_t p    = parse_line(&buffer, &pres);

      LOG_TEST_INFO(test_ith_line_by_line_, "Expected=%s\n", perr_as_cstr(p));
      printf("\t");
      ASSERT(test_ith_perr_unexpected_operand, p == PERR_UNEXPECTED_OPERAND);
      test_perr_unexpected_operand =
          test_perr_unexpected_operand && test_ith_perr_unexpected_operand;

      free(buffer.data);
    }
    LOG_TEST_STATUS(test_perr_unexpected_operand,
                    reduce(test_ith_perr_unexpected_operand, &));
  }

  // Not an operator
  bool test_perr_not_an_operator = true;
  {
    const char *test_perr_not_an_operator_inputs[] = {
        "this is certainly not an operator", "18238192", "8====D"};
    LOG_TEST_START(test_perr_not_an_operator);
    for (size_t i = 0; i < ARR_SIZE(test_perr_not_an_operator_inputs); ++i)
    {
      buffer_t buffer = buffer_read_cstr(
          "*test-parse-line*", test_perr_not_an_operator_inputs[i],
          strlen(test_perr_not_an_operator_inputs[i]));

      pres_t pres = {0};
      perr_t p    = parse_line(&buffer, &pres);

      LOG_TEST_INFO(test_ith_line_by_line_, "Expected=%s\n", perr_as_cstr(p));
      printf("\t");
      ASSERT(test_ith_perr_not_an_operator, p == PERR_ILLEGAL_OPERATOR);
      test_perr_not_an_operator =
          test_perr_not_an_operator && test_ith_perr_not_an_operator;

      free(buffer.data);
    }
    LOG_TEST_STATUS(test_perr_not_an_operator,
                    reduce(test_ith_perr_not_an_operator, &));
  }

  // An illegal jump address from an absolute jump
  bool test_perr_illegal_jump = true;
  {
    const char *inputs[] = {"jmp -200", "jmp -4000", "jmp -0.25"};
    LOG_TEST_START(test_perr_illegal_jump);
    for (size_t i = 0; i < ARR_SIZE(inputs); ++i)
    {
      buffer_t buffer =
          buffer_read_cstr("*test-parse-line*", inputs[i], strlen(inputs[i]));
      pres_t pres = {0};
      perr_t perr = parse_line(&buffer, &pres);
      ASSERT(test_ith_illegal_jump, perr == PERR_ILLEGAL_INST_ADDRESS);
      test_perr_illegal_jump &= test_ith_illegal_jump;

      free(buffer.data);
    }
    LOG_TEST_STATUS(test_perr_illegal_jump, reduce(test_ith_illegal_jump, &));
  }

  // End of file
  // Simple case
  buffer_t buffer = (buffer_t){0};
  pres_t pres     = {0};
  perr_t p        = parse_line(&buffer, &pres);
  ASSERT(test_perr_simple_eof, p == PERR_EOF);

  size_t data_size = 256;
  char *data       = generate_random_data(data_size);
  buffer           = buffer_read_cstr("*test-parse-line*", data, data_size);
  free(data);

  buffer.cur = buffer.available;
  p          = parse_line(&buffer, &pres);
  ASSERT(test_perr_complex_eof, p == PERR_EOF);

  free(buffer.data);

  return test_parsed_completely && test_parsed_operator &&
         test_parsed_operand && test_parsed_perr && test_parsed_immediate &&

         test_whitespace_parsed_completely && test_whitespace_parsed_operator &&
         test_whitespace_parsed_operand && test_whitespace_parsed_perr &&
         test_whitespace_parsed_immediate &&

         test_line_by_line_operator && test_line_by_line_operand &&
         test_line_by_line_perr && test_line_by_line_immediate &&

         test_label_no_perr && test_label_is_label &&
         test_label_correct_label &&

         test_jmp_absolute_no_perr && test_jmp_absolute_is_immediate &&
         test_jmp_absolute_correct_address &&

         test_jmp_relative_no_perr && test_jmp_relative_is_relative &&
         test_jmp_relative_correct_address &&

         test_jmp_label_no_perr && test_jmp_label_is_label &&
         test_jmp_label_correct_label &&

         test_perr_no_operand && test_perr_unexpected_operand &&
         test_perr_not_an_operator && test_perr_illegal_jump &&
         test_perr_simple_eof && test_perr_complex_eof;
}

bool test_parse_buffer(void);
