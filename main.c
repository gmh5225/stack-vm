/* main.c
 * Created: 2023-09-13
 * Author: Aryadev Chavali
 * Description:
 */

#include "./lib.h"
#include "./op.h"
#include "./vm.h"

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

int main(void)
{
  FILE *fp       = NULL;
  vm_t vm        = {0};
  op_t program[] = {OP_CREATE_PUSH(1),    OP_CREATE_PUSH(1),

                    OP_CREATE_LABEL(1),

                    OP_CREATE_DUP(1),     OP_CREATE_DUP(1),

                    OP_CREATE_PLUS,

                    OP_CREATE_PRINT,

                    OP_CREATE_JMP_LBL(1),

                    OP_CREATE_HALT};
  vm_copy_program(&vm, program, ARR_SIZE(program));
  /* fp = fopen("test.bin", "rb"); */
  /* vm_read_program(&vm, fp); */
  /* fclose(fp); */
  err_t err = vm_execute_all(&vm);
  if (err != ERR_OK)
  {
    fprintf(stderr, "ERROR: %s\n", err_as_cstr(err));
    vm_print_all(&vm, stderr);
    return -1;
  }
  fp = fopen("test.bin", "wb");
  vm_write_program(&vm, fp);
  fclose(fp);
  return 0;
}
