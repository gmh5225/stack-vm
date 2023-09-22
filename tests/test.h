#ifndef TEST_H
#define TEST_H

#include "../src/lib.h"

#include <stdbool.h>
#include <stdlib.h>

#define VERBOSE_LOGS 1

typedef bool (*test_fn)(void);
typedef test_fn *test_suite;
typedef struct Test
{
  const char *name;
  test_fn fn;
} test_t;

char *generate_random_data(size_t number);
char *generate_random_text(size_t characters, size_t lines);

#define BOOL_STATUS_TO_STR(b) \
  (b ? TERM_GREEN "PASSED" TERM_RESET : TERM_RED "FAILED" TERM_RESET)

#define LOG_TEST_INFO(TEST_NAME, FORMAT_STRING, ...) \
  printf("\t\t\t[INFO]: %s: " FORMAT_STRING, #TEST_NAME, __VA_ARGS__)

#define LOG_TEST_START(TEST_NAME) \
  printf("\t\t[TEST]: Starting `" #TEST_NAME "`\n")

#define ASSERT(TEST_NAME, COND) \
  bool TEST_NAME = (COND);      \
  LOG_TEST_STATUS(TEST_NAME, COND)

#define LOG_TEST_STATUS(TEST_NAME, COND)                                       \
  if (!TEST_NAME)                                                              \
    printf("\t\t[" TERM_RED "FAILED" TERM_RESET "]: `%s` -> %s\n", #TEST_NAME, \
           #COND);                                                             \
  else                                                                         \
    printf("\t\t[" TERM_GREEN "PASSED" TERM_RESET "]: `%s` -> %s\n",           \
           #TEST_NAME, #COND);

#define CREATE_TEST(NAME) \
  {                       \
    #NAME, NAME           \
  }

#endif
