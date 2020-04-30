
#pragma once
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

#include "GenericQueryBuilder.h"
#include "GroupByManager.h"
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

        JoinValueManager manager;
        std::unordered_map<JoinValue, DataRecord, JoinHash> joinMap;

        DataGeneraterJoin(std::string name, Metadata metadata)
            : manager(name, metadata) {}
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

class GenericDataAggregator : public ColumnStore::DataGeneratorInterface {
    bool _hasNext = false;

    DataRecord record;

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
    GenericDataAggregator(GenericQueryBuilder builder) {
        GenericDataGenerator generator(builder);

        auto generatorMetadata = generator.getMetadata();

        auto &aggregations = builder.aggregations;

        std::vector<ColumnStore::Column> columns;
        for (auto &aggregate : aggregations)
            columns.emplace_back(aggregate->getColumnName(),
                                 ColumnStore::DataType::FLOAT);

        metadata = Metadata(new DataRecordMetadata(columns));

        if (!generator.hasNext()) return;

        for (auto &aggregate : aggregations)
            aggregate->initialize(generatorMetadata);

        while (generator.hasNext()) {
            auto record = generator.next();
            for (auto &aggregate : aggregations) aggregate->addRecord(record);
        }

        std::vector<ColumnStore::DataValue> values;

        for (auto &aggregate : aggregations)
            values.emplace_back(aggregate->getValue());

        record = ColumnStore::DataRecord(values);
        _hasNext = true;
    }
    /**
     * @brief check if a record is available
     *
     * @return true
     * @return false
     */
    bool hasNext() { return _hasNext; }

    /**
     * @brief generates a new record
     *
     * @return DataRecord
     */
    DataRecord next() {
        _hasNext = false;
        return record;
    }
};

class GenericGroupByAggregator : public ColumnStore::DataGeneratorInterface {
    std::queue<ColumnStore::DataRecord> records;

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
    GenericGroupByAggregator(GenericQueryBuilder builder) {
        GenericDataGenerator generator(builder);

        auto generatorMetadata = generator.getMetadata();

        GroupByManager manager(builder.groupBys, generatorMetadata);

        std::vector<std::string> &groupBys = builder.groupBys;
        auto &aggregations = builder.aggregations;

        std::vector<ColumnStore::Column> columns;

        for (auto &grp : groupBys)
            columns.push_back(generatorMetadata->getColumn(grp));

        for (auto &aggregate : aggregations)
            columns.emplace_back(aggregate->getColumnName(),
                                 ColumnStore::DataType::FLOAT);

        metadata = Metadata(new DataRecordMetadata(columns));

        std::unordered_map<GroupByValue,
                           std::vector<ColumnStore::AggregatorQuery>, GroupHash>
            map;

        for (auto &aggregate : aggregations)
            aggregate->initialize(generatorMetadata);

        auto duplicateAggregators = [&]() {
            std::vector<ColumnStore::AggregatorQuery> temp;
            for (auto &agg : aggregations) temp.push_back(agg->clone());
            return temp;
        };

        while (generator.hasNext()) {
            auto record = generator.next();
            auto groupValue = manager.processRecord(record);
            if (!map.count(groupValue))
                map[groupValue] = duplicateAggregators();

            auto &aggregations1 = map[groupValue];

            for (auto &aggregate : aggregations1) aggregate->addRecord(record);
        }

        for (auto &[groupValue, aggregations1] : map) {
            std::vector<ColumnStore::DataValue> values = groupValue.values;
            for (auto &aggregate : aggregations1)
                values.emplace_back(aggregate->getValue());
            records.emplace(values);
        }
    }
    /**
     * @brief check if a record is available
     *
     * @return true
     * @return false
     */
    bool hasNext() { return !records.empty(); }

    /**
     * @brief generates a new record
     *
     * @return DataRecord
     */
    DataRecord next() {
        auto temp = records.front();
        records.pop();
        return temp;
    }
};

}  // namespace GenericQuery
