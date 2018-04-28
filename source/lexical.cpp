#include <sstream>
#include "exceptions.h"
#include "lexical.h"

LexicalAnalyzer::LexicalAnalyzer(bool case_insensetive, bool alternative_names):
    case_insensetive(case_insensetive), alternative_names(alternative_names),
    ready(false), state(NULL), lexeme_line(0), lexeme_column(0),
    input(NULL), cur_char('\0'), line(0), column(0)
{
    buff.reserve(256);
}

void LexicalAnalyzer::get_next_char()
{
    if (!(*input >> std::noskipws >> cur_char)) {
        cur_char = '\0';
    }
    if (case_insensetive &&
        state != &LexicalAnalyzer::state_string &&
        state != &LexicalAnalyzer::state_escape &&
        cur_char >= 'A' && cur_char <= 'Z') {
        cur_char -= 'A';
        cur_char += 'a';
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
    state = new_state;
    get_next_char();
}

void LexicalAnalyzer::transition_buff(AnalyzerState new_state)
{
    buff_char();
    state = new_state;
    get_next_char();
}

void LexicalAnalyzer::transition_push(AnalyzerState new_state, LexemeType type)
{
    buff_char();
    push_lexeme(type);
    state = new_state;
    get_next_char();
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
    std::stringstream stream;
    stream << "Lexical error: " << message <<
        " (line " << line << ", column " << column << ")";
    throw LexicalError(stream.str());
}

static inline bool is_space(char ch)
{
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\v' || ch == '\r';
}

static inline bool is_letter(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

static inline bool is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

inline LexemeType LexicalAnalyzer::get_separator_type()
{
    switch (cur_char) {
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

inline LexemeType LexicalAnalyzer::get_keyword_type()
{
    if (alternative_names) {
        if (buff == "print") {
            return ltWrite;
        } else if (buff == "str") {
            return ltString;
        } else if (buff == "bool") {
            return ltBoolean;
        }
    }
    if (buff == "program") {
        return ltProgram;
    } else if (buff == "if") {
        return ltIf;
    } else if (buff == "else") {
        return ltElse;
    } else if (buff == "while") {
        return ltWhile;
    } else if (buff == "read") {
        return ltRead;
    } else if (buff == "write") {
        return ltWrite;
    } else if (buff == "do") {
        return ltDo;
    } else if (buff == "break") {
        return ltBreak;
    } else if (buff == "continue") {
        return ltContinue;
    } else if (buff == "int") {
        return ltInt;
    } else if (buff == "string") {
        return ltString;
    } else if (buff == "boolean") {
        return ltBoolean;
    } else if (buff == "real") {
        return ltReal;
    } else if (buff == "true" || buff == "false") {
        return ltConstBoolean;
    } else if (buff == "not") {
        return ltNot;
    } else if (buff == "and") {
        return ltAnd;
    } else if (buff == "or") {
        return ltOr;
    } else {
        return ltIdentificator;
    }
}

void LexicalAnalyzer::state_start()
{
    if (cur_char == '\0') {
        transition_eps(NULL);
    } else if (is_space(cur_char)) {
        transition(&LexicalAnalyzer::state_start);
    } else if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_int);
    } else if (is_letter(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_identificator);
    } else if (cur_char == '"') {
        transition(&LexicalAnalyzer::state_string);
    } else if (cur_char == '{' || cur_char == '}' || cur_char == '(' ||
               cur_char == ',' || cur_char == ';') {
        transition_push(&LexicalAnalyzer::state_start, get_separator_type());
    } else if (cur_char == ')') {
        transition_push(&LexicalAnalyzer::state_after_operand, ltBracketClose);
    } else if (cur_char == '+' || cur_char == '-') {
        transition_buff(&LexicalAnalyzer::state_sign);
    } else if (cur_char == '*') {
        transition_push(&LexicalAnalyzer::state_start, ltMul);
    } else if (cur_char == '%') {
        transition_push(&LexicalAnalyzer::state_start, ltMod);
    } else if (cur_char == '/') {
        transition_buff(&LexicalAnalyzer::state_comment_start);
    } else if (cur_char == '=' || cur_char == '!' ||
               cur_char == '<' || cur_char == '>') {
        transition_buff(&LexicalAnalyzer::state_comparison);
    } else {
        transition_error(std::string() +
            "unexpected symbol '" + cur_char + "'");
    }
}

void LexicalAnalyzer::state_after_operand()
{
    if (is_space(cur_char)) {
        transition(&LexicalAnalyzer::state_after_operand);
    } else if (cur_char == '+') {
        transition_push(&LexicalAnalyzer::state_start, ltPlus);
    } else if (cur_char == '-') {
        transition_push(&LexicalAnalyzer::state_start, ltMinus);
    } else if (cur_char == '/') {
        transition_buff(&LexicalAnalyzer::state_comment_start_AO);
    } else {
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_identificator()
{
    if (is_letter(cur_char) || is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_identificator);
    } else {
        LexemeType type = get_keyword_type();
        transition_push_eps(type == ltIdentificator ?
                            &LexicalAnalyzer::state_after_operand :
                            &LexicalAnalyzer::state_start, type);
    }
}

void LexicalAnalyzer::state_sign()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_int);
    } else {
        transition_push_eps(&LexicalAnalyzer::state_start, buff == "+" ? ltPlusUn : ltMinusUn);
    }
}

void LexicalAnalyzer::state_int()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_int);
    } else if (cur_char == '.') {
        transition_buff(&LexicalAnalyzer::state_dot);
    } else if (is_letter(cur_char)) {
        transition_error(std::string() +
            "unexpected symbol '" + cur_char + "' after number");
    } else {
        transition_push_eps(&LexicalAnalyzer::state_after_operand, ltConstInt);
    }
}

void LexicalAnalyzer::state_dot()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_real);
    } else {
        transition_error(std::string() +
            "expected fractional part of number, got '" + cur_char + "'");
    }
}

void LexicalAnalyzer::state_real()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_real);
    } else if (is_letter(cur_char) || cur_char == '.') {
        transition_error(std::string() +
            "unexpected symbol '" + cur_char + "' after number");
    } else {
        transition_push_eps(&LexicalAnalyzer::state_after_operand, ltConstReal);
    }
}

void LexicalAnalyzer::state_comparison()
{
    if (cur_char == '=') {
        switch (buff[0]) {
        case '=':
            transition_push(&LexicalAnalyzer::state_start, ltEq);
            break;
        case '<':
            transition_push(&LexicalAnalyzer::state_start, ltSmEq);
            break;
        case '>':
            transition_push(&LexicalAnalyzer::state_start, ltGrEq);
            break;
        case '!':
            transition_push(&LexicalAnalyzer::state_start, ltNotEq);
            break;
        default:
            throw std::runtime_error("StateReadComparison fail");
        }
    } else {
        switch (buff[0]) {
        case '=':
            transition_push_eps(&LexicalAnalyzer::state_start, ltAssign);
            break;
        case '<':
            transition_push_eps(&LexicalAnalyzer::state_start, ltSm);
            break;
        case '>':
            transition_push_eps(&LexicalAnalyzer::state_start, ltGr);
            break;
        case '!':
            transition_error("unexpected symbol '!'");
        default:
            throw std::runtime_error("StateReadComparison fail");
        }
    }
}

void LexicalAnalyzer::state_string()
{
    if (cur_char == '\\') {
        transition(&LexicalAnalyzer::state_escape);
    } else if (cur_char == '"') {
        push_lexeme(ltConstString);
        transition(&LexicalAnalyzer::state_after_operand);
    } else if (cur_char == '\0' || cur_char == '\n') {
        transition_error("unclosed string");
    } else {
        transition_buff(&LexicalAnalyzer::state_string);
    }
}

void LexicalAnalyzer::state_escape()
{
    if (cur_char == 'n') {
        buff += '\n';
        transition(&LexicalAnalyzer::state_string);
    } else if (cur_char == '\0') {
        transition_error("unclosed string");
    } else {
        transition_buff(&LexicalAnalyzer::state_string);
    }
}

void LexicalAnalyzer::state_comment_start()
{
    if (cur_char == '*') {
        buff = "";
        transition(&LexicalAnalyzer::state_comment);
    } else {
        push_lexeme(ltDiv);
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_comment_start_AO()
{
    if (cur_char == '*') {
        buff = "";
        transition(&LexicalAnalyzer::state_comment_AO);
    } else {
        push_lexeme(ltDiv);
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_comment()
{
    if (cur_char == '\0') {
        transition_error("unclosed comment");
    } else if (cur_char == '*') {
        transition(&LexicalAnalyzer::state_comment_end);
    } else {
        transition(&LexicalAnalyzer::state_comment);
    }
}

void LexicalAnalyzer::state_comment_AO()
{
    if (cur_char == '\0') {
        transition_error("unclosed comment");
    } else if (cur_char == '*') {
        transition(&LexicalAnalyzer::state_comment_end_AO);
    } else {
        transition(&LexicalAnalyzer::state_comment_AO);
    }
}

void LexicalAnalyzer::state_comment_end()
{
    if (cur_char == '/') {
        transition(&LexicalAnalyzer::state_start);
    } else {
        transition_eps(&LexicalAnalyzer::state_comment);
    }
}

void LexicalAnalyzer::state_comment_end_AO()
{
    if (cur_char == '/') {
        transition(&LexicalAnalyzer::state_after_operand);
    } else {
        transition_eps(&LexicalAnalyzer::state_comment_AO);
    }
}


void LexicalAnalyzer::process()
{
    ready = false;
    result.clear();
    buff = "";
    line = 1;
    column = 0;
    transition(&LexicalAnalyzer::state_start);
    while (state) {
        (this->*state)();
    }
    ready = true;
}

void LexicalAnalyzer::parse_stream(std::istream &stream)
{
    input = &stream;
    process();
}

void LexicalAnalyzer::parse_string(const std::string &str)
{
    std::istringstream stream(str);
    parse_stream(stream);
}

const LexemeArray &LexicalAnalyzer::get_lexemes() const
{
    if (!ready) {
        throw std::runtime_error("Attempt to get lexems from analyzer in error state");
    }
    return result;
}
