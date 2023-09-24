#ifndef OP_H
#define OP_H

#include "./data.h"
#include "./lib.h"

#include <stdio.h>

typedef enum OPCODE
{
  OP_NONE = 0,
  OP_HALT,
  OP_PLUS,
  OP_PRINT,
  OP_POP,

  // Instructions with operands
  OP_PUSH,
  OP_DUP,
  OP_JUMP,
  NUMBER_OF_OPERATORS,
} inst_t;

typedef struct
{
  inst_t opcode;
  data_t *operand;
} op_t;

#define OP_CREATE_NOOP    ((op_t){.opcode = OP_NONE, .operand = data_nil()})
#define OP_CREATE_HALT    ((op_t){.opcode = OP_HALT, .operand = data_nil()})
#define OP_CREATE_PUSH(x) ((op_t){.opcode = OP_PUSH, .operand = x})
#define OP_CREATE_POP     ((op_t){.opcode = OP_POP, .operand = data_nil()})
#define OP_CREATE_PLUS    ((op_t){.opcode = OP_PLUS, .operand = data_nil()})
#define OP_CREATE_DUP(x)  ((op_t){.opcode = OP_DUP, .operand = x})
#define OP_CREATE_PRINT   ((op_t){.opcode = OP_PRINT, .operand = data_nil()})
#define OP_CREATE_JMP(x)  ((op_t){.opcode = OP_JUMP, .operand = x})

void op_print(op_t op, FILE *fp);

#endif
