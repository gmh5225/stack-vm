/* vm.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include "./vm.h"

#include <assert.h>
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
      data_print(vm->stack[i], fp);
      fprintf(fp, "\n");
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
    data_t *a = vm->stack[vm->sptr - 2];
    data_t *b = vm->stack[vm->sptr - 1];

    data_type_t a_ = data_type(a);
    data_type_t b_ = data_type(b);

    if (!(data_type_is_numeric(a_) && data_type_is_numeric(b_)))
      return ERR_ILLEGAL_OPERAND;

    data_numerics_promote_on_float(&a, &a_, &b, &b_);

    // Check if float (if so, just add now)
    if (a_ == DATA_FLOAT)
    {
      vm->stack[vm->sptr - 2] = data_float(data_as_float(a) * data_as_float(b));
    }
    else if ((a_ == DATA_INT && b_ == DATA_UINT) ||
             (a_ == DATA_UINT && b_ == DATA_INT))
    {
      u64 c = data_as_uint(a_ == DATA_INT ? b : a);
      i64 d = data_as_int(a_ == DATA_INT ? a : b);
      if (d > 0 && (c > (UINT60_MAX - d)))
        // Integer overflow
        return ERR_INTEGER_OVERFLOW;
      // Cast to integer
      else if (d < 0)
        vm->stack[vm->sptr - 2] = data_int(c + d);
      else
        // Cast to unsigned
        vm->stack[vm->sptr - 2] = data_uint(c + d);
    }
    else if (a_ == DATA_INT)
    {
      i64 c = data_as_int(a);
      i64 d = data_as_int(b);

      if (c > 0 && (d > (INT64_MAX - c)))
        return ERR_INTEGER_OVERFLOW;
      else if (c < 0 && d < (INT64_MIN - c))
        return ERR_INTEGER_UNDERFLOW;
      vm->stack[vm->sptr - 2] = data_int(c + d);
    }
    else
    {
      u64 c = data_as_uint(a);
      u64 d = data_as_uint(b);

      if (d > (INT64_MAX - c))
        return ERR_INTEGER_OVERFLOW;
      vm->stack[vm->sptr - 2] = data_uint(c + d);
    }

    vm->sptr--;
    vm->iptr++;
    break;
  case OP_DUP:
    if (vm->sptr == 0)
      return ERR_STACK_UNDERFLOW;
    else if (vm->sptr >= VM_STACK_MAX)
      return ERR_STACK_OVERFLOW;
    else if (data_type(op.operand) != DATA_UINT)
      return ERR_ILLEGAL_OPERAND;
    vm->stack[vm->sptr] = vm->stack[vm->sptr - 1 - data_as_uint(op.operand)];
    vm->sptr++;
    vm->iptr++;
    break;
  case OP_PRINT:
    if (vm->sptr == 0)
      return ERR_STACK_UNDERFLOW;
    data_print(vm->stack[vm->sptr - 1], stdout);
    printf("\n");
    vm->iptr++;
    break;
  case OP_JUMP: {
    data_type_t type = data_type(op.operand);
    if (!data_type_is_numeric(type))
      return ERR_ILLEGAL_OPERAND;
    if (type != DATA_UINT || data_as_uint(op.operand) > vm->size_program)
      return ERR_ILLEGAL_JUMP;
    vm->iptr = data_as_uint(op.operand);
    break;
  }
  case NUMBER_OF_OPERATORS:
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
  darr_t bytes = {0};
  darr_init(&bytes, 1, sizeof(byte));
  for (size_t i = 0; i < vm->size_program; ++i)
  {
    data_type_t type = data_type(vm->program[i].operand);
    size_t size      = data_type_bytecode_size(type);
    byte bytecode[size + 1];

#if DEBUG == 1
    printf("[INFO]: Assembling `");
    op_print(vm->program[i], stdout);
    puts("`");
#endif

    bytecode[0] = vm->program[i].opcode;
    data_write(vm->program[i].operand, bytecode + 1);

#if DEBUG == 1
    printf("[INFO]: Assembled %lu bytes\n", size + 1);
#endif

    darr_mem_append(&bytes, (byte *)bytecode, size + 1);
  }
  fwrite(bytes.data, sizeof(byte), bytes.used, fp);
  darr_free(&bytes);
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
