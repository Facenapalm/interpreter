#include <sstream>
#include "config.h"
#include "exceptions.h"
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
        cur_lexeme_type = ltNone;
    } else {
        cur_lexeme = &lexemes[pos++];
        cur_lexeme_type = cur_lexeme->get_type();
    }
}

void SyntaxAnalyzer::check_lexeme(LexemeType lexeme, const std::string &error_message)
{
    if (cur_lexeme_type != lexeme) {
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
    throw SyntaxError(stream.str());
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
    throw SemanticError(stream.str());
}

void SyntaxAnalyzer::throw_type_mismatch(const Lexeme *where, ValueType left, ValueType right)
{
    throw_semantic_error(where, "type mismatch (" + value_type_to_string(left) +
                                          " and " + value_type_to_string(right) + ")");
}

void SyntaxAnalyzer::gen_constant(ValueType type, const std::string &value)
{
    ProgramNode result;
    result.type = ntValue;
    switch (type) {
    case vtInteger:
        result.data.value = new IntegerValue(value);
        break;
    case vtString:
        result.data.value = new StringValue(value);
        break;
    case vtBoolean:
        result.data.value = new BooleanValue(value);
        break;
    case vtReal:
        result.data.value = new RealValue(value);
        break;
    default:
        throw std::runtime_error("Unknown constant type");
    }
    program.push_back(result);
}

void SyntaxAnalyzer::gen_constant(Integer value)
{
    ProgramNode result;
    result.type = ntValue;
    result.data.value = new IntegerValue(value);
    program.push_back(result);
}

void SyntaxAnalyzer::gen_operation(Operation operation)
{
    ProgramNode result;
    result.type = ntOperation;
    result.data.operation = operation;
    program.push_back(result);
}


void SyntaxAnalyzer::state_program()
{
    #ifdef COMPARISON_CHAIN_SUPPORT
    variables.register_name("", vtNone); // 0th temp variable
    #endif // COMPARISON_CHAIN_SUPPORT
    check_lexeme(ltProgram, "program should starts with 'program' keyword");
    check_lexeme(ltBlockOpen, "expected '{'");
    state_descriptions();
    state_operators();
    check_lexeme(ltBlockClose, "expected '}'");
    check_lexeme(ltNone, "unexpected continuation after program end");
}

void SyntaxAnalyzer::state_descriptions()
{
    while (cur_lexeme_type >= ltTypesStart && cur_lexeme_type <= ltTypesEnd) {
        ValueType variable_type = keyword_to_value_type(cur_lexeme_type);
        get_next_lexeme();
        state_description(variable_type);
        check_lexeme(ltSemicolon, "expected ';'");
    }
}

void SyntaxAnalyzer::state_description(ValueType variable_type)
{
    state_variable(variable_type);
    while (cur_lexeme_type == ltComma) {
        get_next_lexeme();
        state_variable(variable_type);
    }
}

void SyntaxAnalyzer::state_variable(ValueType variable_type)
{
    Lexeme *lexeme = cur_lexeme;
    check_lexeme(ltIdentificator, "is not a valid identificator");

    if (!variables.register_name(lexeme->get_value(), variable_type)) {
        throw_semantic_error(lexeme, "variable with the same name has already defined");
    }
    VariableID id = variables.get_number(lexeme->get_value());

    if (cur_lexeme_type == ltAssign) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        if (cur_lexeme_type >= ltConstantsStart && cur_lexeme_type <= ltConstantsEnd) {
            ValueType constant_type = constant_to_value_type(cur_lexeme_type);
            if (variable_type != constant_type) {
                throw_type_mismatch(lexeme, variable_type, constant_type);
            }
            gen_constant(constant_type, cur_lexeme->get_value());
            gen_constant(id);
            gen_operation(opSaveVariable);
            gen_operation(opClearStack);
            get_next_lexeme();
        } else {
            throw_syntax_error("bad initialization");
        }
    }
}

void SyntaxAnalyzer::state_operators()
{
    while (cur_lexeme_type != ltBlockClose) {
        state_operator();
    }
}

void SyntaxAnalyzer::state_operator()
{
    Lexeme *lexeme = cur_lexeme;
    VariableID id;

    switch (cur_lexeme_type) {
    case ltNone:
        throw_syntax_error("expected '}'");
        break;
    case ltIf: // TODO
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
        check_lexeme(ltBracketClose, "expected ')'");
        state_operator();
        if (cur_lexeme_type == ltElse) {
            get_next_lexeme();
            state_operator();
        }
        break;
    case ltWhile: // TODO
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
        check_lexeme(ltBracketClose, "expected ')'");
        state_operator();
        break;
    case ltDo: // TODO
        get_next_lexeme();
        state_operator();
        check_lexeme(ltWhile, "expected 'while' keyword");
        check_lexeme(ltBracketOpen, "expected '('");
        if (state_expression().type != vtBoolean) {
            throw_semantic_error(lexeme, "condition should be boolean");
        }
        check_lexeme(ltBracketClose, "expected ')'");
        break;
    case ltContinue: // TODO
        get_next_lexeme();
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltBreak: // TODO
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

        gen_operation(opReadLn);
        switch (variables.get_type(id)) {
        case vtInteger:
            gen_operation(opIntPlusUn);
            break;
        case vtReal:
            gen_operation(opRealPlusUn);
            break;
        case vtBoolean:
            throw_semantic_error(lexeme, "can't read boolean");
        default:
            break;
        }
        gen_constant(id);
        gen_operation(opSaveVariable);
        gen_operation(opClearStack);
        break;
    case ltWrite:
        get_next_lexeme();
        check_lexeme(ltBracketOpen, "expected '('");
        state_expression();
        gen_operation(opWrite);
        while (cur_lexeme_type == ltComma) {
            get_next_lexeme();
            state_expression();
            gen_operation(opWrite);
        }
        check_lexeme(ltBracketClose, "expected ')'");
        gen_operation(opWriteLn);
        check_lexeme(ltSemicolon, "expected ';'");
        break;
    case ltBlockOpen:
        check_lexeme(ltBlockOpen, "expected '{'");
        state_operators();
        check_lexeme(ltBlockClose, "expected '}'");
        break;
    default:
        state_expression();
        check_lexeme(ltSemicolon, "expected ';'");
        gen_operation(opClearStack);
        break;
    }
}

ValueInfo SyntaxAnalyzer::state_expression()
{
    Lexeme *lexeme;
    ValueInfo cur, prev, first;
    std::vector<ProgramNode> variable_links;

    first = cur = state_expression_or();
    while (cur_lexeme_type == ltAssign) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;

        if (!prev.is_var) {
            throw_semantic_error(lexeme, "assignation to non-variable");
        }
        program.pop_back();
        variable_links.push_back(program.back());
        program.pop_back();

        cur = state_expression_or();
        if ((cur.type == vtString) ^ (prev.type == vtString) ||
            (cur.type == vtBoolean) ^ (prev.type == vtBoolean)) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }
    }
    while (!variable_links.empty()) {
        program.push_back(variable_links.back());
        variable_links.pop_back();
        gen_operation(opSaveVariable);
    }
    return first;
}

ValueInfo SyntaxAnalyzer::state_expression_or()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_and();
    while (cur_lexeme_type == ltOr) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_and();
        cur.is_var = false;

        if (cur.type != vtBoolean || prev.type != vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        gen_operation(opBoolOr);
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_and()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_cmp();
    while (cur_lexeme_type == ltAnd) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_cmp();
        cur.is_var = false;

        if (cur.type != vtBoolean || prev.type != vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        gen_operation(opBoolAnd);
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_cmp()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;
    #ifdef COMPARISON_CHAIN_SUPPORT
    int chain_length = 0;
    #endif // COMPARISON_CHAIN_SUPPORT

    cur = state_expression_sum();
    while (cur_lexeme_type >= ltComparersStart && cur_lexeme_type <= ltComparersEnd) {
        #ifdef COMPARISON_CHAIN_SUPPORT
        if (chain_length > 0) {
            gen_constant(0);
            gen_operation(opLoadVariable);
        }
        #endif // COMPARISON_CHAIN_SUPPORT

        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_sum();
        cur.is_var = false;

        #ifdef COMPARISON_CHAIN_SUPPORT
        gen_constant(0);
        gen_operation(opSaveVariable);
        #endif // COMPARISON_CHAIN_SUPPORT

        if (cur.type == vtString && prev.type == vtString) {
            switch (lexeme->get_type()) {
            case ltSm:
                gen_operation(opStrSm);
                break;
            case ltGr:
                gen_operation(opStrGr);
                break;
            case ltEq:
                gen_operation(opStrEq);
                break;
            case ltNotEq:
                gen_operation(opStrNotEq);
                break;
            default:
                throw_type_mismatch(lexeme, prev.type, cur.type);
            };
        } else {
            if (cur.type == vtString || prev.type == vtString ||
                cur.type == vtBoolean || prev.type == vtBoolean) {
                throw_type_mismatch(lexeme, prev.type, cur.type);
            }

            if (cur.type == vtReal || prev.type == vtReal) {
                switch (lexeme->get_type()) {
                case ltSm:
                    gen_operation(opRealSm);
                    break;
                case ltGr:
                    gen_operation(opRealGr);
                    break;
                case ltSmEq:
                    gen_operation(opRealSmEq);
                    break;
                case ltGrEq:
                    gen_operation(opRealGrEq);
                    break;
                case ltEq:
                    gen_operation(opRealEq);
                    break;
                case ltNotEq:
                    gen_operation(opRealNotEq);
                    break;
                default:
                    throw_type_mismatch(lexeme, prev.type, cur.type);
                };
            } else {
                switch (lexeme->get_type()) {
                case ltSm:
                    gen_operation(opIntSm);
                    break;
                case ltGr:
                    gen_operation(opIntGr);
                    break;
                case ltSmEq:
                    gen_operation(opIntSmEq);
                    break;
                case ltGrEq:
                    gen_operation(opIntGrEq);
                    break;
                case ltEq:
                    gen_operation(opIntEq);
                    break;
                case ltNotEq:
                    gen_operation(opIntNotEq);
                    break;
                default:
                    throw_type_mismatch(lexeme, prev.type, cur.type);
                };
            }
        }

        #ifdef COMPARISON_CHAIN_SUPPORT
        chain_length += 1;
        #else
        cur.type = vtBoolean;
        #endif // COMPARISON_CHAIN_SUPPORT
    }
    #ifdef COMPARISON_CHAIN_SUPPORT
    if (chain_length > 0) {
        cur.type = vtBoolean;
    }
    for (int i = 1; i < chain_length; i++) {
        gen_operation(opBoolAnd);
    }
    #endif // COMPARISON_CHAIN_SUPPORT
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_sum()
{
    Lexeme *lexeme;
    ValueInfo cur, prev;

    cur = state_expression_mul();
    while (cur_lexeme_type == ltPlus || cur_lexeme_type == ltMinus) {
        lexeme = cur_lexeme;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_mul();
        cur.is_var = false;

        if (lexeme->get_type() == ltPlus && cur.type == vtString && prev.type == vtString) {
            gen_operation(opStrPlus);
            cur.type = vtString;
            continue;
        }

        if (cur.type == vtString || prev.type == vtString ||
            cur.type == vtBoolean || prev.type == vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        if (cur.type == vtReal || prev.type == vtReal) {
            gen_operation(lexeme->get_type() == ltPlus ? opRealPlus : opRealMinus);
            cur.type = vtReal;
        } else {
            gen_operation(lexeme->get_type() == ltPlus ? opIntPlus : opIntMinus);
            cur.type = vtInteger;
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_mul()
{
    Lexeme *lexeme;
    LexemeType lexeme_type;
    ValueInfo cur, prev;

    cur = state_expression_un();
    while (cur_lexeme_type == ltMul || cur_lexeme_type == ltDiv || cur_lexeme_type == ltMod) {
        lexeme = cur_lexeme;
        lexeme_type = cur_lexeme_type;
        get_next_lexeme();
        prev = cur;
        cur = state_expression_un();
        cur.is_var = false;

        if (cur.type == vtString || prev.type == vtString ||
            cur.type == vtBoolean || prev.type == vtBoolean) {
            throw_type_mismatch(lexeme, prev.type, cur.type);
        }

        if (cur.type == vtReal || prev.type == vtReal) {
            if (lexeme_type == ltMul) {
                gen_operation(opRealMul);
            } else if (lexeme_type == ltDiv) {
                gen_operation(opRealDiv);
            } else if (lexeme_type == ltMod) {
                throw_type_mismatch(lexeme, prev.type, cur.type);
            }
            cur.type = vtReal;
        } else {
            if (lexeme_type == ltMul) {
                gen_operation(opIntMul);
            } else if (lexeme_type == ltDiv) {
                gen_operation(opIntDiv);
            } else if (lexeme_type == ltMod) {
                gen_operation(opIntMod);
            }
            cur.type = vtInteger;
        }
    }
    return cur;
}

ValueInfo SyntaxAnalyzer::state_expression_un()
{
    if (cur_lexeme_type >= ltUnaryOperationsStart && cur_lexeme_type <= ltUnaryOperationsEnd) {
        Lexeme *lexeme = cur_lexeme;
        LexemeType lexeme_type = cur_lexeme_type;
        ValueInfo result;
        bool correct;

        get_next_lexeme();
        result = state_expression_un();
        result.is_var = false;
        switch (result.type) {
        case vtInteger:
            if (lexeme_type == ltPlusUn) {
                gen_operation(opIntPlusUn);
            } else if (lexeme_type == ltMinusUn) {
                gen_operation(opIntMinusUn);
            } else {
                correct = false;
            }
            break;
        case vtString:
            if (lexeme_type == ltPlusUn) {
                gen_operation(opStrPlusUn);
            } else {
                correct = false;
            }
            break;
        case vtBoolean:
            if (lexeme_type == ltNot) {
                gen_operation(opBoolNot);
            } else {
                correct = false;
            }
            break;
        case vtReal:
            if (lexeme_type == ltPlusUn) {
                gen_operation(opRealPlusUn);
            } else if (lexeme_type == ltMinusUn) {
                gen_operation(opRealMinusUn);
            } else {
                correct = false;
            }
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
    if (cur_lexeme_type >= ltConstantsStart && cur_lexeme_type <= ltConstantsEnd) {
        result.type = constant_to_value_type(cur_lexeme_type);
        result.is_var = false;
        gen_constant(result.type, cur_lexeme->get_value());
        get_next_lexeme();
    } else if (cur_lexeme_type == ltIdentificator) {
        VariableID id = variables.get_number(cur_lexeme->get_value());
        if (id < 0) {
            throw_semantic_error(cur_lexeme, "variable is not defined");
        }
        result.type = variables.get_type(id);
        result.is_var = true;
        gen_constant(id);
        gen_operation(opLoadVariable);
        get_next_lexeme();
    } else if (cur_lexeme_type == ltBracketOpen) {
        get_next_lexeme();
        result = state_expression();
        result.is_var = false;
        if (cur_lexeme_type == ltBracketClose) {
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

    program.clear();
    variables.clear();
    ready = false;

    state_program();
    ready = true;
}

Program *SyntaxAnalyzer::get_program()
{
    if (ready) {
        return new Program(program, variables.size());
    } else {
        throw std::runtime_error("Program is not ready");
    }
}
