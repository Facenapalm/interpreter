#include "lexeme.h"

Lexeme::Lexeme(LexemeType type, std::string value, unsigned line, unsigned pos):
    type(type), value(value), line(line), pos(pos) {}

LexemeType Lexeme::get_type()
{
    return type;
}

std::string Lexeme::get_value()
{
    return value;
}

std::string Lexeme::stringify_type()
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

std::string Lexeme::stringify_value()
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

void Lexeme::print(std::ostream &stream)
{
    stream << "lexeme<" << stringify_type() << "> " << stringify_value() <<
        " (line " << line << ", column " << pos << ")";
}
