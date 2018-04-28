#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <vector>
#include "values.h"

typedef Integer VariableID;

class VariablesTable {
private:
    struct VariableInfo {
        VariableID number;
        std::string name;
        ValueType type;
    };

    std::vector<VariableInfo> data;
public:
    void clear();
    bool register_name(const std::string &name, ValueType type);
    VariableID get_number(const std::string &name) const;
    ValueType get_type(VariableID number) const;
    VariableID size() const;
};

#endif // VARIABLES_H
