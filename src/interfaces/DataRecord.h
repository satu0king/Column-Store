#pragma once

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Column.h"
#include "../Fort/fort.hpp"

/** @file DataRecord.h
 * @brief Data storage structures
 */
namespace ColumnStore{
/**
 * @brief Implementation of a single data element
 *
 * The object can hold any data type. The actual data can be extracted by using
 * as<T>() function where T is the type. Type information itself is not stored
 * in the DataValue object and has to be inferred from DataRecordMetadata
 * objects.
 *
 * @see DataRecord
 * @see DataRecordMetadata
 */
class DataValue {
    std::any value;

   public:
    DataValue(std::any value) : value(value) {}

    /**
     * @brief Extracts value by type
     * @tparam T the type of data stored in the value
     * @return actual datavalue after casting
     */
    template <typename T>
    T as() {
        return std::any_cast<T>(value);
    }

    std::any get() { return value; }
};

/**
 * @brief Stores a row of data
 *
 * Internally stores a vector of DataValue objects. Individual data value(s) can
 * be extracted using array like indexing. The type information is not stored in
 * the DataRecord object and has to be extracted from DataRecordMetadata object
 *
 * @see DataRecordMetadata
 * @see DataValue
 */

struct DataRecord {
    std::vector<DataValue> values;
    DataRecord() {}
    DataRecord(std::vector<DataValue>& values) : values(std::move(values)) {}

    /**
     * @brief Extracts data value by index
     * @param index column index of the data value
     * @return DataValue at index
     */
    DataValue& operator[](int index) { return values[index]; }
};

/**
 * @brief Stores metadata information of the data record.
 *
 * Metadata information include names of columns, the types of the columns and
 * also the column index of each column. Each DataGenerator source will provide
 * this metadata. Actual values from individual data records can be extracted
 * using this metadata
 *
 * @see DataRecord
 * @see DataGeneratorInterface
 */
class DataRecordMetadata {
    std::vector<Column> columns;
    /**
     * @brief Map from column name to column index
     *
     */
    std::unordered_map<std::string, int> columnMap;

   public:
    DataRecordMetadata(std::vector<Column> columns) : columns(columns) {
        for (int i = 0; i < columns.size(); i++) columnMap[columns[i].name] = i;
    }
    /**
     * @brief Get the Columns object
     *
     * @return column list
     */
    std::vector<Column>& getColumns() { return columns; }

    /**
     * @brief Access Column by index
     *
     * @param i index
     * @return Column
     */
    Column& operator[](int i) {
        assert(i < columns.size());
        return columns[i];
    }

    /**
     * @brief Get the Column object by name
     *
     * @param name
     * @return Column
     */
    Column& getColumn(std::string name) { return (*this)[name]; }

    /**
     * @brief Get the Column object by index
     *
     * @param i
     * @return Column
     */
    Column& getColumn(int i) { return (*this)[i]; }

    /**
     * @brief Access column object by index
     *
     * @param name
     * @return Column
     */
    Column& operator[](std::string name) {
        assert(columnMap.find(name) != columnMap.end());
        return columns[columnMap[name]];
    }

    /**
     * @brief Pretty print a single data record
     *
     * Prints the column names and the record data in tabular form
     *
     * @param record to be printed
     */
    void print(DataRecord& record) {
        fort::char_table table;
        table << fort::header;

        for (int i = 0; i < columns.size(); i++) table << columns[i].name;

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

    /**
     * @brief Print metadata 
     * 
     * Prints column names and type
     */
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

/**
 * @brief Shared pointer to DataRecordMetadata
 * 
 * Metadata is transfered between objects using this type
 */
typedef std::shared_ptr<DataRecordMetadata> Metadata;
};