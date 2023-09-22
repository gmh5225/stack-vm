/* assembler.c
 * Created: 2023-09-16
 * Author: Aryadev Chavali
 * Description: Compiles assembly to bytecode
 */

#include "./lib.h"
#include "./op.h"
#include "./parser.h"
#include "./vm.h"

#include <errno.h>
#include <string.h>

void usage(FILE *fp)
{
  fputs("./assembler.out [FILE] [OUTPUT]\n"
        "\tAssemble FILE into bytecode, stored at OUTPUT\n"
        "\tFILE: File name for assembly code\n"
        "\tOUTPUT: File name for bytecode storage (will be overwritten)\n",
        fp);
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    usage(stderr);
    return 0;
  }

  const char *in_name  = argv[1];
  const char *out_name = argv[2];

  int ret            = 0;
  buffer_t buf       = {0};
  vm_t vm            = {0};
  op_t *instructions = NULL;

  FILE *fp = fopen(in_name, "rb");
  if (!fp)
  {
    fprintf(stderr, "[ERROR]: Could not read file `%s`: %s\n", in_name,
            strerror(errno));
    usage(stderr);
    ret = 1;
    goto error;
  }

  // Read file into memory
  buf = buffer_read_file(in_name, fp);
  fclose(fp);

  instructions          = NULL;
  u64 instructions_size = 0;
  // Attempt to parse buffer
  perr_t err = parse_buffer(&buf, &instructions, &instructions_size);
  if (err != PERR_OK)
  {
    char *reason = perr_generate(err, &buf);
    fprintf(stderr, "%s\n", reason);
    free(reason);
    usage(stderr);
    ret = 255 - err;
    goto error;
  }
  free(buf.data);

  vm_copy_program(&vm, instructions, instructions_size);
  free(instructions);

  // Now we can output the parsed bytecode
  fp = fopen(out_name, "wb");
  if (!fp)
  {
    fprintf(stderr, "[ERROR]: Could not open file `%s`: %s\n", in_name,
            strerror(errno));
    usage(stderr);
    ret = 1;
    goto error;
  }

  vm_write_program(&vm, fp);
  fclose(fp);

  printf("[INFO]: Successfully compiled `%s`->`%s`", in_name, out_name);

  return 0;
error:
  if (buf.data)
    free(buf.data);
  if (instructions)
    free(instructions);
  return ret;
}
