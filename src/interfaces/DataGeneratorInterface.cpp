#include "DataGeneratorInterface.h"

/** @file
 * @brief Data Generator Interface
 */

namespace ColumnStore {
/**
 * @brief Interface for relational data sources
 *
 * Designed as a generator exposing interfaces next() and hasNext().
 * All data sources should comply to this interface.
 *
 * @see GenericQueryBuilder
 * @see GenericDataGenerator
 */

/**
 * @return Metadata
 *
 * @see DataRecordMetadata
 */
Metadata DataGeneratorInterface::getMetadata() const { return metadata; }

/**
 * @brief Helper function to return batch of records.
 *
 * Intended to be used for batch processing.
 * @return batch of records
 */
std::vector<DataRecord> DataGeneratorInterface::nextBatch(int records) {
    assert(records >= 0);

    std::vector<DataRecord> results;
    for (int i = 0; i < records && hasNext(); ++i) {
        results.push_back(next());
    }
    return results;
}

/**
 * @brief Print the data
 *
 * Pretty print the relational data in a tabular form with column names
 */
void DataGeneratorInterface::print(int recordCount) {
    auto m = getMetadata();
    fort::char_table table;
    table << fort::header;

    auto columns = m->getColumns();

    for (int i = 0; i < columns.size(); i++) table << columns[i].name;

    table << fort::endr;

    while (hasNext() && recordCount--) {
        auto record = next();
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
    }

    std::cout << table.to_string() << std::endl;
}
};  // namespace ColumnStore