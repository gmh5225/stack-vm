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
  fputs("./assembler.out [FILE] [OUTPUT]?\n"
        "\tAssemble FILE into bytecode, stored at OUTPUT\n"
        "\tFILE: File name for assembly code\n"
        "\tOUTPUT: Optional file name for bytecode storage (will be "
        "overwritten)\n",
        fp);
}

void gen_output_filename(const char *name, size_t name_size, char *buffer)
{
  memcpy(buffer, name, name_size + 1);
  char *ext = strstr(buffer, ".asm");
  memcpy(ext, ".out", 4);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    usage(stderr);
    return 0;
  }

  bool generated_output = false;
  const char *in_name   = argv[1];
  char *out_name        = NULL;

  if (argc > 2)
    out_name = argv[2];
  else
  {
    generated_output = true;
    size_t name_size = strlen(in_name);
    out_name         = calloc(name_size + 1, sizeof(*out_name));
    gen_output_filename(in_name, name_size, out_name);
  }

  int ret            = 0;
  buffer_t buf       = {0};
  vm_t vm            = {0};
  op_t *instructions = NULL;

  FILE *fp = fopen(in_name, "rb");
  if (!fp)
  {
    fprintf(stderr,
            "[" TERM_RED "ERROR" TERM_RESET "]: Could not read file `%s`: %s\n",
            in_name, strerror(errno));
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
    fprintf(stderr, "[" TERM_RED "ERROR" TERM_RESET "]: %s\n", reason);
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
    fprintf(stderr,
            "[" TERM_RED "ERROR" TERM_RESET "]: Could not open file `%s`: %s\n",
            in_name, strerror(errno));
    usage(stderr);
    ret = 1;
    goto error;
  }

  vm_write_program(&vm, fp);
  fclose(fp);

#if VERBOSE == 1
  printf("[" TERM_CYAN "ASSEMBLER" TERM_RESET
         "]: Successfully compiled `%s`->`%s`\n",
         in_name, out_name);
#endif

  if (generated_output)
    free(out_name);

  return 0;
error:
  if (buf.data)
    free(buf.data);
  if (instructions)
    free(instructions);
  if (generated_output)
    free(out_name);
  return ret;
}
