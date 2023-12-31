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

  FILE *fp = fopen(file_name, "rb");
  if (!fp)
  {
    fprintf(stderr,
            "[" TERM_RED "ERROR" TERM_RESET "]: Could not read file `%s`: %s\n",
            file_name, strerror(errno));
    usage(stderr);
    return 1;
  }
  buffer_t buffer = buffer_read_file(file_name, fp);
  fclose(fp);

  err_t err_read = vm_read_program(&vm, &buffer);

#if VERBOSE == 1
  printf("[" TERM_CYAN "INTEPRETER" TERM_RESET "]: Total read: %lu bytes\n",
         buffer.cur);
#endif

  free(buffer.data);

  if (err_read != ERR_OK)
  {
    char *message = err_generate(err_read, &buffer);
    fprintf(stderr, "%s (in reading `%s`)\n", message, file_name);
    free(message);
    return -1;
  }

#if VERBOSE == 1
  printf("[" TERM_CYAN "INTEPRETER" TERM_RESET
         "]: Number of instructions: %lu\n",
         vm.size_program);
#endif

  err_t err_exec = vm_execute_all(&vm);
  if (err_exec != ERR_OK)
  {
    fprintf(stderr,
            "[" TERM_RED "ERROR" TERM_RESET "]: %s\n[" TERM_RED
            "ERROR" TERM_RESET "]: Trace:\n",
            err_as_cstr(err_exec));
    vm_print_all(&vm, stderr);
    return -1;
  }

#if VERBOSE == 1
  if (vm.sptr > 0)
  {
    printf("[" TERM_RED "MEMORY LEAK" TERM_RESET "]:  %lu items\n", vm.sptr);
  }
#endif

  return 0;
}
