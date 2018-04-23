#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "values.h"

enum Operation {
    opClearStack,
    opJump,
    opLoadVariable,
    opSaveVariable,
    opWrite,
    opWriteLn,
    opReadLn,
    opDup,

    opIntPlus,
    opIntPlusUn,
    opIntMinus,
    opIntMinusUn,
    opIntMul,
    opIntDiv,
    opIntMod,
    opIntSm,
    opIntGr,
    opIntSmEq,
    opIntGrEq,
    opIntEq,
    opIntNotEq,

    opStrPlus,
    opStrPlusUn,
    opStrSm,
    opStrGr,
    opStrEq,
    opStrNotEq,

    opBoolPlusUn,
    opBoolNot,
    opBoolAnd,
    opBoolOr,

    opRealPlus,
    opRealPlusUn,
    opRealMinus,
    opRealMinusUn,
    opRealMul,
    opRealDiv,
    opRealSm,
    opRealGr,
    opRealSmEq,
    opRealGrEq,
    opRealEq,
    opRealNotEq
};

bool operation_is_unary(Operation op);
Value *operation_execute(Operation op, Value *left);
Value *operation_execute(Operation op, Value *left, Value *right);

#endif // OPERATIONS_H
