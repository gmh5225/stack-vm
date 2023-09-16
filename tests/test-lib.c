/* test-lib.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for lib.h
 */

#include "./test-lib.h"
#include "./test.h"

#include "../lib.h"

#include <errno.h>
#include <string.h>

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

bool test_lib_buffer_read_file(void)
{
  // Create a mock file (using a random number to ensure we're not
  // conflicting with something in the workspace
  const char text[] = "This is some text, I hope buffer will slurp it all!\n"
                      "Here is another line\n"
                      "And my final line\n";

  size_t n = rand() % 10;
  char filepath[30];
  sprintf(filepath, "tests/TEST_LIB_MOCK_FILE%lu.txt", n);
  FILE *fp = fopen(filepath, "w");
  if (!fp)
  {
    printf("\t\t[INFO]: test_lib_buffer_read_file: Could not open file (to "
           "write) `%s`: "
           "%s\n",
           filepath, strerror(errno));
    return false;
  }
  fwrite(text, 1, ARR_SIZE(text), fp);
  fclose(fp);

  fp = fopen(filepath, "r");
  if (!fp)
  {
    printf("\t\t[INFO]: test_lib_buffer_read_file: Could not open file (to "
           "read) `%s`: "
           "%s\n",
           filepath, strerror(errno));
    return false;
  }
  buffer_t buf = buffer_read_file(filepath, fp);
  fclose(fp);

  ASSERT(test_1, buf.available == ARR_SIZE(text));
  ASSERT(test_2, strncmp(buf.data, text, ARR_SIZE(text)) == 0);

  free(buf.data);
  return test_1 && test_2;
}
