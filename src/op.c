/* op.c
 * Created: 2023-09-14
 * Author: Aryadev Chavali
 * Description: Instruction implementation
 */

#include "./op.h"

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
