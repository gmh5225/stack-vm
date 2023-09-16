#ifndef TEST_H
#define TEST_H

#include <stdbool.h>

typedef bool (*test_fn)(void);
typedef test_fn *test_suite;
typedef struct Test
{
  const char *name;
  test_fn fn;
} test_t;

#define TERM_RED   "\x1b[31m"
#define TERM_GREEN "\x1b[32m"
#define TERM_RESET "\x1b[0m"

#define BOOL_STATUS_TO_STR(b) \
  (b ? TERM_GREEN "PASSED" TERM_RESET : TERM_RED "FAILED" TERM_RESET)

#define ASSERT(RET_NAME, COND) \
  bool RET_NAME = (COND);      \
  if (!RET_NAME)               \
    printf("\t\t[" TERM_RED "FAILED" TERM_RESET "]: %s\n", #COND);

#define CREATE_TEST(NAME) \
  {                       \
    #NAME, NAME           \
  }

#endif
