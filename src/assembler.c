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
  buffer_t buffer    = {0};
  stream_t stream    = {0};
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
    goto end;
  }

  // Read file into memory
  buffer = buffer_read_file(in_name, fp);
  fclose(fp);

  // Tokenise buffer
  lerr_t lerr = tokenise_buffer(&stream, &buffer);
  if (lerr != LERR_OK)
  {
    char *reason = lerr_generate(lerr, &buffer);
    fprintf(stderr, "%s\n", reason);
    free(reason);
    ret = 255 - lerr;
    goto end;
  }
  free(buffer.data);
  buffer.data = NULL;

  instructions          = NULL;
  u64 instructions_size = 0;

  // Attempt to parse buffer
  perr_t err = parse_stream(&stream, &instructions, &instructions_size);
  if (err != PERR_OK)
  {
    char *reason = perr_generate(err, &stream);
    fprintf(stderr, "[" TERM_RED "ERROR" TERM_RESET "]: %s\n", reason);
    free(reason);
    ret = 255 - err;
    goto end;
  }
  stream_free(&stream);

  vm_copy_program(&vm, instructions, instructions_size);
  free(instructions);
  instructions = NULL;

  // Now we can output the parsed bytecode
  fp = fopen(out_name, "wb");
  if (!fp)
  {
    fprintf(stderr,
            "[" TERM_RED "ERROR" TERM_RESET "]: Could not open file `%s`: %s\n",
            in_name, strerror(errno));
    usage(stderr);
    ret = 1;
    goto end;
  }

  vm_write_program(&vm, fp);
  fclose(fp);

#if VERBOSE == 1
  printf("[" TERM_CYAN "ASSEMBLER" TERM_RESET
         "]: Successfully compiled `%s`->`%s`\n",
         in_name, out_name);
#endif

end:
  if (buffer.data)
    free(buffer.data);
  if (instructions)
    free(instructions);
  if (stream.tokens)
    stream_free(&stream);
  if (generated_output)
    free(out_name);
  return ret;
}
