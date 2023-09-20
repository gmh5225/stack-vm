#ifndef VM_H
#define VM_H

#include "./err.h"
#include "./lib.h"
#include "./op.h"

#define VM_STACK_MAX   1024
#define VM_PROGRAM_MAX 1024
#define VM_LABEL_MAX   1024

typedef struct
{
  op_t program[VM_PROGRAM_MAX];
  word iptr, size_program;

  i64 stack[VM_STACK_MAX];
  word sptr;
} vm_t;

void vm_print_all(vm_t *vm, FILE *fp);

err_t vm_execute(vm_t *vm);
err_t vm_execute_all(vm_t *vm);

void vm_copy_program(vm_t *vm, op_t *ops, size_t size_ops);
void vm_write_program(vm_t *vm, FILE *fp);
void vm_read_program(vm_t *vm, FILE *fp);

#endif
