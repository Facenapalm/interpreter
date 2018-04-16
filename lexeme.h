#ifndef LEXEME_H
#define LEXEME_H

#include <iostream>
#include <string>
#include <vector>

enum LexemeType {
    ltNone,
    // keywords and types
    ltProgram,
    ltIf,
    ltElse,
    ltWhile,
    ltDo,
    ltBreak,
    ltContinue,
    ltRead,
    ltWrite,
    ltInt,
    ltString,
    ltBoolean,
    ltReal,
    // identificator
    ltIdentificator,
    // constants
    ltConstInt,
    ltConstString,
    ltConstBoolean,
    ltConstReal,
    // separators
    ltComma,
    ltSemicolon,
    ltBlockOpen,
    ltBlockClose,
    ltBracketOpen,
    ltBracketClose,
    // unary operations
    ltNot,
    ltPlusUn,
    ltMinusUn,
    // binary operations
    ltMul,
    ltDiv,
    ltMod,
    ltPlus,
    ltMinus,
    ltSm,
    ltGr,
    ltSmEq,
    ltGrEq,
    ltEq,
    ltNotEq,
    ltAnd,
    ltOr,
    // assignment
    ltAssign,

    // labels
    ltKeywordsStart = ltProgram,
    ltKeywordsEnd = ltReal,
    ltTypesStart = ltInt,
    ltTypesEnd = ltReal,
    ltConstantsStart = ltConstInt,
    ltConstantsEnd = ltConstReal,
    ltSeparatorsStart = ltComma,
    ltSeparatorsEnd = ltBracketClose,
    ltUnaryOperationsStart = ltNot,
    ltUnaryOperationsEnd = ltMinusUn,
    ltBinaryOperationsStart = ltMul,
    ltBinaryOperationsEnd = ltOr,
    ltComparersStart = ltSm,
    ltComparersEnd = ltNotEq,
};

class Lexeme {
private:
    LexemeType type;
    std::string value;

    unsigned line;
    unsigned pos;

    std::string stringify_type();
    std::string stringify_value();
public:
    Lexeme(LexemeType type, std::string value, unsigned line, unsigned pos);
    LexemeType get_type();
    std::string get_value();
    void print(std::ostream &stream);
};

typedef std::vector<Lexeme> LexemeArray;

#endif // LEXEME_H
