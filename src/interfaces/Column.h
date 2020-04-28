#pragma once

#include <string>

#include "assert.h"

/** @file Column.h
 * @brief Column Metadata and Type information
 */

namespace ColumnStore {
/**
 * @brief Different datatypes supported by this project
 *
 */
enum class DataType { INT, FLOAT, STRING };

/**
 * @brief Struct which maintains metadata of a single column
 *
 */
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

    Column(std::string name, DataType type, int index)
        : name(name), type(type), index(index) {}
        
    Column() {}
};

// class ColumnNotFoundException : public exception {
//     std::string msg;
//     public:
//         ColumnNotFoundException(std::string &r_name, std::string &c_name) :
//         msg("Column " + c_name + " not found in " + r_name) {} virtual const
//         char* what() const throw() {
//             return msg.c_str();
//         }
// };

};  // namespace ColumnStore