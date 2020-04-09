#pragma once

#include <string>

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
    std::string name;
    DataType type;

    /**
     * @brief Index of the column in the data record - Needed for fast retrieval
     */
    int index;
};
