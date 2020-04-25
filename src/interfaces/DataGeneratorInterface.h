#pragma once

#include <memory>
#include <vector>

#include "Column.h"
#include "DataRecord.h"
#include "../Fort/fort.hpp"

/** @file
 * @brief Data Generator Interface
 */

/**
 * @brief Interface for relational data sources
 *
 * Designed as a generator exposing interfaces next() and hasNext().
 * All data sources should comply to this interface.
 *
 * @see GenericQueryBuilder
 * @see GenericDataGenerator
 */
class DataGeneratorInterface {
   protected:
   /** @brief metadata for processing queries*/
    Metadata metadata;

   public:
    /**
     * @return next record
     */
    virtual DataRecord next() = 0;

    /**
     * @return returns true if record can be generated
     */
    virtual bool hasNext() = 0;

    /**
     * @return Metadata
     *
     * @see DataRecordMetadata
     */
    Metadata getMetadata() const { return metadata; }

    /**
     * @brief Helper function to return batch of records.
     *
     * Intended to be used for batch processing.
     * @return batch of records
     */
    virtual std::vector<DataRecord> nextBatch(int records) {
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
    void print() {
        auto m = getMetadata();
        fort::char_table table;
        table << fort::header;

        auto columns = m->getColumns();

        for (int i = 0; i < columns.size(); i++) table << columns[i].name;

        table << fort::endr;

        while (hasNext()) {
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

    /**
     * @brief Destroy the Data Generator Interface object
     *
     */
    virtual ~DataGeneratorInterface(){};
};

/**
 * @brief Shared pointer to the DataGeneratorInterface
 *
 */
typedef std::shared_ptr<DataGeneratorInterface> DataSource;