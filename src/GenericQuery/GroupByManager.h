#pragma once
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "GenericQueryBuilder.h"
#include "interfaces/ConditionQuery.h"
#include "interfaces/DataGeneratorInterface.h"
#include "interfaces/Validators.h"

namespace GenericQuery {

struct GroupByValue;

struct GroupByManager {
    std::vector<int> indices;
    std::vector<DataType> types;
    GroupByManager(std::vector<std::string> &columns, Metadata metadata) {
        indices.reserve(columns.size());
        types.reserve(columns.size());
        for (auto &c : columns) {
            indices.push_back(metadata->getColumn(c).index);
            types.push_back(metadata->getColumn(c));
        }
    }

    bool compare(const GroupByValue &value1, const GroupByValue &value2) const;
    size_t hash(const GroupByValue &value) const;
    GroupByValue processRecord(DataRecord &record);
};

struct GroupByValue {
    std::vector<ColumnStore::DataValue> values;
    GroupByManager *manager;
    bool operator==(const GroupByValue &value) const {
        return manager->compare(*this, value);
    }

    const DataValue &operator[](int index) const { return values[index]; }
};

struct GroupHash {
   public:
    // Use sum of lengths of first and last names
    // as hash function.
    size_t operator()(const GroupByValue &value) const {
        return value.manager->hash(value);
    }
};
}