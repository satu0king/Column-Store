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
    Metadata getMetadata() const;

    /**
     * @brief Helper function to return batch of records.
     *
     * Intended to be used for batch processing.
     * @return batch of records
     */
    virtual std::vector<DataRecord> nextBatch(int batchSize);

    /**
     * @brief Print the data
     *
     * Pretty print the relational data in a tabular form with column names
     */
    void print();

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