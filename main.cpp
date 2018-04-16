#include <iostream>
#include "lexeme.h"
#include "lexical.h"
#include "syntax.h"

int main()
{
    std::string str, cur_str;
    LexicalAnalyzer lexical;
    SyntaxAnalyzer syntax;
    LexemeArray result;
    while (true) {
        str = "";
        do {
            std::getline(std::cin, cur_str);
            str += cur_str + "\n";
        } while (cur_str != "");
        try {
            lexical.parse_string(str);
            result = lexical.get_lexemes();
            syntax.parse_array(result);
            std::cout << "ok" << std::endl;
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}
