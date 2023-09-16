/* test-lib.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for lib.h
 */

#include "./test-lib.h"
#include "./test.h"

#include "../lib.h"

bool test_lib_MAX(void)
{
  ASSERT(test_1, MAX(0, 1) == 1);
  ASSERT(test_2, MAX(-2, 2) == 2);
  return test_1 && test_2;
}

bool test_lib_MIN(void)
{
  ASSERT(test_1, (MIN(0, 1) == 0));
  ASSERT(test_2, (MIN(-2, 2) == -2));
  return test_1 && test_2;
}

bool test_lib_ARR_SIZE(void)
{
  const char test_str[] = "This string is 29 characters";
  ASSERT(test_1, ARR_SIZE(test_str) == 29);

  const size_t test_ints_size = rand() % 1024;
  int64_t test_ints[test_ints_size];
  printf("\t\t[INFO]: test_lib_ARR_SIZE: test_ints=%lu\n", test_ints_size);
  ASSERT(test_2, ARR_SIZE(test_ints) == test_ints_size);
  return test_1 && test_2;
}
