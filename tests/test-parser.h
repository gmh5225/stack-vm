#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "./test.h"

bool test_perr_generate(void);
bool test_parse_i64(void);
bool test_parse_line(void);
bool test_parse_buffer(void);

static const test_t TEST_PARSER_SUITE[] = {
    CREATE_TEST(test_perr_generate), CREATE_TEST(test_parse_i64),
    CREATE_TEST(test_parse_line),
    /* CREATE_TEST(test_parse_buffer), */
};

#endif
