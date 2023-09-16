/* interpreter.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Bytecode interpreter
 */

#include "./lib.h"
#include "./op.h"
#include "./parser.h"
#include "./vm.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void usage(FILE *fp)
{
  fputs("./interpreter.out [FILE]\n"
        "\\tInterpret bytecode in FILE\n"
        "\tFILE: File name for bytecode\n",
        fp);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    usage(stderr);
    return 0;
  }

  vm_t vm               = {0};
  const char *file_name = argv[1];
  FILE *fp              = fopen(file_name, "rb");
  if (!fp)
  {
    fprintf(stderr, "[ERROR]: Could not read file `%s`: %s\n", file_name,
            strerror(errno));
    usage(stderr);
    return 1;
  }
  vm_read_program(&vm, fp);
  fclose(fp);
  err_t err = vm_execute_all(&vm);
  if (err != ERR_OK)
  {
    fprintf(stderr, "[ERROR]: %s\n[ERROR]: Trace:\n", err_as_cstr(err));
    vm_print_all(&vm, stderr);
    return -1;
  }

  return 0;
}
