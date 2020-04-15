#pragma once

#include <string>

#include "assert.h"


/** @file Column.h
 * @brief Column Metadata and Type information
 */

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
};
