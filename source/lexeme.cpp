#include "lexeme.h"

Lexeme::Lexeme(LexemeType type, const std::string &value, unsigned line, unsigned pos):
    type(type), value(value), line(line), pos(pos) {}

LexemeType Lexeme::get_type() const
{
    return type;
}

const std::string &Lexeme::get_value() const
{
    return value;
}

inline std::string Lexeme::stringify_type() const
{
    if (type >= ltKeywordsStart && type <= ltKeywordsEnd) {
        return "Keyword";
    } else if (type == ltIdentificator) {
        return "Identificator";
    } else if (type >= ltConstantsStart && type <= ltConstantsEnd) {
        return "Constant";
    } else if (type >= ltSeparatorsStart && type <= ltSeparatorsEnd) {
        return "Separator";
    } else if (type >= ltUnaryOperationsStart && type <= ltUnaryOperationsEnd) {
        return "UnaryOperation";
    } else if (type >= ltBinaryOperationsStart && type <= ltBinaryOperationsEnd) {
        return "BinaryOperation";
    } else if (type == ltAssign) {
        return "Assignment";
    } else {
        return "Unknown";
    }
}

inline std::string Lexeme::stringify_value() const
{
    switch (type) {
    case ltConstInt:
    case ltConstBoolean:
    case ltConstReal:
        return value;
    case ltConstString:
        return "\"" + value + "\"";
    default:
        return "'" + value + "'";
    }
}

void Lexeme::print(std::ostream &stream) const
{
    stream << "lexeme<" << stringify_type() << "> " << stringify_value() <<
        " (line " << line << ", column " << pos << ")";
}
