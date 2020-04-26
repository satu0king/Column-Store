
#include "GenericGenerator.h"

/**
 * @brief Data Generator for GenericQueryBuilder
 *
 * @see GenericQueryBuilder
 *
 */

namespace GenericQuery {
/**
 * @brief internal function to process and generate next valid record
 *
 * @return DataRecord
 */
DataRecord GenericDataGenerator::_getNext() {
    DataRecord candidate = _getCandidateNext();

    while (!recordValidator->validate(candidate)) {
        candidate = _getCandidateNext();
    }
    return candidate;
}

/**
 * @brief internal function to set the next valid record
 *
 * If no valid record is found, it sets _hasNext to false
 */
void GenericDataGenerator::_setNext() {
    _hasNext = false;

    try {
        nextRecord = _getNext();
        _hasNext = true;
    } catch (const std::exception&) {
        _hasNext = false;
    }
}

/**
 * @brief Gets the next candidate record
 *
 * Gets the next record from base source. Applies join on the join map data.
 * This function does not perform record validation.
 *
 * @return DataRecord
 */
DataRecord GenericDataGenerator::_getCandidateNext() {
    std::vector<DataValue> values;

    if (!baseSource->hasNext()) {
        throw std::runtime_error("No more Records Left");
    }

    DataRecord baseRecord = baseSource->next();

    // Extract base column values
    for (int i : baseIndices) {
        values.push_back(baseRecord[i]);
    }

    // Find join record and extract join column values
    for (DataGeneraterJoin& join : joins) {
        DataRecord& record = join.joinMap[baseRecord[join.FKIndex].as<int>()];
        for (int index : join.indices) values.push_back(record[index]);
    }

    return DataRecord(values);
}

/**
 * @brief Construct a new Generic Data Generator object
 *
 * The generator processes all column data and maps it to column indices for
 * efficiency. All joined data sources are loaded into memory in the
 * constructor
 *
 * @param builder object to constuct generator from
 */
GenericDataGenerator::GenericDataGenerator(GenericQueryBuilder builder)
    : builder(builder) {
    metadata = builder.generateMetadata();
    recordValidator = builder.generateRecordValidator();

    baseSource = builder.data_sources[builder.baseSource];
    Metadata baseSource_metadatda = baseSource->getMetadata();

    // Extract base source column indices
    for (std::string name : builder.baseColumns) {
        baseIndices.push_back(baseSource_metadatda->getColumn(name).index);
    }

    // Generating Join Metadata and data
    for (auto const& [source, _join] : builder.joins) {
        std::vector<int> indices;
        DataSource joinSource = builder.data_sources[source];
        Metadata join_metadata = joinSource->getMetadata();

        DataGeneraterJoin join;

        // Extracting FK index
        join.FKIndex = baseSource_metadatda->getColumn(_join.foreignKey).index;

        int PKIndex = join_metadata->getColumn(_join.primaryKey).index;

        // Extracting join source column indices
        for (auto column : _join.columns)
            indices.push_back(join_metadata->getColumn(column).index);

        join.indices = indices;

        // Join has to be done over INT only
        assert(join_metadata->getColumn(_join.primaryKey).type ==
               DataType::INT);
        assert(baseSource_metadatda->getColumn(_join.foreignKey).type ==
               DataType::INT);

        // Building Join Data Map (this will be in memory)
        while (joinSource->hasNext()) {
            DataRecord record = joinSource->next();
            join.joinMap[record[PKIndex].as<int>()] = record;
        }

        joins.push_back(join);
    }

    // Sets a new record and in turn sets _hasNext value
    _setNext();
}

/**
 * @brief check if a record is available
 *
 * @return true
 * @return false
 */
bool GenericDataGenerator::hasNext() { return _hasNext; }

/**
 * @brief generates a new record
 *
 * @return DataRecord
 */
DataRecord GenericDataGenerator::next() {
    DataRecord result = nextRecord;
    _setNext();
    return result;
}  //< Base
}  // namespace GenericQuery