#pragma once;

#include <string>
#include <unordered_map>
#include <any>

#include "Column.h"

class DataValue {
    std::any value;

   public:
    DataValue(std::any value) : value(value) {}

    template <typename T>
    T as() {
        return std::any_cast<T>(value);
    }
};

class DataRecordMetadata {
    std::vector<Column> columns;
    std::unordered_map<std::string, int> columnMap;

   public:
    DataRecordMetadata(std::vector<Column> columns) : columns(columns) {
        for (int i = 0; i < columns.size(); i++) columnMap[columns[i].name] = i;
    }

    std::vector<Column>& getColumns() { return columns; }

    Column& operator[](int i) {
        assert(i < columns.size());
        return columns[i];
    }

    Column& operator[](std::string name) {
        assert(columnMap.find(name) != columnMap.end());
        return columns[columnMap[name]];
    }
};

class DataRecord {
    std::vector<DataValue> values;
};
