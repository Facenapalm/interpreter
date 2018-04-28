#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <string>
#include "lexeme.h"

class LexicalAnalyzer {
private:
    typedef void (LexicalAnalyzer::*AnalyzerState)();

    bool case_insensetive;
    bool alternative_names;

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

    inline LexemeType get_separator_type();
    inline LexemeType get_keyword_type();

    void state_start();
    void state_after_operand();
    void state_identificator();
    void state_sign();
    void state_int();
    void state_dot();
    void state_real();
    void state_comparison();
    void state_string();
    void state_escape();
    void state_comment_start();
    void state_comment_start_AO();
    void state_comment();
    void state_comment_AO();
    void state_comment_end();
    void state_comment_end_AO();

    void process();
public:
    LexicalAnalyzer(bool case_insensetive=false, bool alternative_names=false);
    void parse_stream(std::istream &stream);
    void parse_string(const std::string &str);
    const LexemeArray &get_lexemes() const;
};

#endif // LEXICAL_H
