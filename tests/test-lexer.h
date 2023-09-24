#ifndef TEST_LEXER_H
#define TEST_LEXER_H

#include "./test.h"

bool test_tokenise_one_character(void);
bool test_tokenise_whitespace(void);
bool test_tokenise_symbol(void);
bool test_tokenise_character_literal(void);
bool test_tokenise_number_literal(void);
bool test_tokenise_errors(void);

static const test_t TEST_LEXER_SUITE[] = {
    CREATE_TEST(test_tokenise_one_character),
    /* CREATE_TEST(test_tokenise_whitespace), */
    /* CREATE_TEST(test_tokenise_symbol), */
    /* CREATE_TEST(test_tokenise_character_literal), */
    /* CREATE_TEST(test_tokenise_number_literal), */
    /* CREATE_TEST(test_tokenise_errors), */
};
#endif
