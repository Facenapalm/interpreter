#include <iostream>
#include "exceptions.h"
#include "program.h"

Program::Program(const std::vector<ProgramNode> &program, VariableID variables_count): program(program)
{
    variables.resize(variables_count);
}

void Program::clear_variables()
{
    for (size_t i = 0; i < variables.size(); i++) {
        if (variables[i] != NULL) {
            delete variables[i];
            variables[i] = NULL;
        }
    }
}

void Program::clear_stack()
{
    for (size_t i = 0; i < stack.size(); i++) {
        delete stack[i];
    }
    stack.clear();
}

inline void Program::push(Value *value)
{
    stack.push_back(value);
}

inline Value *Program::top()
{
    if (stack.size() == 0) {
        throw std::runtime_error("The stack is empty");
    }
    return stack.back();
}

inline Value *Program::pop()
{
    Value *result = top();
    stack.pop_back();
    return result;
}

void Program::execute(std::istream &in, std::ostream &out)
{
    pos = 0;
    clear_variables();
    clear_stack();
    while (pos < program.size()) {
        ProgramNode node = program[pos++];
        if (node.type == ntValue) {
            push(node.data.value->clone());
            continue;
        }
        Operation op = node.data.operation;

        Integer id;
        String read_data;
        Value *left, *right;
        switch (op) {
        case opClearStack:
            clear_stack();
            continue;
        case opJump:
            right = pop();
            left = pop();
            if (!left->to_boolean()) {
                pos = right->to_integer();
            }
            delete left;
            delete right;
            continue;
        case opLoadVariable:
            left = pop();
            id = left->to_integer();
            delete left;
            if (variables[id] != NULL) {
                push(variables[id]->clone());
            } else {
                throw InterpretationError("Uninitialized variable used.");
            }
            continue;
        case opSaveVariable:
            right = pop();
            left = top();
            id = right->to_integer();
            if (variables[id] != NULL) {
                delete variables[id];
            }
            variables[id] = left->clone();
            delete right;
            continue;
        case opWrite:
            left = pop();
            out << left->to_string();
            delete left;
            continue;
        case opWriteLn:
            out << "\n";
            continue;
        case opReadLn:
            std::getline(in, read_data);
            push(new StringValue(read_data));
            continue;
        case opDup:
            push(top()->clone());
            continue;
        default:
            if (operation_is_unary(op)) {
                left = pop();
                try {
                    push(operation_execute(op, left));
                } catch(...) {
                    delete left;
                    throw;
                }
                delete left;
            } else {
                right = pop();
                left = pop();
                try {
                    push(operation_execute(op, left, right));
                } catch (...) {
                    delete left;
                    delete right;
                    throw;
                }
                delete left;
                delete right;
            }
        }
    }
}

void Program::print(std::ostream &out)
{
    const std::string operations = ";FlswWrd++--*/%<>()=~++<>=~+!&|++--*/<>()=~";
    const std::string values = " isbr";
    out << "Program (" << variables.size() << " variables, "
        << program.size() << " operands)." << std::endl;
    for (size_t i = 0; i < program.size(); i++) {
        Operation op = program[i].data.operation;
        Value *value = program[i].data.value;

        out << i << "\t";
        switch (program[i].type) {
        case ntOperation:
            std::cout << "o\t" << op << "\t" << operations[op];
            break;
        case ntValue:
            std::cout << values[value->get_type()] << "\t" << value->to_string();
            break;
        }
        out << std::endl;
    }
}

Program::~Program()
{
    for (size_t i = 0; i < program.size(); i++) {
        if (program[i].type == ntValue) {
            delete program[i].data.value;
        }
    }
    clear_variables();
    clear_stack();
}
