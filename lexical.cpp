#include <sstream>
#include <stdexcept>

#include "lexical.h"

LexicalAnalyzer::LexicalAnalyzer()
{
    buff.reserve(256);
    state = asError;
}

void LexicalAnalyzer::get_next_char()
{
    if (!(*input >> std::noskipws >> cur_char)) {
        cur_char = '\0';
    }
    if (cur_char == '\n') {
        line += 1;
        column = 0;
    } else {
        column += 1;
    }
}

void LexicalAnalyzer::buff_char()
{
    if (buff == "") {
        lexeme_line = line;
        lexeme_column = column;
    }
    buff += cur_char;
}

void LexicalAnalyzer::push_lexeme(LexemeType type)
{
    result.push_back(Lexeme(type, buff, lexeme_line, lexeme_column));
    buff = "";
}

void LexicalAnalyzer::transition(AnalyzerState new_state)
{
    get_next_char();
    state = new_state;
}

void LexicalAnalyzer::transition_buff(AnalyzerState new_state)
{
    buff_char();
    get_next_char();
    state = new_state;
}

void LexicalAnalyzer::transition_push(AnalyzerState new_state, LexemeType type)
{
    buff_char();
    push_lexeme(type);
    get_next_char();
    state = new_state;
}

void LexicalAnalyzer::transition_eps(AnalyzerState new_state)
{
    state = new_state;
}

void LexicalAnalyzer::transition_push_eps(AnalyzerState new_state, LexemeType type)
{
    push_lexeme(type);
    state = new_state;
}

void LexicalAnalyzer::transition_error(const std::string &message)
{
    state = asError;
    std::stringstream stream;
    stream << "Lexical error: " << message <<
        " (line " << line << ", column " << column << ")";
    throw std::runtime_error(stream.str());
}

static inline bool is_space(char ch)
{
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\v' || ch == '\r';
}

static inline bool is_letter(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static inline bool is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static inline LexemeType get_separator_type(char ch)
{
    switch (ch) {
    case ',':
        return ltComma;
    case '{':
        return ltBlockOpen;
    case '}':
        return ltBlockClose;
    case '(':
        return ltBracketOpen;
    case ')':
        return ltBracketClose;
    default:
        return ltSemicolon;
    }
}

static inline LexemeType get_keyword_type(const std::string &str)
{
    if (str == "program") {
        return ltProgram;
    } else if (str == "if") {
        return ltIf;
    } else if (str == "else") {
        return ltElse;
    } else if (str == "while") {
        return ltWhile;
    } else if (str == "read") {
        return ltRead;
    } else if (str == "write") {
        return ltWrite;
    } else if (str == "do") {
        return ltDo;
    } else if (str == "break") {
        return ltBreak;
    } else if (str == "continue") {
        return ltContinue;
    } else if (str == "int") {
        return ltInt;
    } else if (str == "string") {
        return ltString;
    } else if (str == "boolean") {
        return ltBoolean;
    } else if (str == "real") {
        return ltReal;
    } else if (str == "true" || str == "false") {
        return ltConstBoolean;
    } else if (str == "not") {
        return ltNot;
    } else if (str == "and") {
        return ltAnd;
    } else if (str == "or") {
        return ltOr;
    } else {
        return ltIdentificator;
    }
}

void LexicalAnalyzer::process()
{
    while (state != asDone) {
        switch (state) {
        case asStart:
            if (cur_char == '\0') {
                transition_eps(asDone);
            } else if (is_space(cur_char)) {
                transition(asStart);
            } else if (is_digit(cur_char)) {
                transition_buff(asReadInt);
            } else if (is_letter(cur_char)) {
                transition_buff(asReadIdentificator);
            } else if (cur_char == '"') {
                transition(asReadString);
            } else if (cur_char == '{' || cur_char == '}' || cur_char == '(' ||
                       cur_char == ',' || cur_char == ';') {
                transition_push(asStart, get_separator_type(cur_char));
            } else if (cur_char == ')') {
                transition_push(asAfterOperand, ltBracketClose);
            } else if (cur_char == '+' || cur_char == '-') {
                transition_buff(asReadSign);
            } else if (cur_char == '*') {
                transition_push(asStart, ltMul);
            } else if (cur_char == '%') {
                transition_push(asStart, ltMod);
            } else if (cur_char == '/') {
                transition_buff(asReadCommentStart);
            } else if (cur_char == '=' || cur_char == '!' ||
                       cur_char == '<' || cur_char == '>') {
                transition_buff(asReadComparison);
            } else {
                transition_error(std::string() +
                    "unexpected symbol '" + cur_char + "'");
            }
            break;
        case asAfterOperand:
            if (is_space(cur_char)) {
                transition(asAfterOperand);
            } else if (cur_char == '+') {
                transition_push(asStart, ltPlus);
            } else if (cur_char == '-') {
                transition_push(asStart, ltMinus);
            } else if (cur_char == '/') {
                transition_buff(asReadCommentStartAO);
            } else {
                transition_eps(asStart);
            }
            break;
        case asReadIdentificator:
            if (is_letter(cur_char) || is_digit(cur_char)) {
                transition_buff(asReadIdentificator);
            } else {
                LexemeType type = get_keyword_type(buff);
                transition_push_eps(type == ltIdentificator ? asAfterOperand : asStart, type);
            }
            break;
        case asReadSign:
            if (is_digit(cur_char)) {
                transition_buff(asReadInt);
            } else {
                transition_push_eps(asStart, buff == "+" ? ltPlusUn : ltMinusUn);
            }
            break;
        case asReadInt:
            if (is_digit(cur_char)) {
                transition_buff(asReadInt);
            } else if (cur_char == '.') {
                transition_buff(asReadDot);
            } else if (is_letter(cur_char)) {
                transition_error(std::string() +
                    "unexpected symbol '" + cur_char + "' after number");
            } else {
                transition_push_eps(asAfterOperand, ltConstInt);
            }
            break;
        case asReadDot:
            if (is_digit(cur_char)) {
                transition_buff(asReadReal);
            } else {
                transition_error(std::string() +
                    "expected fractional part of number, got '" + cur_char + "'");
            }
            break;
        case asReadReal:
            if (is_digit(cur_char)) {
                transition_buff(asReadReal);
            } else if (is_letter(cur_char) || cur_char == '.') {
                transition_error(std::string() +
                    "unexpected symbol '" + cur_char + "' after number");
            } else {
                transition_push_eps(asAfterOperand, ltConstReal);
            }
            break;
        case asReadComparison:
            if (cur_char == '=') {
                switch (buff[0]) {
                case '=':
                    transition_push(asStart, ltEq);
                    break;
                case '<':
                    transition_push(asStart, ltSmEq);
                    break;
                case '>':
                    transition_push(asStart, ltGrEq);
                    break;
                case '!':
                    transition_push(asStart, ltNotEq);
                    break;
                }
            } else {
                switch (buff[0]) {
                case '=':
                    transition_push_eps(asStart, ltAssign);
                    break;
                case '<':
                    transition_push_eps(asStart, ltSm);
                    break;
                case '>':
                    transition_push_eps(asStart, ltGr);
                    break;
                case '!':
                    transition_error("unexpected symbol '!'");
                }
            }
            break;
        case asReadString:
            if (cur_char == '\\') {
                transition(asReadStringEscape);
            } else if (cur_char == '"') {
                push_lexeme(ltConstString);
                transition(asAfterOperand);
            } else if (cur_char == '\0' || cur_char == '\n') {
                transition_error("unclosed string");
            } else {
                transition_buff(asReadString);
            }
            break;
        case asReadStringEscape:
            if (cur_char == 'n') {
                buff += '\n';
                transition(asReadString);
            } else if (cur_char == '\0') {
                transition_error("unclosed string");
            } else {
                transition_buff(asReadString);
            }
            break;
        case asReadCommentStart:
        case asReadCommentStartAO:
            if (cur_char == '*') {
                transition(state == asReadCommentStart ? asReadComment : asReadCommentAO);
            } else {
                push_lexeme(ltDiv);
                transition_eps(asStart);
            }
            break;
        case asReadComment:
        case asReadCommentAO:
            if (cur_char == '\0') {
                transition_error("unclosed comment");
            } else if (cur_char == '*') {
                transition(state == asReadComment ? asReadCommentEnd : asReadCommentEndAO);
            } else {
                transition(state);
            }
            break;
        case asReadCommentEnd:
        case asReadCommentEndAO:
            if (cur_char == '/') {
                transition(state == asReadCommentEnd ? asStart : asAfterOperand);
            } else {
                transition_eps(state == asReadCommentEnd ? asReadComment : asReadCommentAO);
            }
            break;
        default: ;
        }
    }
}

void LexicalAnalyzer::parse_stream(std::istream &stream)
{
    input = &stream;

    result.clear();
    buff = "";
    line = 1;
    column = 0;

    transition(asStart);
    process();
}

void LexicalAnalyzer::parse_string(const std::string &str)
{
    std::istringstream stream(str);
    parse_stream(stream);
}

const LexemeArray &LexicalAnalyzer::get_lexemes() const
{
    if (state != asDone) {
        throw std::runtime_error("Attempt to get lexems from analyzer in error state.");
    }
    return result;
}
