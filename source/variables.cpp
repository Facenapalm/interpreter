#include "variables.h"

void VariablesTable::clear()
{
    data.clear();
}

bool VariablesTable::register_name(const std::string &name, ValueType type)
{
    if (get_number(name) != -1) {
        return false;
    }
    data.push_back({ size(), name, type });
    return true;
}

VariableID VariablesTable::get_number(const std::string &name) const
{
    for (auto i = data.begin(); i != data.end(); i++) {
        if (i->name == name) {
            return i->number;
        }
    }
    return -1;
}

ValueType VariablesTable::get_type(VariableID number) const
{
    return data[number].type;
}

VariableID VariablesTable::size() const
{
    return (VariableID)data.size();
}
