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

#define DEBUG 0

typedef uint64_t u64;
typedef int64_t i64;

typedef u64 word;

typedef struct
{
  enum OPCODE
  {
    OP_NONE = 0,
    OP_HALT,
    OP_PUSH,
    OP_PLUS,
    OP_DUP,
    OP_PRINT,
    OP_JUMP_REL,
    OP_JUMP_LABEL,
    OP_LABEL
  } opcode;

  word operand;
} op_t;

#define OP_CREATE_HALT       ((op_t){.opcode = OP_HALT})
#define OP_CREATE_PUSH(x)    ((op_t){.opcode = OP_PUSH, .operand = x})
#define OP_CREATE_PLUS       ((op_t){.opcode = OP_PLUS})
#define OP_CREATE_DUP(x)     ((op_t){.opcode = OP_DUP, .operand = x})
#define OP_CREATE_PRINT      ((op_t){.opcode = OP_PRINT})
#define OP_CREATE_JMP_REL(x) ((op_t){.opcode = OP_JUMP_REL, .operand = x})
#define OP_CREATE_JMP_LBL(x) ((op_t){.opcode = OP_JUMP_LABEL, .operand = x})
#define OP_CREATE_LABEL(x)   ((op_t){.opcode = OP_LABEL, .operand = x})

#define VM_STACK_MAX   1024
#define VM_PROGRAM_MAX 1024
#define VM_LABEL_MAX   1024

typedef struct
{
  word labels[VM_LABEL_MAX];
  word lptr;

  op_t program[VM_PROGRAM_MAX];
  word iptr, size_program;

  i64 stack[VM_STACK_MAX];
  word sptr;
} vm_t;

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
  case OP_DUP:
    fprintf(fp, "OP_DUP(%" PRId64 ")", op.operand);
    break;
  case OP_PRINT:
    fprintf(fp, "OP_PRINT");
    break;
  case OP_JUMP_REL:
    fprintf(fp, "OP_JUMP_REL(%" PRId64 ")", op.operand);
    break;
  case OP_JUMP_LABEL:
    fprintf(fp, "OP_JUMP_LABEL(%" PRId64 ")", op.operand);
    break;
  case OP_LABEL:
    fprintf(fp, "OP_LABEL(%" PRId64 ")", op.operand);
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
  vm->size_program = size / sizeof(vm->program[0]);
  fread(vm->program, sizeof(vm->program[0]), vm->size_program, fp);
}

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
  vm_execute_all(&vm);
  vm_print_all(&vm, stderr);
  fp = fopen("test.bin", "wb");
  vm_write_program(&vm, fp);
  fclose(fp);
  return 0;
}
