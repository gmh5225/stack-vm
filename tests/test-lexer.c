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
