#pragma once

#include <queue>
#include <vector>

#include "CSVparser.hpp"
#include "DataGeneratorInterface.h"

/** @file
 * @brief Data Generator for CSV files
 *
 */

/**
 * @brief CSV Data Source generator
 *
 * This implementation loads the whole CSV file into memory and exposes the data
 * generator interface. As this loads the entire file it is intended for testing
 * only and not for use with large CSV files.
 *
 * CSV File Format Expectation
 *
 * First row is the column Header which is comma seperated column names. Second
 * row is the Ttpe Header which is comma seperated data types
 *
 * CSV Parser Library used - https://github.com/rsylvian/CSVparser
 *
 * See example CSV file here -
 *
 * ~~~~~~~~~~~~~~~~~~~~~
 * Year,Make,Model
 * INT,STRING,STRING
 * 1997,Ford,E350
 * 2000,Mercury,Cougar
 * ~~~~~~~~~~~~~~~~~~~~~
 */

using ColumnStore::Column;
using ColumnStore::DataRecord;
using ColumnStore::DataRecordMetadata;
using ColumnStore::DataType;
using ColumnStore::DataValue;
using ColumnStore::Metadata;

namespace CSV {
class CSVDataSource : public ColumnStore::DataGeneratorInterface {
    /** @brief queue of data records for the generator */
    std::queue<DataRecord> data;

   public:
    /**
     * @brief Construct a new CSVDataSource object
     *
     * Constructor parses CSV data, generates all the data records and stores it
     * in a queue for data record generation
     * @param filename of the CSV file
     */
    CSVDataSource(std::string filename);

    /**
     * @brief Generates the next record
     *
     * @return DataRecord
     */
    DataRecord next();

    /**
     * @brief Checks if a record is available
     *
     * @return true
     * @return false
     */
    bool hasNext();
};

}  // namespace CSV
