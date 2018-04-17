#include <sstream>
#include <stdexcept>

#include "syntax.h"

static inline ValueType keyword_to_value_type(LexemeType lexeme)
{
    switch (lexeme) {
    case ltInt:
        return vtInteger;
    case ltString:
        return vtString;
    case ltBoolean:
        return vtBoolean;
    case ltReal:
        return vtReal;
    default:
        return vtNone;
    }
}

static inline ValueType constant_to_value_type(LexemeType lexeme)
{
    switch (lexeme) {
    case ltConstInt:
        return vtInteger;
    case ltConstString:
        return vtString;
    case ltConstBoolean:
        return vtBoolean;
    case ltConstReal:
        return vtReal;
    default:
        return vtNone;
    }
}

static inline std::string value_type_to_string(ValueType type)
{
    switch (type) {
    case vtInteger:
        return "Integer";
    case vtString:
        return "String";
    case vtBoolean:
        return "Boolean";
    case vtReal:
        return "Real";
    default:
        return "None";
    }
}

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

void SyntaxAnalyzer::check_lexeme(LexemeType lexeme, const std::string &error_message)
{
    if (cur_type != lexeme) {
        throw_syntax_error(error_message);
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

void SyntaxAnalyzer::throw_semantic_error(const Lexeme *where, const std::string &message)
{
    std::stringstream stream;
    stream << "Semantic error";
    if (where != NULL) {
        stream << " at ";
        where->print(stream);
    }
    if (message != "") {
        stream << ": " << message;
    }
    throw std::runtime_error(stream.str());
}

void SyntaxAnalyzer::throw_type_mismatch(const Lexeme *where, ValueType left, ValueType right)
{
    throw_semantic_error(where, "type mismatch (" + value_type_to_string(left) +
                                          " and " + value_type_to_string(right) + ")");
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
        ValueType type = keyword_to_value_type(cur_type);
        get_next_lexeme();
        state_description(type);
        check_lexeme(ltSemicolon, "expected ';'");
    }
}

void SyntaxAnalyzer::state_description(ValueType type)
{
    state_variable(type);
    while (cur_type == ltComma) {
        get_next_lexeme();
        state_variable(type);
    }
}

void SyntaxAnalyzer::state_variable(ValueType type)
{
    Lexeme *lexeme = cur_lexeme;
    check_lexeme(ltIdentificator, "is not a valid identificator");

    if (!variables.register_name(lexeme->get_value(), type)) {
        throw_semantic_error(lexeme, "variable with the same name has already defined");
    }

    if (cur_type == ltAssign) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        if (cur_type >= ltConstantsStart && cur_type <= ltConstantsEnd) {
            ValueType constant_type = constant_to_value_type(cur_type);
            if (type != constant_type) {
                throw_type_mismatch(lexeme, type, constant_type);
            }
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
    Lexeme *lexeme = cur_lexeme;
    VariableID id;

    switch (cur_type) {
    case ltNone:
        throw_syntax_error("expected '}'");
        break;
    case ltIf:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
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
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
        check_lexeme(ltBracketClose, "expected ')'");
        state_operator();
        break;
    case ltDo:
        get_next_lexeme();
        state_operator();
        check_lexeme(ltWhile, "expected 'while' keyword");
        check_lexeme(ltBracketOpen, "expected '('");
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
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
        lexeme = cur_lexeme;
        check_lexeme(ltIdentificator, "is not a valid identificator");
        check_lexeme(ltBracketClose, "expected ')'");
        check_lexeme(ltSemicolon, "expected ';'");

        id = variables.get_number(lexeme->get_value());
        if (id < 0) {
            throw_semantic_error(lexeme, "variable is not defined");
        }
        if (variables.get_type(id) == vtBoolean) {
            throw_semantic_error(lexeme, "can't read boolean");
        }
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

ValueInfo SyntaxAnalyzer::state_expression()
{
    Lexeme *lexeme;
    ValueInfo cur, prev, first;

    first = cur = state_expression_or();
    while (cur_type == ltAssign) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_or();
        cur.is_var = false;

        if (!prev.is_var) {
            throw_semantic_error(lexeme, "assignation to non-variable");
        }

        if (cur.type == vtString && prev.type == vtString) {
            continue;
        }

        if (cur.type == vtBoolean && prev.type == vtBoolean) {
            continue;
        }

        if (cur.type == vtString || prev.type == vtString ||
            cur.type == vtBoolean || prev.type == vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }
    }
    return first;
}

ValueInfo SyntaxAnalyzer::state_expression_or()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_and();
    while (cur_type == ltOr) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_and();
        cur.is_var = false;

        if (cur.type != vtBoolean || prev.type != vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_and()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_cmp();
    while (cur_type == ltAnd) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_cmp();
        cur.is_var = false;

        if (cur.type != vtBoolean || prev.type != vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_cmp()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_sum();
    while (cur_type >= ltComparersStart && cur_type <= ltComparersEnd) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_sum();

        if (cur.type == vtString && prev.type == vtString) {
            ;
        } else {
            if (cur.type == vtString || prev.type == vtString ||
                cur.type == vtBoolean || prev.type == vtBoolean) {
                throw_type_mismatch(lexeme, prev.type, cur.type);
            }

            if (cur.type == vtReal || prev.type == vtReal) {
                ;
            } else {
                ;
            }
        }

        cur.is_var = false;
        cur.type = vtBoolean;
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_sum()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_mul();
    while (cur_type == ltPlus || cur_type == ltMinus) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_mul();
        cur.is_var = false;

        if (cur_type == ltPlus && cur.type == vtString && prev.type == vtString) {
            cur.type = vtString;
            continue;
        }

        if (cur.type == vtString || prev.type == vtString ||
            cur.type == vtBoolean || prev.type == vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        if (cur.type == vtReal || prev.type == vtReal) {
            cur.type = vtReal;
        } else {
            cur.type = vtInteger;
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_mul()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_un();
    while (cur_type == ltMul || cur_type == ltDiv || cur_type == ltMod) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_un();
        cur.is_var = false;

        if (cur.type == vtString || prev.type == vtString ||
            cur.type == vtBoolean || prev.type == vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        if (cur.type == vtReal || prev.type == vtReal) {
            cur.type = vtReal;
        } else {
            cur.type = vtInteger;
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_un()
{
    if (cur_type >= ltUnaryOperationsStart && cur_type <= ltUnaryOperationsEnd) {
        Lexeme *lexeme = cur_lexeme;
        LexemeType type = cur_type;
        ValueInfo result;
        bool correct;

        get_next_lexeme();
        result = state_expression_un();
        result.is_var = false;
        switch (result.type) {
        case vtInteger:
            correct = type == ltPlusUn || type == ltMinusUn;
            break;
        case vtString:
            correct = type == ltPlusUn;
            break;
        case vtBoolean:
            correct = type == ltNone;
            break;
        case vtReal:
            correct = type == ltPlusUn || type == ltMinusUn;
            break;
        default:
            correct = false;
            break;
        }
        if (!correct) {
            throw_semantic_error(lexeme, "type mismatch (" +
                                 value_type_to_string(result.type) + ")");
        }
        return result;
    } else {
        return state_operand();
    }
}

ValueInfo SyntaxAnalyzer::state_operand()
{
    ValueInfo result;
    if (cur_type >= ltConstantsStart && cur_type <= ltConstantsEnd) {
        result.type = constant_to_value_type(cur_type);
        result.is_var = false;
        get_next_lexeme();
    } else if (cur_type == ltIdentificator) {
        VariableID id = variables.get_number(cur_lexeme->get_value());
        if (id < 0) {
            throw_semantic_error(cur_lexeme, "variable is not defined");
        }
        result.type = variables.get_type(id);
        result.is_var = true;
        get_next_lexeme();
    } else if (cur_type == ltBracketOpen) {
        get_next_lexeme();
        result = state_expression();
        if (cur_type == ltBracketClose) {
            get_next_lexeme();
        } else {
            throw_syntax_error("expected ')'");
        }
    } else {
       throw_syntax_error("expected operand");
    }
    return result;
}

void SyntaxAnalyzer::parse_array(const LexemeArray &array)
{
    lexemes = array;
    pos = 0;
    get_next_lexeme();

    variables.clear();
    ready = false;

    state_program();
}
