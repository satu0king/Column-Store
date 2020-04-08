#pragma once

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Column.h"
#include "fort.hpp"

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

struct DataRecord {
    // std::shared_ptr<DataRecordMetadata> metadata;
    std::vector<DataValue> values;
    DataRecord() {}
    DataRecord(std::vector<DataValue>& values) : values(std::move(values)) {}

    DataValue& operator[](int index) { return values[index]; }
    //    public:
    //     DataRecord(std::vector<DataValue> values,
    //                std::shared_ptr<DataRecordMetadata> metadata)
    //         : values(values), metadata(metadata) {}
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

    Column& getColumn(std::string name) { return (*this)[name]; }

    Column& getColumn(int i) { return (*this)[i]; }

    Column& operator[](std::string name) {
        assert(columnMap.find(name) != columnMap.end());
        return columns[columnMap[name]];
    }

    void print(DataRecord& record) {
        fort::char_table table;
        table << fort::header;

        for (int i = 0; i < columns.size(); i++)
            table << columns[i].name;

        table << fort::endr;
        
        for (int i = 0; i < columns.size(); i++) {
            auto type = columns[i].type;
            if (type == DataType::INT)
                table << record[i].as<int>();
            else if (type == DataType::FLOAT)
                table << record[i].as<float>();
            else if (type == DataType::STRING)
                table << record[i].as<std::string>();
            else
                throw std::runtime_error("Unknown DataType");
        }
        table << fort::endr;

        std::cout << table.to_string() << std::endl;
    }

    void print() {
        for (int i = 0; i < columns.size(); i++) {
            auto type = columns[i].type;
            std::cout << columns[i].name << " - ";
            if (type == DataType::INT)
                std::cout << "INT";
            else if (type == DataType::FLOAT)
                std::cout << "FLOAT";
            else if (type == DataType::STRING)
                std::cout << "STRING";
            else
                throw std::runtime_error("Unknown DataType");

            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
};

typedef std::shared_ptr<DataRecordMetadata> Metadata;