#pragma once

#include "interfaces/Column.h"
#include <iostream>

namespace Parser {
struct DataType {
    ColumnStore::DataType dataType;
    int size;
    DataType(ColumnStore::DataType dataType, int size = 0)
        : dataType(dataType), size(size) {
        if (dataType == ColumnStore::DataType::INT ||
            dataType == ColumnStore::DataType::FLOAT)
            this->size = 4;
    }

    DataType(const char *type) : DataType(std::string(type)) {}

    DataType(std::string type) {
        if (type == "int") {
            dataType = ColumnStore::DataType::INT;
            size = 4;
        } else if (type == "float") {
            dataType = ColumnStore::DataType::FLOAT;
            size = 4;
        } else if (type.substr(0, 6) == "string") {
            dataType = ColumnStore::DataType::STRING;
            size = stoi(type.substr(6));
        }

        // std::cout << "SIZE " << type << " " << size << std::endl;
    }

    operator int() { return size; };
    operator ColumnStore::DataType() { return dataType; }
};

struct Column {
    /** @brief name of the column */
    std::string name;

    /** @brief data type of the column */
    DataType type;

    /**
     * @brief Index of the column in the data record.
     *
     * Index is used to retrieve column value from record
     */
    int index;

    Column(std::string name, DataType type, int index = -1)
        : name(name), type(type), index(index) {}

    ColumnStore::Column getColumnStoreColumn() {
        return ColumnStore::Column{name, type.dataType, index};
    }

    operator ColumnStore::Column() { return getColumnStoreColumn(); }

    operator ColumnStore::DataType() { return type; }
    operator Parser::DataType() { return type; }
    operator int() { return type; }
};
}  // namespace Parser