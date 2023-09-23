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

      if (c > 0 && (d > (INT60_MAX - c)))
        return ERR_INTEGER_OVERFLOW;
      else if (c < 0 && d < (INT60_MIN - c))
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
#if VERBOSE == 1
    printf("[" TERM_GREEN "vm_write_program" TERM_RESET "]: Assembling `");
    op_print(vm->program[i], stdout);
    printf("`...");
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

#if VERBOSE == 1
    printf(" %lu bytes\n", size + 1);
#endif
  }
  fwrite(bytes.data, sizeof(byte), bytes.used, fp);
  darr_free(&bytes);
}

err_t read_numeric_from_bytes(buffer_t *buffer, op_t *ret)
{
  byte tag = buffer_pop(buffer);
  if (!data_type_is_numeric(tag))
    return ERR_ILLEGAL_TYPE;

  size_t offset = data_type_bytecode_size(tag) - 1;

  if (buffer_space_left(*buffer) < offset)
    return ERR_BYTECODE_EOF;

  ret->operand = data_read(tag, ((byte *)buffer->data) + buffer->cur);
  buffer->cur += offset;
  return ERR_OK;
}

err_t read_type_from_bytes(buffer_t *buffer, data_type_t type, op_t *ret)
{
  byte tag = buffer_pop(buffer);

  if (tag != type)
    return ERR_ILLEGAL_TYPE;
  else if (tag == DATA_NIL)
  {
    ret->operand = data_nil();
    return ERR_OK;
  }

  size_t offset = data_type_bytecode_size(type) - 1;

  if (buffer_space_left(*buffer) < offset)
    return ERR_BYTECODE_EOF;

  ret->operand = data_read(type, ((byte *)buffer->data) + buffer->cur);
  buffer->cur += offset;
  return ERR_OK;
}

err_t read_immediate_from_bytes(buffer_t *buffer, op_t *ret)
{
  byte tag = buffer_pop(buffer);
  if (!(tag <= DATA_FLOAT && tag >= DATA_NIL))
    return ERR_ILLEGAL_TYPE;
  else if (tag == DATA_NIL)
  {
    ret->operand = data_nil();
    return ERR_OK;
  }

  size_t offset = data_type_bytecode_size(tag) - 1;

  if (buffer_space_left(*buffer) < offset)
    return ERR_BYTECODE_EOF;

  ret->operand = data_read(tag, ((byte *)buffer->data) + buffer->cur);
  buffer->cur += offset;
  return ERR_OK;
}

err_t vm_read_program(vm_t *vm, buffer_t *buffer)
{
  size_t j = 0;
#if VERBOSE == 1
  size_t prev_bytes = 0;
#endif
  while (j < VM_PROGRAM_MAX && buffer_at_end(*buffer) != BUFFER_PAST_END)
  {
#if VERBOSE == 1
    prev_bytes = buffer->cur;
#endif
    // first byte is an opcode
    inst_t opcode = buffer_pop(buffer);
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
      // Basically any immediate data can be pushed
      err_t err_read_type =
          read_immediate_from_bytes(buffer, vm->program + (j++));
      if (err_read_type != ERR_OK)
        return err_read_type;
      break;
    }
    case OP_PLUS: {
      vm->program[j].opcode    = OP_PLUS;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_DUP: {
      vm->program[j].opcode = OP_DUP;
      err_t err_read_type =
          read_type_from_bytes(buffer, DATA_UINT, vm->program + (j++));
      if (err_read_type != ERR_OK)
        return err_read_type;
      break;
    }
    case OP_PRINT: {
      vm->program[j].opcode    = OP_PRINT;
      vm->program[j++].operand = data_nil();
      break;
    }
    case OP_JUMP: {
      vm->program[j].opcode = OP_JUMP;
      err_t err_read_type =
          read_type_from_bytes(buffer, DATA_UINT, vm->program + (j++));
      if (err_read_type != ERR_OK)
        return err_read_type;
      break;
    }
    case NUMBER_OF_OPERATORS:
    default:
      return ERR_ILLEGAL_INSTRUCTION;
    }

#if VERBOSE == 1
    printf("[" TERM_GREEN "vm_read_program" TERM_RESET "]: Read `");
    op_print(vm->program[j - 1], stdout);
    size_t diff = buffer->cur - prev_bytes;
    printf("` %lu %s\n", diff, diff == 1 ? "byte" : "bytes");
#endif
  }

  if (j == VM_PROGRAM_MAX)
    assert(false && "vm_read_program: Program is larger than VM_PROGRAM_MAX");

  vm->size_program = j;
  return ERR_OK;
}
