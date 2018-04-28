#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

class Exception: public std::runtime_error {
public:
    explicit Exception(const std::string &message):
        std::runtime_error(message) {}
};

class LexicalError: public Exception {
public:
    explicit LexicalError(const std::string &message):
        Exception(message) {}
};

class SyntaxError: public Exception {
public:
    explicit SyntaxError(const std::string &message):
        Exception(message) {}
};

class SemanticError: public Exception {
public:
    explicit SemanticError(const std::string &message):
        Exception(message) {}
};

class InterpretationError: public Exception {
public:
    explicit InterpretationError(const std::string &message):
        Exception(message) {}
};

#endif //EXCEPTIONS_H
