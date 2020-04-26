#pragma once

#include "interfaces/Column.h"

namespace Parser {
struct DataType {
    ColumnStore::DataType dataType;
    int size;
    DataType(ColumnStore::DataType dataType, int size = 0)
        : dataType(dataType), size(size) {}
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

    ColumnStore::Column getColumnStoreColumn() {
        return ColumnStore::Column{name, type.dataType, index};
    }
};
}  // namespace Parser