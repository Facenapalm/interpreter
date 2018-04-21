#include <iostream>
#include "exceptions.h"
#include "lexical.h"
#include "syntax.h"
#include "program.h"

int main()
{
    std::string str, cur_str;
    LexicalAnalyzer lexical;
    SyntaxAnalyzer syntax;
    Program *program;
    LexemeArray result;
    while (true) {
        str = "";
        do {
            std::getline(std::cin, cur_str);
            str += cur_str + "\n";
        } while (cur_str != "");
        try {
            std::cout << "----" << std::endl;
            lexical.parse_string(str);
            result = lexical.get_lexemes();
            syntax.parse_array(result);
            program = syntax.get_program();
            program->print(std::cout);
            std::cout << "----" << std::endl;
            program->execute(std::cin, std::cout);
            delete program;
        } catch (const Exception &e) {
            std::cout << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}
