#pragma once;

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Column.h"

class DataValue {
    std::any value;

   public:
    DataValue(std::any value) : value(value) {}

    template <typename T>
    T as() {
        return std::any_cast<T>(value);
    }

    std::any get() { return value; }
};

class DataRecordMetadata {
    std::vector<Column> columns;
    std::unordered_map<std::string, int> columnMap;

   public:
    DataRecordMetadata(std::vector<Column> columns) : columns(columns) {
        for (int i = 0; i < columns.size(); i++) 
            columnMap[columns[i].name] = i;
    }

    std::vector<Column>& getColumns() { return columns; }

    Column& operator[](int i) {
        assert(i < columns.size());
        return columns[i];
    }

    Column& getColumn(std::string name) {
        return (*this)[name];
    }

    Column& getColumn(int i) {
        return (*this)[i];
    }
    
    Column& operator[](std::string name) {
        assert(columnMap.find(name) != columnMap.end());
        return columns[columnMap[name]];
    }
};

typedef std::shared_ptr<DataRecordMetadata> Metadata;

struct DataRecord {
    // std::shared_ptr<DataRecordMetadata> metadata;
    std::vector<DataValue> values;

    DataRecord(std::vector<DataValue> &values) : values(std::move(values)) {}

    DataValue& operator[](int index) {return values[index];}
    //    public:
    //     DataRecord(std::vector<DataValue> values,
    //                std::shared_ptr<DataRecordMetadata> metadata)
    //         : values(values), metadata(metadata) {}
};
