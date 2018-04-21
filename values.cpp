#include <sstream>
#include <cstdlib>
#include "values.h"

ValueType Value::get_type() const
{
    return vtNone;
}

IntegerValue::IntegerValue(Integer value): value(value) {}

IntegerValue::IntegerValue(const String &str)
{
    StringValue temp(str);
    value = temp.to_integer();
}

Value *IntegerValue::clone() const
{
    return new IntegerValue(value);
}

ValueType IntegerValue::get_type() const
{
    return vtInteger;
}

Integer IntegerValue::to_integer() const
{
    return value;
}

String IntegerValue::to_string() const
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

Boolean IntegerValue::to_boolean() const
{
    return value != 0;
}

Real IntegerValue::to_real() const
{
    return (Real)value;
}

StringValue::StringValue(const String &value): value(value) {}

Value *StringValue::clone() const
{
    return new StringValue(value);
}

ValueType StringValue::get_type() const
{
    return vtString;
}

Integer StringValue::to_integer() const
{
    return atoll(value.c_str());
}

String StringValue::to_string() const
{
    return value;
}

Boolean StringValue::to_boolean() const
{
    return value != "false";
}

Real StringValue::to_real() const
{
    return atof(value.c_str());
}

BooleanValue::BooleanValue(Boolean value): value(value) {}

BooleanValue::BooleanValue(const String &str)
{
    StringValue temp(str);
    value = temp.to_boolean();
}

Value *BooleanValue::clone() const
{
    return new BooleanValue(value);
}

ValueType BooleanValue::get_type() const
{
    return vtBoolean;
}

Integer BooleanValue::to_integer() const
{
    if (value) {
        return 1;
    } else {
        return 0;
    }
}

String BooleanValue::to_string() const
{
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

Boolean BooleanValue::to_boolean() const
{
    return value;
}

Real BooleanValue::to_real() const
{
    if (value) {
        return 1.0;
    } else {
        return 0.0;
    }
}

RealValue::RealValue(Real value): value(value) {}

RealValue::RealValue(const String &str)
{
    StringValue temp(str);
    value = temp.to_real();
}

Value *RealValue::clone() const
{
    return new RealValue(value);
}

ValueType RealValue::get_type() const
{
    return vtReal;
}

Integer RealValue::to_integer() const
{
    return (Integer)value;
}

String RealValue::to_string() const
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

Boolean RealValue::to_boolean() const
{
    return (Boolean)value;
}

Real RealValue::to_real() const
{
    return value;
}
