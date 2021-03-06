#ifndef VALUES_H
#define VALUES_H

#include <string>

enum ValueType {
    vtNone,
    vtInteger,
    vtString,
    vtBoolean,
    vtReal
};

typedef long long Integer;
typedef std::string String;
typedef bool Boolean;
typedef double Real;

class Value {
public:
    virtual Value *clone() const = 0;
    virtual ValueType get_type() const;
    virtual Integer to_integer() const = 0;
    virtual String to_string() const = 0;
    virtual Boolean to_boolean() const = 0;
    virtual Real to_real() const = 0;
    virtual ~Value() = default;
};

class IntegerValue: public Value {
private:
    Integer value;
public:
    explicit IntegerValue(Integer value);
    explicit IntegerValue(const String &str);
    Value *clone() const override;
    ValueType get_type() const override;
    Integer to_integer() const override;
    String to_string() const override;
    Boolean to_boolean() const override;
    Real to_real() const override;
};

class StringValue: public Value {
private:
    String value;
public:
    explicit StringValue(const String &value);
    Value *clone() const override;
    ValueType get_type() const override;
    Integer to_integer() const override;
    String to_string() const override;
    Boolean to_boolean() const override;
    Real to_real() const override;
};

class BooleanValue: public Value {
private:
    Boolean value;
public:
    explicit BooleanValue(Boolean value);
    explicit BooleanValue(const String &str);
    Value *clone() const override;
    ValueType get_type() const override;
    Integer to_integer() const override;
    String to_string() const override;
    Boolean to_boolean() const override;
    Real to_real() const override;
};

class RealValue: public Value {
private:
    Real value;
public:
    explicit RealValue(Real value);
    explicit RealValue(const String &str);
    Value *clone() const override;
    ValueType get_type() const override;
    Integer to_integer() const override;
    String to_string() const override;
    Boolean to_boolean() const override;
    Real to_real() const override;
};

#endif // VALUES_H
