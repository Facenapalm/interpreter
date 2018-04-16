#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <string>

#include "lexeme.h"

class LexicalAnalyzer {
private:
    enum AnalyzerState {
        asStart,
        asAfterOperand,
        asReadIdentificator,
        asReadSign,
        asReadInt,
        asReadDot,
        asReadReal,
        asReadComparison,
        asReadString,
        asReadStringEscape,
        asReadCommentStart,
        asReadCommentStartAO,
        asReadComment,
        asReadCommentAO,
        asReadCommentEnd,
        asReadCommentEndAO,
        asDone,
        asError
    };

    LexemeArray result;

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

    void throw_exception(const std::string &message);

    void process();
public:
    LexicalAnalyzer();
    void parse_stream(std::istream &stream);
    void parse_string(const std::string &str);
    const LexemeArray &get_lexemes() const;
};

#endif // LEXICAL_H
