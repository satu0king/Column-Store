
#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "GenericQueryBuilder.h"
#include "interfaces/ConditionQuery.h"
#include "interfaces/DataGeneratorInterface.h"
#include "interfaces/Validators.h"

namespace GenericQuery {
/**
 * @brief Data Generator for GenericQueryBuilder
 *
 * @see GenericQueryBuilder
 *
 */

using ColumnStore::DataRecord;
using ColumnStore::DataSource;
using ColumnStore::DataType;
using ColumnStore::DataValue;
using ColumnStore::RecordValidator;

class GenericDataGenerator : public ColumnStore::DataGeneratorInterface {
    /** @brief  builder object from which we are generating data */
    GenericQueryBuilder builder;
    RecordValidator recordValidator;
    DataSource baseSource;

    /** @brief  List of column indices to extract values from base source
     * records */
    std::vector<int> baseIndices;

    /**
     * @brief helper structure to efficiently process joins
     *
     */
    struct DataGeneraterJoin {
        /** @brief indices of columns required for joined result */
        std::vector<int> indices;
        /** @brief FK index in base data source */
        int FKIndex;
        /**
         * @brief map of PK of join source to join records
         *
         * The data generator loads all the joined table data into memory and
         * processes the PK-FK join in memory. This is not an issue as long as
         * the number of joined table records is small.
         */
        std::unordered_map<int, DataRecord> joinMap;
    };

    bool _hasNext = false;

    DataRecord nextRecord;

    /** @brief List of joins */
    std::vector<DataGeneraterJoin> joins;

    /**
     * @brief internal function to process and generate next valid record
     *
     * @return DataRecord
     */
    DataRecord _getNext();

    /**
     * @brief internal function to set the next valid record
     *
     * If no valid record is found, it sets _hasNext to false
     */
    void _setNext();

    /**
     * @brief Gets the next candidate record
     *
     * Gets the next record from base source. Applies join on the join map data.
     * This function does not perform record validation.
     *
     * @return DataRecord
     */
    DataRecord _getCandidateNext();

   public:
    /**
     * @brief Construct a new Generic Data Generator object
     *
     * The generator processes all column data and maps it to column indices for
     * efficiency. All joined data sources are loaded into memory in the
     * constructor
     *
     * @param builder object to constuct generator from
     */
    GenericDataGenerator(GenericQueryBuilder builder);
    /**
     * @brief check if a record is available
     *
     * @return true
     * @return false
     */
    bool hasNext();

    /**
     * @brief generates a new record
     *
     * @return DataRecord
     */
    DataRecord next();
};
}  // namespace GenericQuery