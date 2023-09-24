/* test-lexer.c
 * Created: 2023-09-24
 * Author: Aryadev Chavali
 * Description: Unit tests for lexer.h
 */

#include "./test-lexer.h"
#include "../src/lexer.h"

#include <assert.h>

bool test_tokenise_one_character(void)
{
  static_assert(TOKEN_WHITESPACE == 3,
                "test_tokenise_one_character is outdated");

  // Setting up mocks
  const char *name = "test-tokenise";
  stream_t stream  = {0};
  buffer_t buffer  = {0};

  // Test eof
  bool test_eof = true;
  {
    buffer = buffer_read_cstr(name, "\0", 1);
    tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_eof);
    printf("\t");
    ASSERT(test_eof_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_eof_lexeme_first_is_eof, stream.tokens[0].type == TOKEN_EOF);
    printf("\t");
    ASSERT(test_eof_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_eof = test_eof_buffer_parsed & test_eof_lexeme_first_is_eof &
               test_eof_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_eof, test_eof_ *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test dot
  bool test_dot = true;
  {
    buffer = buffer_read_cstr(name, ".", 1);
    tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_dot);
    printf("\t");
    ASSERT(test_dot_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_dot_lexeme_first_is_dot, stream.tokens[0].type == TOKEN_DOT);
    printf("\t");
    ASSERT(test_dot_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_dot = test_dot_buffer_parsed & test_dot_lexeme_first_is_dot &
               test_dot_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_dot, test_dot_ *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test dash
  bool test_dash = true;
  {

    buffer = buffer_read_cstr(name, "-", 1);
    tokenise_buffer(&stream, &buffer);

    LOG_TEST_START(test_dash);
    printf("\t");
    ASSERT(test_dash_buffer_parsed, stream.size == 2);
    printf("\t");
    ASSERT(test_dash_lexeme_first_is_dash, stream.tokens[0].type == TOKEN_DASH);
    printf("\t");
    ASSERT(test_dash_lexeme_second_is_eof, stream.tokens[1].type == TOKEN_EOF);
    test_dash = test_dash_buffer_parsed & test_dash_lexeme_first_is_dash &
                test_dash_lexeme_second_is_eof;
    LOG_TEST_STATUS(test_dash, test_dash *);

    free(buffer.data);
    stream_free(&stream);
  }

  // Test comment

  return test_eof & test_dot & test_dash;
}
