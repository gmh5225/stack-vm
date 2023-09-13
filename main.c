/* main.c
 * Created: 2023-09-13
 * Author: Aryadev Chavali
 * Description:
 */

#include <inttypes.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint64_t u64;
typedef int64_t i64;

typedef u64 word;

typedef struct
{
  enum OPCODE
  {
    OP_NONE = 0,
    OP_PUSH,
    OP_PLUS,
    OP_PRINT,
    OP_HALT,
  } opcode;

  word operand;
} op_t;

#define OP_CREATE_PUSH(x) ((op_t){.opcode = OP_PUSH, .operand = x})
#define OP_CREATE_PLUS    ((op_t){.opcode = OP_PLUS})
#define OP_CREATE_PRINT   ((op_t){.opcode = OP_PRINT})
#define OP_CREATE_HALT    ((op_t){.opcode = OP_HALT})

#define VM_STACK_MAX   1024
#define VM_PROGRAM_MAX 1024

typedef struct
{
  op_t program[VM_PROGRAM_MAX];
  word iptr, size_program;

  i64 stack[VM_STACK_MAX];
  word sptr;
} vm_t;

void vm_execute(vm_t *vm)
{
  op_t op = (vm->program[vm->iptr++]);
  switch (op.opcode)
  {
  case OP_NONE:
    break;
  case OP_HALT:
    break;
  case OP_PUSH:
    vm->stack[vm->sptr++] = op.operand;
    break;
  case OP_PLUS:
    vm->stack[vm->sptr - 2] += vm->stack[vm->sptr - 1];
    vm->sptr--;
    break;
  case OP_PRINT:
    printf("%" PRId64, vm->stack[vm->sptr]);
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

void op_print(op_t op, FILE *fp)
{
  switch (op.opcode)
  {
  case OP_NONE:
    fprintf(fp, "OP_NONE");
    break;
  case OP_HALT:
    fprintf(fp, "OP_HALT");
    break;
  case OP_PUSH:
    fprintf(fp, "OP_PUSH(%" PRId64 ")", op.operand);
    break;
  case OP_PLUS:
    fprintf(fp, "OP_PLUS");
    break;
  case OP_PRINT:
    fprintf(fp, "OP_PRINT");
    break;
  }
}

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

  fprintf(fp, "Stack={\n");
  for (i64 i = vm->sptr - 1; i >= 0; --i)
  {
    fprintf(fp, "\t");
    fprintf(fp, "%" PRId64 "\n", vm->stack[i]);
  }
  fprintf(fp, "}\n");
}

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

int main(void)
{
  vm_t vm        = {0};
  op_t program[] = {OP_CREATE_PUSH(1), OP_CREATE_PUSH(2), OP_CREATE_PLUS,
                    OP_CREATE_PRINT, OP_CREATE_HALT};
  vm_copy_program(&vm, program, ARR_SIZE(program));
  vm_execute_all(&vm);
  return 0;
}
