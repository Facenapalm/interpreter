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

static bool case_insensetive = false;
static bool alternative_names = false;
static bool comparison_chains = true;
static bool lazy_evaluations = false;

inline void hr()
{
    std::cout << "----" << std::endl;
}

void show_help()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "interpreter [filename] [flags]" << std::endl;
    std::cout << "If no filename specified, the program will read console input " \
        "until first empty line and then try to interpretate it. Available flags: " << std::endl;
    std::cout << "--help         - display help" << std::endl;
    std::cout << "--dump-lexemes - display tokenized program" << std::endl;
    std::cout << "--dump-rpn     - display RPN representation of a program" << std::endl;
    std::cout << "--infinite     - interpretate program over and over again" << std::endl;
    std::cout << "--case-insensetive" << std::endl;
    std::cout << "--case-sensetive [default]" << std::endl;
    std::cout << "--lazy-evaluations" << std::endl;
    std::cout << "--greed-evaluations [default]" << std::endl;
    std::cout << "--allow-altnames - print instead of write etc" << std::endl;
    std::cout << "--disallow-altnames [default]" << std::endl;
    std::cout << "--allow-cmpchains - (3 > i > 1) etc [default]" << std::endl;
    std::cout << "--disallow-cmpchains" << std::endl;
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
    LexicalAnalyzer lexical(case_insensetive, alternative_names);
    SyntaxAnalyzer syntax(comparison_chains, lazy_evaluations);
    Program *program = NULL;
    LexemeArray lexemes;

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
        program = syntax.parse(lexemes);
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
        if (program) {
            delete program;
        }
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
                return 0;
            } else if (current == "--dump-lexemes") {
                dump_lexemes = true;
            } else if (current == "--dump-rpn") {
                dump_rpn = true;
            } else if (current == "--infinite") {
                infinite = true;
            } else if (current == "--case-insensetive") {
                case_insensetive = true;
            } else if (current == "--case-sensetive") {
                case_insensetive = false;
            } else if (current == "--allow-altnames") {
                alternative_names = true;
            } else if (current == "--disallow-altnames") {
                alternative_names = false;
            } else if (current == "--allow-cmpchains") {
                comparison_chains = true;
            } else if (current == "--disallow-cmpchains") {
                comparison_chains = false;
            } else if (current == "--lazy-evaluations") {
                lazy_evaluations = true;
            } else if (current == "--greed-evaluations") {
                lazy_evaluations = false;
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
