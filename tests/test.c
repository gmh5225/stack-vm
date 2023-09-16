/* test.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for program
 */

#include "../op.h"
#include "../parser.h"
#include "../vm.h"

#include "./test-lib.h"
#include "./test.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void run_test_suite(const char *suite_name, const test_t *tests, size_t n_tests)
{
  printf("[START]: Suite(%s)\n", suite_name);
  bool suite_passed = true;
  for (size_t i = 0; i < n_tests; ++i)
  {
    bool passed = tests[i].fn();
    printf("\t[%s #%lu]: `%s`\n", BOOL_STATUS_TO_STR(passed), i, tests[i].name);
    suite_passed = suite_passed && passed;
  }
  printf("[%s]: Suite(%s)\n", BOOL_STATUS_TO_STR(suite_passed), suite_name);
}

char *generate_random_data(size_t number)
{
  char *mem = calloc(number + 1, sizeof(mem[0]));
  for (size_t i = 0; i < number; ++i)
    mem[i] = rand() % 255;
  mem[number] = '\0';
  return mem;
}

/* Lib testing */
int main(void)
{
  srand(time(NULL));
  run_test_suite("LIB", TEST_LIB_SUITE, ARR_SIZE(TEST_LIB_SUITE));
  return 0;
}
