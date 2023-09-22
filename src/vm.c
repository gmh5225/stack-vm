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
    fprintf(fp, "  %lu: ", i);
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
      fprintf(fp, "  ");
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
      return ERR_ILLEGAL_TYPE;

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
      return ERR_ILLEGAL_TYPE;
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
      return ERR_ILLEGAL_TYPE;
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
#if DEBUG == 1
    printf("[INFO]: Assembling `");
    op_print(vm->program[i], stdout);
    puts("`");
#endif
    size_t size = 0;
    if (vm->program[i].opcode >= OP_PUSH)
    {
      data_type_t type = data_type(vm->program[i].operand);
      size             = data_type_bytecode_size(type) + 1;
      byte bytecode[size];

      bytecode[0] = vm->program[i].opcode;
      if (vm->program[i].opcode >= OP_PUSH)
        data_write(vm->program[i].operand, bytecode + 1);

      darr_mem_append(&bytes, (byte *)bytecode, size);
    }
    else
    {
      size = 1;
      darr_mem_append(&bytes, &vm->program[i].opcode, 1);
    }

#if DEBUG == 1
    printf("[INFO]: Assembled %lu bytes\n", size + 1);
#endif
  }
  fwrite(bytes.data, sizeof(byte), bytes.used, fp);
  darr_free(&bytes);
}

err_t vm_read_program(vm_t *vm, FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  byte bytes[size];
  for (size_t i = 0; i < (size_t)size; ++i)
    fread(bytes + i, sizeof(byte), 1, fp);

  size_t j = 0;
  for (size_t i = 0; i < (size_t)size && j < VM_PROGRAM_MAX; ++i)
  {
    size_t offset = 0;
    // first byte is an opcode
    inst_t opcode = bytes[i];
    switch (opcode)
    {
    case OP_NONE: {
      vm->program[j].opcode    = OP_NONE;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_HALT: {
      vm->program[j].opcode    = OP_HALT;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_PUSH: {
      vm->program[j].opcode = OP_PUSH;
      byte tag              = bytes[++i];
      if (tag != DATA_INT)
        return ERR_ILLEGAL_TYPE;
      ++i;
      offset                   = data_type_bytecode_size(DATA_INT);
      vm->program[j++].operand = data_read(DATA_INT, bytes + i);
      i += offset - 2;
      break;
    }
    case OP_PLUS: {
      vm->program[j].opcode    = OP_PLUS;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_DUP: {
      vm->program[j].opcode = OP_DUP;
      byte tag              = bytes[++i];
      if (tag != DATA_UINT)
        return ERR_ILLEGAL_TYPE;
      ++i;
      offset                   = data_type_bytecode_size(DATA_UINT);
      vm->program[j++].operand = data_read(DATA_UINT, bytes + i);
      i += offset - 2;
      break;
    }
    case OP_PRINT: {
      vm->program[j].opcode    = OP_PRINT;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_JUMP: {
      vm->program[j].opcode = OP_JUMP;
      byte tag              = bytes[++i];
      if (tag != DATA_UINT)
        return ERR_ILLEGAL_TYPE;
      ++i;
      offset                   = data_type_bytecode_size(DATA_UINT);
      vm->program[j++].operand = data_read(DATA_UINT, bytes + i);
      i += offset - 2;
      break;
    }
    case NUMBER_OF_OPERATORS:
    default:
      return ERR_ILLEGAL_INSTRUCTION;
    }

#if DEBUG == 1
    printf("[INFO]: Read instruction `");
    op_print(vm->program[j - 1], stdout);
    printf("` from file\n");
#endif
  }

  if (j == VM_PROGRAM_MAX)
    assert(false && "vm_read_program: Program is larger than VM_PROGRAM_MAX");

  vm->size_program = j;
  return ERR_OK;
}
