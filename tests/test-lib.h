#ifndef TEST_LIB_H
#define TEST_LIB_H

#include "./test.h"

bool test_lib_MAX(void);
bool test_lib_MIN(void);
bool test_lib_ARR_SIZE(void);

bool test_lib_buffer_read_file(void);
bool test_lib_buffer_read_cstr(void);
bool test_lib_buffer_peek(void);
bool test_lib_buffer_seek_next(void);
bool test_lib_buffer_seek_nextline(void);
bool test_lib_buffer_at_end(void);
bool test_lib_buffer_space_left(void);

bool test_lib_darr_ensure_capacity(void);
bool test_lib_darr_tighten(void);

static const test_t TEST_LIB_SUITE[] = {
    CREATE_TEST(test_lib_MAX), CREATE_TEST(test_lib_MIN),
    CREATE_TEST(test_lib_ARR_SIZE),
    /* CREATE_TEST(test_lib_buffer_read_file), */
    /* CREATE_TEST(test_lib_buffer_read_cstr), */
    /* CREATE_TEST(test_lib_buffer_peek), */
    /* CREATE_TEST(test_lib_buffer_seek_next), */
    /* CREATE_TEST(test_lib_buffer_seek_nextline), */
    /* CREATE_TEST(test_lib_buffer_at_end), */
    /* CREATE_TEST(test_lib_buffer_space_left), */
    /* CREATE_TEST(test_lib_darr_ensure_capacity), */
    /* CREATE_TEST(test_lib_darr_tighten), */
};

#endif
