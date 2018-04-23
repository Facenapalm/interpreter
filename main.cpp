#include <iostream>
#include <sstream>
#include <fstream>
#include "exceptions.h"
#include "lexical.h"
#include "syntax.h"
#include "program.h"

static bool dump_lexemes = false;
static bool dump_rpn = false;
static bool infinite = false;

static void hr()
{
    std::cout << "----" << std::endl;
}

void show_help()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "interpreter [filename] [flags]" << std::endl;
    std::cout << "If no filename specified, the program will read console input " \
        "until first empty line and then try to interpretate it. Available flags: ";
    std::cout << "--dump-lexemes - display tokenized program" << std::endl;
    std::cout << "--dump-rpn     - display RPN representation of a program" << std::endl;
    std::cout << "--infinite     - interpretate program over and over again" << std::endl;
    std::cout << "--help         - displays help" << std::endl;
    hr();
}

std::string read_program()
{
    std::string result = "";
    std::string line;
    do {
        std::getline(std::cin, line);
        result += line + "\n";
    } while (line != "");
    return result;
}

void execute(std::istream &stream)
{
    LexicalAnalyzer lexical;
    SyntaxAnalyzer syntax;
    LexemeArray lexemes;
    Program *program;

    try {
        lexical.parse_stream(stream);
        lexemes = lexical.get_lexemes();
        if (dump_lexemes) {
            std::cout << "Lexemes:" << std::endl;
            for (size_t i = 0; i < lexemes.size(); i++) {
                lexemes[i].print(std::cout);
                std::cout << std::endl;
            }
            hr();
        }
        syntax.parse_array(lexemes);
        program = syntax.get_program();
        if (dump_rpn) {
            program->print(std::cout);
            hr();
        }
        program->execute(std::cin, std::cout);
        while (infinite) {
            hr();
            program->execute(std::cin, std::cout);
        }
        delete program;
    } catch (const Exception &e) {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
    bool program_specified = false;

    if (argc == 1) {
        std::cout << "Add --help argument for help." << std::endl << std::endl;
    } else {
        for (int i = 1; i < argc; i++) {
            std::string current = argv[i];
            if (current == "--help") {
                show_help();
            } else if (current == "--dump-lexemes") {
                dump_lexemes = true;
            } else if (current == "--dump-rpn") {
                dump_rpn = true;
            } else if (current == "--infinite") {
                infinite = true;
            } else {
                if (i == 1) {
                    program_specified = true;
                } else {
                    std::cout << "Unrecognized flag: " << argv[i] << std::endl;
                }
            }
        }
    }
    if (program_specified) {
        std::ifstream stream(argv[1]);
        if (stream.is_open()) {
            execute(stream);
        } else {
            std::cout << "Error: could not open file." << std::endl;
        }
    } else {
        std::istringstream stream(read_program());
        hr();
        execute(stream);
    }

    return 0;
}
