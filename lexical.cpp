#include <sstream>
#include "exceptions.h"
#include "lexical.h"

LexicalAnalyzer::LexicalAnalyzer()
{
    buff.reserve(256);
    ready = false;
}

void LexicalAnalyzer::get_next_char()
{
    if (!(*input >> std::noskipws >> cur_char)) {
        cur_char = '\0';
    }
    #ifdef CASE_INSENSETIVE
    if (state != &LexicalAnalyzer::state_read_string &&
        state != &LexicalAnalyzer::state_read_string_escape &&
        cur_char >= 'A' && cur_char <= 'Z') {
        cur_char -= 'A';
        cur_char += 'a';
    }
    #endif // CASE_INSENSETIVE
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
    #ifdef ALLOW_ALTERNATIVE_NAMES
    } else if (str == "print") {
        return ltWrite;
    } else if (str == "str") {
        return ltString;
    } else if (str == "bool") {
        return ltBoolean;
    #endif // ALLOW_ALTERNATIVE_NAMES
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
        transition_buff(&LexicalAnalyzer::state_read_int);
    } else if (is_letter(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_identificator);
    } else if (cur_char == '"') {
        transition(&LexicalAnalyzer::state_read_string);
    } else if (cur_char == '{' || cur_char == '}' || cur_char == '(' ||
               cur_char == ',' || cur_char == ';') {
        transition_push(&LexicalAnalyzer::state_start, get_separator_type(cur_char));
    } else if (cur_char == ')') {
        transition_push(&LexicalAnalyzer::state_after_operand, ltBracketClose);
    } else if (cur_char == '+' || cur_char == '-') {
        transition_buff(&LexicalAnalyzer::state_read_sign);
    } else if (cur_char == '*') {
        transition_push(&LexicalAnalyzer::state_start, ltMul);
    } else if (cur_char == '%') {
        transition_push(&LexicalAnalyzer::state_start, ltMod);
    } else if (cur_char == '/') {
        transition_buff(&LexicalAnalyzer::state_read_comment_start);
    } else if (cur_char == '=' || cur_char == '!' ||
               cur_char == '<' || cur_char == '>') {
        transition_buff(&LexicalAnalyzer::state_read_comparison);
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
        transition_buff(&LexicalAnalyzer::state_read_comment_start_AO);
    } else {
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_read_identificator()
{
    if (is_letter(cur_char) || is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_identificator);
    } else {
        LexemeType type = get_keyword_type(buff);
        transition_push_eps(type == ltIdentificator ?
                            &LexicalAnalyzer::state_after_operand :
                            &LexicalAnalyzer::state_start, type);
    }
}

void LexicalAnalyzer::state_read_sign()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_int);
    } else {
        transition_push_eps(&LexicalAnalyzer::state_start, buff == "+" ? ltPlusUn : ltMinusUn);
    }
}

void LexicalAnalyzer::state_read_int()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_int);
    } else if (cur_char == '.') {
        transition_buff(&LexicalAnalyzer::state_read_dot);
    } else if (is_letter(cur_char)) {
        transition_error(std::string() +
            "unexpected symbol '" + cur_char + "' after number");
    } else {
        transition_push_eps(&LexicalAnalyzer::state_after_operand, ltConstInt);
    }
}

void LexicalAnalyzer::state_read_dot()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_real);
    } else {
        transition_error(std::string() +
            "expected fractional part of number, got '" + cur_char + "'");
    }
}

void LexicalAnalyzer::state_read_real()
{
    if (is_digit(cur_char)) {
        transition_buff(&LexicalAnalyzer::state_read_real);
    } else if (is_letter(cur_char) || cur_char == '.') {
        transition_error(std::string() +
            "unexpected symbol '" + cur_char + "' after number");
    } else {
        transition_push_eps(&LexicalAnalyzer::state_after_operand, ltConstReal);
    }
}

void LexicalAnalyzer::state_read_comparison()
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

void LexicalAnalyzer::state_read_string()
{
    if (cur_char == '\\') {
        transition(&LexicalAnalyzer::state_read_string_escape);
    } else if (cur_char == '"') {
        push_lexeme(ltConstString);
        transition(&LexicalAnalyzer::state_after_operand);
    } else if (cur_char == '\0' || cur_char == '\n') {
        transition_error("unclosed string");
    } else {
        transition_buff(&LexicalAnalyzer::state_read_string);
    }
}

void LexicalAnalyzer::state_read_string_escape()
{
    if (cur_char == 'n') {
        buff += '\n';
        transition(&LexicalAnalyzer::state_read_string);
    } else if (cur_char == '\0') {
        transition_error("unclosed string");
    } else {
        transition_buff(&LexicalAnalyzer::state_read_string);
    }
}

void LexicalAnalyzer::state_read_comment_start()
{
    if (cur_char == '*') {
        buff = "";
        transition(&LexicalAnalyzer::state_read_comment);
    } else {
        push_lexeme(ltDiv);
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_read_comment_start_AO()
{
    if (cur_char == '*') {
        buff = "";
        transition(&LexicalAnalyzer::state_read_comment_AO);
    } else {
        push_lexeme(ltDiv);
        transition_eps(&LexicalAnalyzer::state_start);
    }
}

void LexicalAnalyzer::state_read_comment()
{
    if (cur_char == '\0') {
        transition_error("unclosed comment");
    } else if (cur_char == '*') {
        transition(&LexicalAnalyzer::state_read_comment_end);
    } else {
        transition(&LexicalAnalyzer::state_read_comment);
    }
}

void LexicalAnalyzer::state_read_comment_AO()
{
    if (cur_char == '\0') {
        transition_error("unclosed comment");
    } else if (cur_char == '*') {
        transition(&LexicalAnalyzer::state_read_comment_end_AO);
    } else {
        transition(&LexicalAnalyzer::state_read_comment_AO);
    }
}

void LexicalAnalyzer::state_read_comment_end()
{
    if (cur_char == '/') {
        transition(&LexicalAnalyzer::state_start);
    } else {
        transition_eps(&LexicalAnalyzer::state_read_comment);
    }
}

void LexicalAnalyzer::state_read_comment_end_AO()
{
    if (cur_char == '/') {
        transition(&LexicalAnalyzer::state_after_operand);
    } else {
        transition_eps(&LexicalAnalyzer::state_read_comment_AO);
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
