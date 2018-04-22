#ifndef SYNTAX_H
#define SYNTAX_H

#include "lexeme.h"
#include "variables.h"
#include "labels.h"
#include "program.h"

struct ValueInfo {
    ValueType type;
    bool is_var;
};

class SyntaxAnalyzer {
private:
    enum JumpType {
        jtUnconditional,
        jtAtTrue,
        jtAtFalse
    };
    LexemeArray lexemes;
    size_t pos;
    Lexeme *cur_lexeme;
    LexemeType cur_lexeme_type;

    ProgramNodes program;
    VariablesTable variables;
    LabelsTable labels;
    bool ready;

    void get_next_lexeme();
    void check_lexeme(LexemeType lexeme, const std::string &error_message);

    void throw_syntax_error(const std::string &message);
    void throw_semantic_error(const Lexeme *where, const std::string &message);
    void throw_type_mismatch(const Lexeme *where, ValueType left, ValueType right);

    void gen_constant(ValueType type, const std::string &value);
    void gen_constant(Integer value);
    void gen_constant();
    void gen_operation(Operation operation);
    void gen_label(LabelID label);
    void gen_jump(LabelID label, JumpType type);

    // program -> `program` `{` descriptions operands `}`
    // descriptions -> { type desctiption `;` }
    // type -> `int` | `string` | `bool` | `real`
    // desctiption -> variable { `,` variable }
    // variable -> Identificator [ `=` constant ]
    // constant -> ConstInt | ConstString | ConstBoolean | ConstReal
    // operators -> { operator }
    // operator -> `if` `(` expression `)` operator [ `else` operator ]
    //           | `while` `(` expression `)` operator
    //           | `do` operator `while` `(` expression `)` `;`
    //           | `continue` `;`
    //           | `break` `;`
    //           | `read` `(` Identificator `)` `;`
    //           | `write` `(` expression { `,` expression } `)` `;`
    //           | `{` operators `}`
    //           | expression `;`
    // expression -> expression_or { `=` expression_or }
    // expression_or -> expression_and { `or` expression_and }
    // expression_and -> expression_cmp { `and` expression_cmp }
    // expression_cmp -> expression_sum { comparer expression_sum }
    // comparer -> `<` | `>` | `<=` | `>=` | `==` | `!=`
    // expression_sum -> expression_mul { summator expression_mul }
    // summator -> `+` | `-`
    // expression_mul -> expression_un { multiplier expression_un }
    // multiplier -> `*` | `/` | `%`
    // expression_un -> unary_op expression_un | operand
    // unary_op -> `+` | `-` | `not`
    // operand -> constant | Identificator | `(` expression `)`
    void state_program();
    void state_descriptions();
    void state_description(ValueType type);
    void state_variable(ValueType type);
    void state_operators(LabelID cont_label, LabelID break_label);
    void state_operator(LabelID cont_label, LabelID break_label);
    ValueInfo state_expression();
    ValueInfo state_expression_or();
    ValueInfo state_expression_and();
    ValueInfo state_expression_cmp();
    ValueInfo state_expression_sum();
    ValueInfo state_expression_mul();
    ValueInfo state_expression_un();
    ValueInfo state_operand();
public:
    SyntaxAnalyzer();
    void parse_array(const LexemeArray &array);
    Program *get_program();
};

#endif // SYNTAX_H
