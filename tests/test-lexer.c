/* test-lexer.c
 * Created: 2023-09-24
 * Author: Aryadev Chavali
 * Description: Unit tests for lexer.h
 */

#include "./test-lexer.h"
#include "../src/lexer.h"

#include <assert.h>
#include <string.h>

bool test_tokenise_one_character(void)
{
  static_assert(TOKEN_WHITESPACE == 3,
                "test_tokenise_one_character is outdated");

  // Setting up mocks
  const char *name = "*test-one-character*";
  stream_t stream  = {0};
  buffer_t buffer  = {0};

  // Test eof
  bool test_eof = true;
  {
    buffer      = buffer_read_cstr(name, "\0", 1);
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_eof);
    printf("\t");
    ASSERT(test_eof_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_eof_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_eof_lexeme_first_is_eof, stream.tokens[0].type == TOKEN_EOF);
    printf("\t");
    ASSERT(test_eof_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_eof = test_eof_no_lerr & test_eof_buffer_parsed &
               test_eof_lexeme_first_is_eof & test_eof_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_eof, test_eof_ *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test dot
  bool test_dot = true;
  {
    buffer      = buffer_read_cstr(name, ".", 1);
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_dot);
    printf("\t");
    ASSERT(test_dot_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_dot_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_dot_lexeme_first_is_dot, stream.tokens[0].type == TOKEN_DOT);
    printf("\t");
    ASSERT(test_dot_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_dot = test_dot_no_lerr & test_dot_buffer_parsed &
               test_dot_lexeme_first_is_dot & test_dot_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_dot, test_dot_ *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test dash
  bool test_dash = true;
  {

    buffer      = buffer_read_cstr(name, "-", 1);
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_dash);
    printf("\t");
    ASSERT(test_dash_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_dash_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_dash_lexeme_first_is_dash, stream.tokens[0].type == TOKEN_DASH);
    printf("\t");
    ASSERT(test_dash_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_dash = test_dash_no_lerr & test_dash_buffer_parsed &
                test_dash_lexeme_first_is_dash & test_dash_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_dash, test_dash *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test comment

  return test_eof & test_dot & test_dash;
}

bool test_tokenise_whitespace(void)
{
  const char *name = "*test-whitespace*";
  buffer_t buffer  = {0};
  stream_t stream  = {0};
  // Test variety of whitespace
  bool test_whitespace_variety = true;
  LOG_TEST_START(test_whitespace_variety);
  {
    const char *test_input = "    \n   \n\t\t     \r\f\n   \v";
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_whitespace_variety_no_lerr, lerr == LERR_OK);

    printf("\t");
    ASSERT(test_whitespace_variety_only_whitespace,
           stream.size == 2 && stream.tokens[0].type == TOKEN_WHITESPACE);

    test_whitespace_variety = test_whitespace_variety_only_whitespace &
                              test_whitespace_variety_no_lerr;

    free(buffer.data);
    stream_free(&stream);
  }
  LOG_TEST_STATUS(test_whitespace_variety, _);

  // Test chunks of whitespace
  bool test_whitespace_chunks = true;
  LOG_TEST_START(test_whitespace_chunks);
  {
    const char *test_input =
        "      \nthis-is-some-text     \nnext-chunk\n\t\tfinal-chunk";
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_whitespace_chunks_no_lerr, lerr == LERR_OK);

    printf("\t");
    ASSERT(test_whitespace_chunks_seven_chunks, stream.size == 7);

    printf("\t");
    ASSERT(test_whitespace_chunks_first_token_is_whitespace,
           stream.tokens[0].type == TOKEN_WHITESPACE);
    printf("\t");
    ASSERT(test_whitespace_chunks_third_token_is_whitespace,
           stream.tokens[2].type == TOKEN_WHITESPACE);
    printf("\t");
    ASSERT(test_whitespace_chunks_fifth_token_is_whitespace,
           stream.tokens[4].type == TOKEN_WHITESPACE);

    printf("\t");
    ASSERT(test_whitespace_chunks_first_whitespace_col_line,
           stream.tokens[0].column == 0 && stream.tokens[0].line == 1);
    printf("\t");
    ASSERT(test_whitespace_chunks_third_whitespace_col_line,
           stream.tokens[2].column == 17 && stream.tokens[2].line == 2);
    printf("\t");
    ASSERT(test_whitespace_chunks_fifth_whitespace_col_line,
           stream.tokens[4].column == 10 && stream.tokens[4].line == 3);

    test_whitespace_chunks = test_whitespace_chunks_no_lerr &
                             test_whitespace_chunks_seven_chunks &
                             test_whitespace_chunks_first_token_is_whitespace &
                             test_whitespace_chunks_third_token_is_whitespace &
                             test_whitespace_chunks_fifth_token_is_whitespace &
                             test_whitespace_chunks_first_whitespace_col_line &
                             test_whitespace_chunks_third_whitespace_col_line &
                             test_whitespace_chunks_fifth_whitespace_col_line;

    free(buffer.data);
    stream_free(&stream);
  }
  LOG_TEST_STATUS(test_whitespace_chunks, _);
  return test_whitespace_variety & test_whitespace_chunks;
}

bool test_tokenise_symbol(void)
{
  const char name[] = "*test-symbol*";
  buffer_t buffer   = {0};
  stream_t stream   = {0};

  const char *test_input =
      "this_is_a_symbol This-Is-Also-A-Symbol\nAnOThEr_SYmB-ol";
  const char *expected_symbols[] = {"this_is_a_symbol", "This-Is-Also-A-Symbol",
                                    "AnOThEr_SYmB-ol"};
  buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
  lerr_t lerr = tokenise_buffer(&stream, &buffer);

  ASSERT(test_symbol_no_lerr, lerr == LERR_OK);
  ASSERT(test_symbol_expected_number_tokens, stream.size == 6);

  ASSERT(test_symbol_first_is_symbol, stream.tokens[0].type == TOKEN_SYMBOL);
  ASSERT(test_symbol_first_is_correct_symbol,
         strncmp(stream.tokens[0].content, expected_symbols[0],
                 stream.tokens[0].size) == 0);
  ASSERT(test_symbol_first_in_correct_column, stream.tokens[0].column == 0);
  ASSERT(test_symbol_first_in_correct_line, stream.tokens[0].line == 1);

  ASSERT(test_symbol_third_is_symbol, stream.tokens[2].type == TOKEN_SYMBOL);
  ASSERT(test_symbol_third_is_correct_symbol,
         strncmp(stream.tokens[2].content, expected_symbols[1],
                 stream.tokens[2].size) == 0);
  ASSERT(test_symbol_third_in_correct_column, stream.tokens[2].column == 17);
  ASSERT(test_symbol_third_in_correct_line, stream.tokens[2].line == 1);

  ASSERT(test_symbol_fifth_is_symbol, stream.tokens[4].type == TOKEN_SYMBOL);
  ASSERT(test_symbol_fifth_is_correct_symbol,
         strncmp(stream.tokens[4].content, expected_symbols[2],
                 stream.tokens[4].size) == 0);
  ASSERT(test_symbol_fifth_in_correct_column, stream.tokens[4].column == 0);
  ASSERT(test_symbol_fifth_in_correct_line, stream.tokens[4].line == 2);

  free(buffer.data);
  stream_free(&stream);

  return test_symbol_no_lerr & test_symbol_expected_number_tokens &
         test_symbol_first_is_symbol & test_symbol_first_is_correct_symbol &
         test_symbol_third_is_symbol & test_symbol_third_is_correct_symbol &
         test_symbol_fifth_is_symbol & test_symbol_fifth_is_correct_symbol &
         test_symbol_first_in_correct_column &
         test_symbol_first_in_correct_line &
         test_symbol_third_in_correct_column &
         test_symbol_third_in_correct_line &
         test_symbol_fifth_in_correct_column &
         test_symbol_fifth_in_correct_line;
}

bool test_tokenise_character(void)
{
  const char name[] = "*test-character*";
  buffer_t buffer   = {0};
  stream_t stream   = {0};

  bool test_character_escape = true;
  LOG_TEST_START(test_character_escape);
  {
    const char *test_input = "'\\n' '\\t' '\\r' '\\v' '\\f'";
    const char expected[]  = {'\n', '\t', '\r', '\v', '\f'};
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_escape_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_escape_expected_number, stream.size == ARR_SIZE(expected) * 2);
    bool test_escape_expected_escapes = true;
    for (size_t i = 0, j = 0; i < stream.size; i += 2, j += 1)
    {
      printf("\t\t");
      ASSERT(test_ith_escape_type, stream.tokens[i].type == TOKEN_CHARACTER);
      printf("\t\t");
      ASSERT(test_ith_escape_expected_literal,
             stream.tokens[i].content[0] == expected[j]);

      test_escape_expected_escapes *=
          test_ith_escape_type & test_ith_escape_expected_literal;
    }

    free(buffer.data);
    stream_free(&stream);

    test_character_escape = test_escape_no_lerr & test_escape_expected_number &
                            test_escape_expected_escapes;
  }
  LOG_TEST_STATUS(test_character_escape, _);

  bool test_character_general = true;
  LOG_TEST_START(test_character_general);
  {
    const char *test_input = "'a' 'z' 'A' 'Z' '0' '9' '~'";
    const char expected[]  = {'a', 'z', 'A', 'Z', '0', '9', '~'};
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_general_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_general_expected_number, stream.size == ARR_SIZE(expected) * 2);
    bool test_general_expected_characters = true;
    for (size_t i = 0, j = 0; i < stream.size; i += 2, j += 1)
    {
      printf("\t\t");
      ASSERT(test_ith_general_type, stream.tokens[i].type == TOKEN_CHARACTER);
      printf("\t\t");
      ASSERT(test_ith_general_expected_literal,
             stream.tokens[i].content[0] == expected[j]);

      test_general_expected_characters *=
          test_ith_general_type & test_ith_general_expected_literal;
    }

    free(buffer.data);
    stream_free(&stream);

    test_character_general = test_general_no_lerr &
                             test_general_expected_number &
                             test_general_expected_characters;
  }
  LOG_TEST_STATUS(test_character_general, _);
  return test_character_escape & test_character_general;
}

bool test_tokenise_number(void)
{
  const char name[] = "*test-number*";
  buffer_t buffer   = {0};
  stream_t stream   = {0};

  // Integral values
  bool test_integral_values = true;
  LOG_TEST_START(test_integral_values);
  {
    const char *test_input =
        "1 100 1000 1180591620717411303424 -1 -100 -1000 -3141592653";
    const char *expected_output[] = {
        "1",  "100",  "1000",  "1180591620717411303424",
        "-1", "-100", "-1000", "-3141592653"};
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_integral_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_integral_expected_amount,
           stream.size == ARR_SIZE(expected_output) * 2);

    bool test_integral_expected_numbers = true;
    for (size_t i = 0, j = 0; i < stream.size; i += 2, j += 1)
    {
      printf("\t\t");
      ASSERT(test_ith_integral_type, stream.tokens[i].type == TOKEN_NUMBER);
      printf("\t\t");
      ASSERT(test_ith_integral_expected_literal,
             strncmp(stream.tokens[i].content, expected_output[j],
                     stream.tokens[i].size) == 0);

      test_integral_expected_numbers *=
          test_ith_integral_type & test_ith_integral_expected_literal;
    }

    free(buffer.data);
    stream_free(&stream);

    test_integral_values = test_integral_no_lerr &
                           test_integral_expected_amount &
                           test_integral_expected_numbers;
  }
  LOG_TEST_STATUS(test_integral_values, _);
  // Floating point/decimal values
  bool test_floating_point_values = true;
  LOG_TEST_START(test_floating_point_values);
  {
    const char *test_input = "3.141592653 1.4142135623730951 2023.2609 -420.69";
    const char *expected_output[] = {"3.141592653", "1.4142135623730951",
                                     "2023.2609", "-420.69"};
    buffer      = buffer_read_cstr(name, test_input, strlen(test_input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);

    printf("\t");
    ASSERT(test_floating_point_no_lerr, lerr == LERR_OK);
    printf("\t");
    ASSERT(test_floating_point_expected_amount,
           stream.size == ARR_SIZE(expected_output) * 2);

    bool test_floating_point_expected_numbers = true;
    for (size_t i = 0, j = 0; i < stream.size; i += 2, j += 1)
    {
      printf("\t\t");
      ASSERT(test_ith_floating_point_type,
             stream.tokens[i].type == TOKEN_NUMBER);
      printf("\t\t");
      ASSERT(test_ith_floating_point_expected_literal,
             strncmp(stream.tokens[i].content, expected_output[j],
                     stream.tokens[i].size) == 0);

      test_floating_point_expected_numbers *=
          test_ith_floating_point_type &
          test_ith_floating_point_expected_literal;
    }

    free(buffer.data);
    stream_free(&stream);

    test_floating_point_values = test_floating_point_no_lerr &
                                 test_floating_point_expected_amount &
                                 test_floating_point_expected_numbers;
  }
  LOG_TEST_STATUS(test_floating_point_values, _);

  return test_integral_values & test_floating_point_values;
}

bool test_tokenise_comments(void)
{
  const char *name = "test-comments";
  buffer_t buffer  = {0};
  stream_t stream  = {0};

  bool test_comment_doc_string = true;
  LOG_TEST_START(test_comment_doc_string);
  {

    const char *input = "#this is a comment\n"
                        "# another comment with    a    lot   of   space";

    const char *expected_comments[] = {
        "this is a comment", " another comment with    a    lot   of   space"};

    buffer      = buffer_read_cstr(name, input, strlen(input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);
    printf("\t");
    ASSERT(test_comment_doc_string_no_lerr, lerr == LERR_OK);
    test_comment_doc_string &= test_comment_doc_string_no_lerr;
    for (size_t i = 0, j = 0; i < stream.size; i += 2, ++j)
    {
      printf("\t");
      ASSERT(test_ith_comment_doc_string,
             strncmp(stream.tokens[i].content, expected_comments[j],
                     stream.tokens[i].size) == 0);
      test_comment_doc_string &= test_ith_comment_doc_string;
    }

    free(buffer.data);
    stream_free(&stream);
  }
  LOG_TEST_STATUS(test_comment_doc_string, _);

  bool test_comment_inline = true;
  LOG_TEST_START(test_comment_inline);
  {
    const char *input               = "a-symbol#with a comment\n"
                                      "2000 #a number with a comment\n"
                                      "push 10 # push 10 onto the stack\n";
    const char *expected_comments[] = {
        "with a comment", "a number with a comment", " push 10 onto the stack"};

    buffer      = buffer_read_cstr(name, input, strlen(input));
    lerr_t lerr = tokenise_buffer(&stream, &buffer);
    ASSERT(test_comment_inline_no_lerr, lerr == LERR_OK);
    test_comment_inline &= test_comment_inline_no_lerr;

    for (size_t i = 0, j = 0; i < stream.size; ++i)
    {
      if (stream.tokens[i].type == TOKEN_COMMENT)
      {
        printf("\t");
        ASSERT(test_ith_comment_inline,
               strncmp(stream.tokens[i].content, expected_comments[j++],
                       stream.tokens[i].size) == 0);
        test_comment_inline &= test_ith_comment_inline;
      }
    }

    free(buffer.data);
    stream_free(&stream);
  }
  LOG_TEST_STATUS(test_comment_inline, _);
  return test_comment_doc_string & test_comment_inline;
}
