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

err_t vm_execute(vm_t *vm)
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
    if (vm->sptr >= VM_STACK_MAX)
      return ERR_STACK_OVERFLOW;
    vm->stack[vm->sptr] = op.operand;
    vm->sptr++;
    vm->iptr++;
    break;
  case OP_PLUS:
    if (vm->sptr < 2)
      return ERR_STACK_UNDERFLOW;
    i64 a = vm->stack[vm->sptr - 2];
    i64 b = vm->stack[vm->sptr - 1];
    if (a > 0 && (b > (INT64_MAX - a)))
      return ERR_INTEGER_OVERFLOW;
    else if (b < (INT64_MIN - a))
      return ERR_INTEGER_UNDERFLOW;
    vm->stack[vm->sptr - 2] += b;
    vm->sptr--;
    vm->iptr++;
    break;
  case OP_DUP:
    if (vm->sptr == 0)
      return ERR_STACK_UNDERFLOW;
    else if (vm->sptr >= VM_STACK_MAX)
      return ERR_STACK_OVERFLOW;
    vm->stack[vm->sptr] = vm->stack[vm->sptr - 1 - op.operand];
    vm->sptr++;
    vm->iptr++;
    break;
  case OP_PRINT:
    if (vm->sptr == 0)
      return ERR_STACK_UNDERFLOW;
    printf("%" PRId64 "\n", vm->stack[vm->sptr - 1]);
    vm->iptr++;
    break;
  case OP_LABEL:
    if (op.operand > VM_LABEL_MAX)
      return ERR_LABEL_OVERFLOW;
    vm->labels[op.operand] = vm->iptr + 1;
    vm->iptr++;
    break;
  case OP_JUMP_REL:
    if (vm->iptr + op.operand > VM_PROGRAM_MAX ||
        ((i64)vm->iptr) + op.operand < 0)
      return ERR_ILLEGAL_JUMP;
    vm->iptr += op.operand;
    break;
  case OP_JUMP_LABEL:
    if (op.operand > VM_LABEL_MAX)
      return ERR_LABEL_OVERFLOW;
    vm->iptr = vm->labels[op.operand];
    break;
  default:
    return ERR_ILLEGAL_INSTRUCTION;
  }
  return ERR_OK;
}

err_t vm_execute_all(vm_t *vm)
{
  while (vm->program[vm->iptr].opcode != OP_HALT && vm->iptr < vm->size_program)
  {
    err_t err = vm_execute(vm);
    if (err != ERR_OK)
      return err;
  }
  return ERR_OK;
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
