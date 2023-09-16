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

char *generate_random_text(size_t characters, size_t newlines)
{
  const char table[] = {
      'a',  'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n',  'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      'A',  'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N',  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      '\n', '.', ',', ';', ':', '=', '+', '#', '~',
  };
  char *mem = calloc(characters + 1, sizeof(mem[0]));
  for (size_t i = 0; i < characters; ++i)
  {
    size_t ret = 0;
    if (newlines != 0)
    {
      // Bias the probability towards newlines
      ret = rand() % (2 * ARR_SIZE(table));
      if (ret >= ARR_SIZE(table))
        ret = '\n';
      else
        ret = table[ret];
    }
    else
      ret = table[rand() % ARR_SIZE(table)];

    if (ret == '\n')
    {
      if (newlines == 0)
        ret = table[ARR_SIZE(table) - 1];
      else
        --newlines;
    }
    mem[i] = ret;
  }
  mem[characters] = '\0';
  return mem;
}

/* Lib testing */
int main(void)
{
  srand(time(NULL));
  run_test_suite("LIB", TEST_LIB_SUITE, ARR_SIZE(TEST_LIB_SUITE));
  return 0;
}
