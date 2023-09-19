#ifndef OP_H
#define OP_H

#include "./lib.h"

#include <stdio.h>

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
    OP_LABEL,
    OP_JUMP_REL,
    OP_JUMP_LABEL,
    NUMBER_OF_OPERATORS,
  } opcode;

  i64 operand;
} op_t;

#define OP_CREATE_HALT       ((op_t){.opcode = OP_HALT, .operand = 0})
#define OP_CREATE_PUSH(x)    ((op_t){.opcode = OP_PUSH, .operand = x})
#define OP_CREATE_PLUS       ((op_t){.opcode = OP_PLUS, .operand = 0})
#define OP_CREATE_DUP(x)     ((op_t){.opcode = OP_DUP, .operand = x})
#define OP_CREATE_PRINT      ((op_t){.opcode = OP_PRINT, .operand = 0})
#define OP_CREATE_JMP_REL(x) ((op_t){.opcode = OP_JUMP_REL, .operand = x})
#define OP_CREATE_JMP_LBL(x) ((op_t){.opcode = OP_JUMP_LABEL, .operand = x})
#define OP_CREATE_LABEL(x)   ((op_t){.opcode = OP_LABEL, .operand = x})

void op_print(op_t op, FILE *fp);

#endif
