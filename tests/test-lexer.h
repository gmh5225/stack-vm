#ifndef TEST_LEXER_H
#define TEST_LEXER_H

#include "./test.h"

bool test_tokenise_one_character(void);
bool test_tokenise_whitespace(void);
bool test_tokenise_symbol(void);
bool test_tokenise_character(void);
bool test_tokenise_number(void);
bool test_tokenise_comments(void);

static const test_t TEST_LEXER_SUITE[] = {
    CREATE_TEST(test_tokenise_one_character),
    CREATE_TEST(test_tokenise_whitespace),
    CREATE_TEST(test_tokenise_symbol),
    CREATE_TEST(test_tokenise_character),
    CREATE_TEST(test_tokenise_number),
    CREATE_TEST(test_tokenise_comments),
};
#endif
