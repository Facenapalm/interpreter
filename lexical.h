#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <string>
#include "config.h"
#include "lexeme.h"

class LexicalAnalyzer {
private:
    typedef void (LexicalAnalyzer::*AnalyzerState)();

    LexemeArray result;
    bool ready;

    AnalyzerState state;
    std::string buff;
    unsigned lexeme_line;
    unsigned lexeme_column;

    std::istream *input;
    char cur_char;
    unsigned line;
    unsigned column;

    void get_next_char();
    void buff_char();
    void push_lexeme(LexemeType type);

    void transition(AnalyzerState new_state);
    void transition_buff(AnalyzerState new_state);
    void transition_push(AnalyzerState new_state, LexemeType type);
    void transition_eps(AnalyzerState new_state);
    void transition_push_eps(AnalyzerState new_state, LexemeType type);
    void transition_error(const std::string &message);

    void state_start();
    void state_after_operand();
    void state_read_identificator();
    void state_read_sign();
    void state_read_int();
    void state_read_dot();
    void state_read_real();
    void state_read_comparison();
    void state_read_string();
    void state_read_string_escape();
    void state_read_comment_start();
    void state_read_comment_start_AO();
    void state_read_comment();
    void state_read_comment_AO();
    void state_read_comment_end();
    void state_read_comment_end_AO();

    void process();
public:
    LexicalAnalyzer();
    void parse_stream(std::istream &stream);
    void parse_string(const std::string &str);
    const LexemeArray &get_lexemes() const;
};

#endif // LEXICAL_H
