#include <iostream>
#include "exceptions.h"
#include "program.h"

Program::Program(const std::vector<ProgramNode> &program, VariableID variables_count): program(program)
{
    variables.resize(variables_count);
}

void Program::clear()
{
    pos = 0;
    for (size_t i = 0; i < variables.size(); i++) {
        if (variables[i] != NULL) {
            delete variables[i];
            variables[i] = NULL;
        }
    }

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
    clear();
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
            stack.clear();
            continue;
        case opJump:
            right = pop();
            left = pop();
            if (!left->to_boolean()) {
                pos = right->to_integer();
                stack.clear();
            }
            delete left;
            delete right;
            continue;
        case opLoadVariable:
            left = pop();
            id = left->to_integer();
            if (variables[id] != NULL) {
                push(variables[id]->clone());
            } else {
                throw InterpretationError("Uninitialized variable used.");
            }
            delete left;
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
                push(operation_execute(op, left));
                delete left;
            } else {
                right = pop();
                left = pop();
                push(operation_execute(op, left, right));
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
        switch(program[i].type) {
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
    clear();
}
