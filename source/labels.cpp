#include "exceptions.h"
#include "labels.h"

LabelInfo::LabelInfo(const std::string &name): name(name), value(NULL) {}

void LabelInfo::set_value(Value *value)
{
    if (this->value != NULL) {
        throw SemanticError("Semantic error: label " + name + " defined twice.");
    }
    this->value = value;
}

void LabelInfo::add_node(size_t idx)
{
    node_indexes.push_back(idx);
}

void LabelInfo::propagate(ProgramNodes &program)
{
    size_t length = node_indexes.size();
    if (length == 0) {
        if (value != NULL) {
            delete value;
        }
        return;
    }
    if (value == NULL) {
        throw SemanticError("Semantic error: no label " + name + " found.");
    }
    program[node_indexes[0]].data.value = value;
    for (size_t i = 1; i < length; i++) {
        program[node_indexes[i]].data.value = value->clone();
    }
}

void LabelsTable::clear()
{
    labels.clear();
}

LabelID LabelsTable::new_label()
{
    LabelID result = labels.size();
    labels.push_back(LabelInfo());
    return result;
}

void LabelsTable::set_value(LabelID label, Value *value)
{
    labels[label].set_value(value);
}

void LabelsTable::add_node(LabelID label, size_t idx)
{
    labels[label].add_node(idx);
}

void LabelsTable::propagate(ProgramNodes &program)
{
    for (size_t i = 0; i < labels.size(); i++) {
        labels[i].propagate(program);
    }
    clear();
}
