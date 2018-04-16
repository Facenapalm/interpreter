#include <sstream>
#include <stdexcept>

#include "syntax.h"

SyntaxAnalyzer::SyntaxAnalyzer(): ready(false) {}

void SyntaxAnalyzer::get_next_lexeme()
{
    if (pos == lexemes.size()) {
        cur_lexeme = NULL;
        cur_type = ltNone;
    } else {
        cur_lexeme = &lexemes[pos++];
        cur_type = cur_lexeme->get_type();
    }
}

void SyntaxAnalyzer::check_lexeme(LexemeType lexeme, const std::string &exception)
{
    if (cur_type != lexeme) {
        throw_syntax_error(exception);
    } else {
        get_next_lexeme();
    }
}

void SyntaxAnalyzer::throw_syntax_error(const std::string &message)
{
    std::stringstream stream;
    stream << "Syntax error at ";
    if (cur_lexeme != NULL) {
        cur_lexeme->print(stream);
    } else {
        stream << "end of file";
    }
    if (message != "") {
        stream << ": " << message;
    }
    throw std::runtime_error(stream.str());
}

void SyntaxAnalyzer::state_program()
{
    check_lexeme(ltProgram, "program should starts with 'program' keyword");
    check_lexeme(ltBlockOpen, "expected '{'");
    state_descriptions();
    state_operators();
    check_lexeme(ltBlockClose, "expected '}'");
    check_lexeme(ltNone, "unexpected continuation after program end");
}

void SyntaxAnalyzer::state_descriptions()
{
    while (cur_type >= ltTypesStart && cur_type <= ltTypesEnd) {
        get_next_lexeme();
        state_description();
        check_lexeme(ltSemicolon, "expected ';'");
    }
}

void SyntaxAnalyzer::state_description()
{
    state_variable();
    while (cur_type == ltComma) {
        get_next_lexeme();
        state_variable();
    }
}

void SyntaxAnalyzer::state_variable()
{
    check_lexeme(ltIdentificator, "is not a valid identificator");
    if (cur_type == ltAssign) {
        get_next_lexeme();
        if (cur_type >= ltConstantsStart && cur_type <= ltConstantsEnd) {
            get_next_lexeme();
        } else {
            throw_syntax_error("bad initialization");
        }
    }
}

void SyntaxAnalyzer::state_operators()
{
    while (cur_type != ltBlockClose) {
        state_operator();
    }
}

void SyntaxAnalyzer::state_operator()
{
    switch (cur_type) {
    case ltNone:
        throw_syntax_error("expected '}'");
        break;
    case ltIf:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        state_expression();
        check_lexeme(ltBracketClose, "expected ')'");
        state_operator();
        if (cur_type == ltElse) {
            get_next_lexeme();
            state_operator();
        }
        break;
    case ltWhile:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        state_expression();
        check_lexeme(ltBracketClose, "expected ')'");
        state_operator();
        break;
    case ltDo:
        get_next_lexeme();
        state_operator();
        check_lexeme(ltWhile, "expected 'while' keyword");
        check_lexeme(ltBracketOpen, "expected '('");
        state_expression();
        check_lexeme(ltBracketClose, "expected ')'");
        break;
    case ltContinue:
        get_next_lexeme();
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltBreak:
        get_next_lexeme();
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltRead:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        check_lexeme(ltIdentificator, "is not a valid identificator");
        check_lexeme(ltBracketClose, "expected ')'");
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltWrite:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        state_expression();
        while (cur_type == ltComma) {
            get_next_lexeme();
            state_expression();
        }
        check_lexeme(ltBracketClose, "expected ')'");
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltBlockOpen:
        check_lexeme(ltBlockOpen, "expected '{'");
        state_operators();
        check_lexeme(ltBlockClose, "expected '}'");
        break;
    default:
        state_expression();
        if (cur_type != ltSemicolon) {
            throw_syntax_error("expected ';'");
        }
        get_next_lexeme();
        break;
    }
}

void SyntaxAnalyzer::state_expression()
{
    state_expression_or();
    while (cur_type == ltAssign) {
        get_next_lexeme();
        state_expression_or();
    }
}

void SyntaxAnalyzer::state_expression_or()
{
    state_expression_and();
    while (cur_type == ltOr) {
        get_next_lexeme();
        state_expression_and();
    }
}

void SyntaxAnalyzer::state_expression_and()
{
    state_expression_cmp();
    while (cur_type == ltAssign) {
        get_next_lexeme();
        state_expression_cmp();
    }
}

void SyntaxAnalyzer::state_expression_cmp()
{
    state_expression_sum();
    while (cur_type >= ltComparersStart && cur_type <= ltComparersEnd) {
        get_next_lexeme();
        state_expression_sum();
    }
}

void SyntaxAnalyzer::state_expression_sum()
{
    state_expression_mul();
    while (cur_type == ltPlus || cur_type == ltMinus) {
        get_next_lexeme();
        state_expression_mul();
    }
}

void SyntaxAnalyzer::state_expression_mul()
{
    state_expression_un();
    while (cur_type == ltMul || cur_type == ltDiv || cur_type == ltMod) {
        get_next_lexeme();
        state_expression_un();
    }
}

void SyntaxAnalyzer::state_expression_un()
{
    if (cur_type >= ltUnaryOperationsStart && cur_type <= ltUnaryOperationsEnd) {
        get_next_lexeme();
        state_expression_un();
    } else {
        state_operand();
    }
}

void SyntaxAnalyzer::state_operand()
{
    if (cur_type >= ltConstantsStart && cur_type <= ltConstantsEnd) {
        get_next_lexeme();
    } else if (cur_type == ltIdentificator) {
        get_next_lexeme();
    } else if (cur_type == ltBracketOpen) {
        get_next_lexeme();
        state_expression();
        if (cur_type == ltBracketClose) {
            get_next_lexeme();
        } else {
            throw_syntax_error("expected ')'");
        }
    } else {
       throw_syntax_error("expected operand");
    }
}

void SyntaxAnalyzer::parse_array(const LexemeArray &array)
{
    ready = false;
    lexemes = array;
    pos = 0;
    get_next_lexeme();
    state_program();
}
