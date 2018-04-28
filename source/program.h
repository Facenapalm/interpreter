#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include "values.h"
#include "variables.h"
#include "operations.h"

enum NodeType {
    ntOperation,
    ntValue
};

union NodeData {
    Operation operation;
    Value *value;
};

struct ProgramNode {
    NodeType type;
    NodeData data;
};

typedef std::vector<ProgramNode> ProgramNodes;

class Program {
private:
    ProgramNodes program;
    std::vector<Value *> variables;
    std::vector<Value *> stack;
    size_t pos;

    void clear_variables();
    void clear_stack();

    inline void push(Value *value);
    inline Value *top();
    inline Value *pop();
public:
    Program(const ProgramNodes &program, VariableID variables_count);
    void execute(std::istream &in, std::ostream &out);
    void print(std::ostream &out);
    ~Program();
};

#endif // PROGRAM_H
