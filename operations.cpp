#include "exceptions.h"
#include "operations.h"

bool operation_is_unary(Operation op)
{
    return
        op == opIntPlusUn || op == opIntMinusUn ||
        op == opRealPlusUn || op == opRealMinusUn ||
        op == opStrPlusUn || op == opBoolPlusUn ||
        op == opBoolNot;
}

Value *operation_execute(Operation op, Value *left)
{
    switch (op) {
    case opIntPlusUn:
        return new IntegerValue(+left->to_integer());
    case opIntMinusUn:
        return new IntegerValue(-left->to_integer());
    case opRealPlusUn:
        return new RealValue(+left->to_real());
    case opRealMinusUn:
        return new RealValue(-left->to_real());
    case opStrPlusUn:
        return new StringValue(left->to_string());
    case opBoolPlusUn:
        return new BooleanValue(left->to_boolean());;
    case opBoolNot:
        return new BooleanValue(not left->to_boolean());;
    default:
        throw std::runtime_error("Unknown unary operation");
    }
}

Value *operation_execute(Operation op, Value *left, Value *right)
{
    switch (op) {
    case opIntPlus:
        return new IntegerValue(left->to_integer() + right->to_integer());
    case opIntMinus:
        return new IntegerValue(left->to_integer() - right->to_integer());
    case opIntMul:
        return new IntegerValue(left->to_integer() * right->to_integer());
    case opIntDiv:
        if (right->to_integer() == 0) {
            throw InterpretationError("Divide by zero.");
        }
        return new IntegerValue(left->to_integer() / right->to_integer());
    case opIntMod:
        if (right->to_integer() == 0) {
            throw InterpretationError("Divide by zero.");
        }
        return new IntegerValue(left->to_integer() % right->to_integer());
    case opIntSm:
        return new BooleanValue(left->to_integer() < right->to_integer());
    case opIntGr:
        return new BooleanValue(left->to_integer() > right->to_integer());
    case opIntSmEq:
        return new BooleanValue(left->to_integer() <= right->to_integer());
    case opIntGrEq:
        return new BooleanValue(left->to_integer() >= right->to_integer());
    case opIntEq:
        return new BooleanValue(left->to_integer() == right->to_integer());
    case opIntNotEq:
        return new BooleanValue(left->to_integer() != right->to_integer());
    case opStrPlus:
        return new StringValue(left->to_string() + right->to_string());
    case opStrGr:
        return new BooleanValue(left->to_string() > right->to_string());
    case opStrSm:
        return new BooleanValue(left->to_string() < right->to_string());
    case opStrEq:
        return new BooleanValue(left->to_string() == right->to_string());
    case opStrNotEq:
        return new BooleanValue(left->to_string() != right->to_string());
    case opBoolAnd:
        return new BooleanValue(left->to_boolean() and right->to_boolean());
    case opBoolOr:
        return new BooleanValue(left->to_boolean() or right->to_boolean());
    case opRealPlus:
        return new RealValue(left->to_real() + right->to_real());
    case opRealMinus:
        return new RealValue(left->to_real() - right->to_real());
    case opRealMul:
        return new RealValue(left->to_real() * right->to_real());
    case opRealDiv:
        return new RealValue(left->to_real() / right->to_real());
    case opRealSm:
        return new BooleanValue(left->to_real() < right->to_real());
    case opRealGr:
        return new BooleanValue(left->to_real() > right->to_real());
    case opRealSmEq:
        return new BooleanValue(left->to_real() <= right->to_real());
    case opRealGrEq:
        return new BooleanValue(left->to_real() >= right->to_real());
    case opRealEq:
        return new BooleanValue(left->to_real() == right->to_real());
    case opRealNotEq:
        return new BooleanValue(left->to_real() != right->to_real());
    default:
        throw std::runtime_error("Unknown binary operation");
    };
}
