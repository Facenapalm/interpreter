#include <iostream>
#include "lexeme.h"
#include "lexical.h"

int main()
{
    std::string str, cur_str;
    LexicalAnalyzer tester;
    LexemeArray result;
    while (true) {
        str = "";
        do {
            std::getline(std::cin, cur_str);
            str += cur_str + "\n";
        } while (cur_str != "");
        std::cout << "-----" << std::endl;
        try {
            tester.parse_string(str);
            result = tester.get_lexemes();
            for (size_t i = 0; i < result.size(); i++) {
                result[i].print(std::cout);
                std::cout << std::endl;
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        std::cout << "-----" << std::endl;
    }
    return 0;
}
