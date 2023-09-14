/* vm.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include "./vm.h"

#include <string.h>

void vm_print_all(vm_t *vm, FILE *fp)
{
  fprintf(fp, "Program={\n");
  for (size_t i = 0; i < vm->size_program; ++i)
  {
    fprintf(fp, "\t");
    op_print(vm->program[i], fp);
    if (i == vm->iptr)
      fprintf(fp, "<--");
    fprintf(fp, "\n");
  }
  fprintf(fp, "}\n");

  if (vm->sptr == 0)
  {
    fprintf(fp, "Stack={}\n");
  }
  else
  {
    fprintf(fp, "Stack={\n");
    for (i64 i = vm->sptr - 1; i >= 0; --i)
    {
      fprintf(fp, "\t");
      fprintf(fp, "%" PRId64 "\n", vm->stack[i]);
    }
    fprintf(fp, "}\n");
  }
}

void vm_execute(vm_t *vm)
{
#if DEBUG
  vm_print_all(vm, stderr);
  fputs("\n", stderr);
#endif
  op_t op = (vm->program[vm->iptr]);
  switch (op.opcode)
  {
  case OP_NONE:
    vm->iptr++;
    break;
  case OP_HALT:
    vm->iptr++;
    break;
  case OP_PUSH:
    vm->stack[vm->sptr] = op.operand;
    vm->sptr++;
    vm->iptr++;
    break;
  case OP_PLUS:
    vm->stack[vm->sptr - 2] += vm->stack[vm->sptr - 1];
    vm->sptr--;
    vm->iptr++;
    break;
  case OP_DUP:
    vm->stack[vm->sptr] = vm->stack[vm->sptr - 1 - op.operand];
    vm->sptr++;
    vm->iptr++;
    break;
  case OP_PRINT:
    printf("%" PRId64 "\n", vm->stack[vm->sptr - 1]);
    vm->iptr++;
    break;
  case OP_LABEL:
    vm->labels[op.operand] = vm->iptr + 1;
    vm->iptr++;
    break;
  case OP_JUMP_REL:
    vm->iptr += op.operand;
    break;
  case OP_JUMP_LABEL:
    vm->iptr = vm->labels[op.operand];
    break;
  }
}

void vm_execute_all(vm_t *vm)
{
  while (vm->program[vm->iptr].opcode != OP_HALT && vm->iptr < vm->size_program)
    vm_execute(vm);
}

void vm_copy_program(vm_t *vm, op_t *ops, size_t size_ops)
{
  memcpy(vm->program, ops, size_ops * sizeof(*ops));
  vm->size_program = size_ops;
}

void vm_write_program(vm_t *vm, FILE *fp)
{
  fwrite(vm->program, sizeof(vm->program[0]), vm->size_program, fp);
}

void vm_read_program(vm_t *vm, FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t program_size = size / sizeof(vm->program[0]);
  op_t program[program_size];

  fread(program, sizeof(program[0]), program_size, fp);
  vm_copy_program(vm, program, program_size);
}
