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

struct JoinValue;

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

struct JoinValueManager {
    DataType type;
    JoinValueManager(std::string name, Metadata metadata) {
        type = metadata->getColumn(name);
    }

    bool compare(const JoinValue &value1, const JoinValue &value2) const;
    size_t hash(const JoinValue &value) const;
    JoinValue processValue(DataValue &value);
};

struct GroupByValue {
    std::vector<ColumnStore::DataValue> values;
    GroupByManager *manager;
    bool operator==(const GroupByValue &value) const {
        return manager->compare(*this, value);
    }

    const DataValue &operator[](int index) const { return values[index]; }
};

struct JoinValue {
    ColumnStore::DataValue value;
    JoinValueManager *manager;
    JoinValue(ColumnStore::DataValue value, JoinValueManager *manager)
        : value(value), manager(manager) {}
    bool operator==(const JoinValue &value) const {
        return manager->compare(*this, value);
    }

    operator const ColumnStore::DataValue() const { return value; }
};

struct GroupHash {
   public:
    size_t operator()(const GroupByValue &value) const {
        return value.manager->hash(value);
    }
};

struct JoinHash {
    size_t operator()(const JoinValue &value) const {
        return value.manager->hash(value);
    }
};
}  // namespace GenericQuery