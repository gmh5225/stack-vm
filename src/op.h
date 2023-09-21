#ifndef OP_H
#define OP_H

#include "./lib.h"

#include <stdio.h>

typedef enum OPCODE
{
  OP_NONE = 0,
  OP_HALT,
  OP_PUSH,
  OP_PLUS,
  OP_DUP,
  OP_PRINT,
  OP_JUMP,
  NUMBER_OF_OPERATORS,
} inst_t;

typedef struct
{
  inst_t opcode;
  i64 operand;
} op_t;

#define OP_CREATE_HALT    ((op_t){.opcode = OP_HALT, .operand = 0})
#define OP_CREATE_PUSH(x) ((op_t){.opcode = OP_PUSH, .operand = x})
#define OP_CREATE_PLUS    ((op_t){.opcode = OP_PLUS, .operand = 0})
#define OP_CREATE_DUP(x)  ((op_t){.opcode = OP_DUP, .operand = x})
#define OP_CREATE_PRINT   ((op_t){.opcode = OP_PRINT, .operand = 0})
#define OP_CREATE_JMP(x)  ((op_t){.opcode = OP_JUMP, .operand = x})

void op_print(op_t op, FILE *fp);

#endif
