/* test-lib.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Unit tests for lib.h
 */

#include "./test-lib.h"
#include "./test.h"

#include "../lib.h"

#include <ctype.h>
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

bool test_lib_buffer_read_file_fixed(size_t text_size)
{
  char *text = generate_random_data(text_size);

  // Create a mock file
  size_t n = snprintf(NULL, 0, "%lu", text_size);
  char filepath[30 + n];
  sprintf(filepath, "tests/TEST_LIB_MOCK_FILE-%lu.txt", text_size);
  FILE *fp = fopen(filepath, "w");
  if (!fp)
  {
    printf("\t\t[INFO]: test_lib_buffer_read_file: Could not open file (to "
           "write) `%s`: "
           "%s\n",
           filepath, strerror(errno));
    return false;
  }
  fwrite(text, 1, text_size, fp);
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

  ASSERT(test_1, buf.available == text_size);
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_file(void)
{
  // Create a mock file (using a random number to ensure we're not
  // conflicting with something in the workspace
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 10);
  bool test_small = test_lib_buffer_read_file_fixed(1 << 10);
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 20);
  bool test_medium = test_lib_buffer_read_file_fixed(1 << 20);
  printf("\t\t[INFO]: test_lib_buffer_read_file: Testing data size %lu\n",
         1LU << 25);
  bool test_large = test_lib_buffer_read_file_fixed(1 << 25);
  return test_small && test_medium && test_large;
}

bool test_lib_buffer_read_cstr_fixed(size_t text_size)
{
  char *text   = generate_random_data(text_size);
  buffer_t buf = buffer_read_cstr("*test-cstr*", text, text_size);

  ASSERT(test_1, buf.available == text_size);
  ASSERT(test_2, strncmp(buf.data, text, text_size) == 0);
  free(buf.data);
  free(text);

  return test_1 && test_2;
}

bool test_lib_buffer_read_cstr(void)
{
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 10);
  bool test_small = test_lib_buffer_read_cstr_fixed(1 << 10);
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 20);
  bool test_medium = test_lib_buffer_read_cstr_fixed(1 << 20);
  printf("\t\t[INFO]: test_lib_buffer_read_cstr: Testing data size %lu\n",
         1LU << 25);
  bool test_large = test_lib_buffer_read_cstr_fixed(1 << 25);
  return test_small && test_medium && test_large;
}

