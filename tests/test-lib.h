#ifndef TEST_LIB_H
#define TEST_LIB_H

#include "./test.h"

bool test_lib_MAX(void);
bool test_lib_MIN(void);
bool test_lib_ARR_SIZE(void);

static const test_t TEST_LIB_SUITE[] = {
    CREATE_TEST(test_lib_MAX),
    CREATE_TEST(test_lib_MIN),
    CREATE_TEST(test_lib_ARR_SIZE),
};

#endif
