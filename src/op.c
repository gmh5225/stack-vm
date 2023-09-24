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
  case OP_POP:
    fprintf(fp, "OP_POP");
    break;
  case OP_PUSH:
    fprintf(fp, "OP_PUSH(");
    data_print(op.operand, fp);
    fprintf(fp, ")");
    break;
  case OP_PLUS:
    fprintf(fp, "OP_PLUS");
    break;
  case OP_DUP:
    fprintf(fp, "OP_DUP(");
    data_print(op.operand, fp);
    fprintf(fp, ")");
    break;
  case OP_PRINT:
    fprintf(fp, "OP_PRINT");
    break;
  case OP_JUMP:
    fprintf(fp, "OP_JUMP(");
    data_print(op.operand, fp);
    fprintf(fp, ")");
    break;
  case NUMBER_OF_OPERATORS:
    fputs("", fp);
    break;
  }
}
