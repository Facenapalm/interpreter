#ifndef SYNTAX_H
#define SYNTAX_H

#include "lexeme.h"

class SyntaxAnalyzer {
private:
    LexemeArray lexemes;
    size_t pos;

    // POLIZ result;
    bool ready;

    Lexeme *cur_lexeme;
    LexemeType cur_type;

    void get_next_lexeme();
    void check_lexeme(LexemeType lexeme, const std::string &exception);
    void throw_syntax_error(const std::string &message);

    void state_program();        // program -> `program` `{` descriptions operands `}`

    void state_descriptions();   // descriptions -> { type desctiption `;` }
                                 // type -> `int` | `string` | `bool` | `real`
    void state_description();    // desctiption -> variable { `,` variable }
    void state_variable();       // variable -> Identificator [ `=` constant ]
                                 // constant -> ConstInt | ConstString | ConstBoolean | ConstReal

    void state_operators();      // operators -> { operator }
    void state_operator();       // operator -> `if` `(` expression `)` operator [ `else` operator ]
                                 //           | `while` `(` expression `)` operator
                                 //           | `do` operator `while` `(` expression `)` `;`
                                 //           | `continue` `;`
                                 //           | `break` `;`
                                 //           | `read` `(` Identificator `)` `;`
                                 //           | `write` `(` expression { `,` expression } `)` `;`
                                 //           | `{` operators `}`
                                 //           | expression `;`

    void state_expression();     // expression -> expression_or { `=` expression_or }
    void state_expression_or();  // expression_or -> expression_and { `or` expression_and }
    void state_expression_and(); // expression_and -> expression_cmp { `and` expression_cmp }
    void state_expression_cmp(); // expression_cmp -> expression_sum { comparer expression_sum }
                                 // comparer -> `<` | `>` | `<=` | `>=` | `==` | `!=`
    void state_expression_sum(); // expression_sum -> expression_mul { summator expression_mul }
                                 // summator -> `+` | `-`
    void state_expression_mul(); // expression_mul -> expression_un { multiplier expression_un }
                                 // multiplier -> `*` | `/` | `%`
    void state_expression_un();  // expression_un -> unary_op expression_un | operand
                                 // unary_op -> `+` | `-` | `not`
    void state_operand();        // operand -> constant | Identificator | `(` expression `)`
public:
    SyntaxAnalyzer();
    void parse_array(const LexemeArray &array);
};

#endif // SYNTAX_H
