#include "GroupByManager.h"

namespace GenericQuery {

bool JoinValueManager::compare(const JoinValue &value1,
                               const JoinValue &value2) const {
    if (type == ColumnStore::DataType::INT) {
        if (value1.value.as<int>() != value2.value.as<int>()) return false;
    } else if (type == ColumnStore::DataType::FLOAT) {
        if (value1.value.as<float>() != value2.value.as<float>()) return false;
    } else if (type == ColumnStore::DataType::STRING) {
        if (value1.value.as<std::string>() != value2.value.as<std::string>())
            return false;
    }

    return true;
}

size_t JoinValueManager::hash(const JoinValue &value) const {
    size_t hash = 0;
    if (type == ColumnStore::DataType::INT) {
        hash ^= std::hash<int>()(value.value.as<int>());
    } else if (type == ColumnStore::DataType::FLOAT) {
        hash ^= std::hash<float>()(value.value.as<float>());
    } else if (type == ColumnStore::DataType::STRING) {
        hash ^= std::hash<std::string>()(value.value.as<std::string>());
    }

    return hash;
}

JoinValue JoinValueManager::processValue(DataValue &value) {
    return JoinValue(value, this);
}

bool GroupByManager::compare(const GroupByValue &value1,
                             const GroupByValue &value2) const {
    for (int i = 0; i < types.size(); i++) {
        if (types[i] == ColumnStore::DataType::INT) {
            if (value1[i].as<int>() != value2[i].as<int>()) return false;
        } else if (types[i] == ColumnStore::DataType::FLOAT) {
            if (value1[i].as<float>() != value2[i].as<float>()) return false;
        } else if (types[i] == ColumnStore::DataType::STRING) {
            if (value1[i].as<std::string>() != value2[i].as<std::string>())
                return false;
        }
    }

    return true;
}

size_t GroupByManager::hash(const GroupByValue &value) const {
    size_t hash = 0;
    for (int i = 0; i < types.size(); i++) {
        if (types[i] == ColumnStore::DataType::INT) {
            hash ^= std::hash<int>()(value[i].as<int>());
        } else if (types[i] == ColumnStore::DataType::FLOAT) {
            hash ^= std::hash<float>()(value[i].as<float>());
        } else if (types[i] == ColumnStore::DataType::STRING) {
            hash ^= std::hash<std::string>()(value[i].as<std::string>());
        }
    }

    return hash;
}

GroupByValue GroupByManager::processRecord(DataRecord &record) {
    GroupByValue value;
    value.values.reserve(indices.size());
    value.manager = this;

    for (int index : indices) value.values.push_back(record[index]);

    return value;
}

}  // namespace GenericQuery