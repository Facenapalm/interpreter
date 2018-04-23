#ifndef LABELS_H
#define LABELS_H

#include <vector>
#include "program.h"

typedef size_t LabelID;

const LabelID undefined_label = (size_t)-1;

class LabelInfo {
private:
    std::string name;
    Value *value;
    std::vector<size_t> node_indexes;
public:
    LabelInfo(const std::string &name="<Anonymous>");
    void set_value(Value *value);
    void add_node(size_t idx);
    void propagate(ProgramNodes &program);
};

class LabelsTable {
private:
    std::vector<LabelInfo> labels;
public:
    void clear();
    LabelID new_label();
    void set_value(LabelID label, Value *value);
    void add_node(LabelID label, size_t idx);
    void propagate(ProgramNodes &program);
};

#endif // LABELS_H
