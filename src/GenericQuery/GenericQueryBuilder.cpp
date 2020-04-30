
#include "GenericQueryBuilder.h"

#include "GenericGenerator.h"

/**
 * @brief Generic Query Builder Class
 *
 * Generic Query builder can be used to query any data source. The query
 * mechanisim is performed in a general manner and will not take any advantage
 * of the underlying storage mechanisim, i.e. indexes, encoding, ordering etc
 * will not be used for optimization. Advantage of this builder is that it can
 * work with any data source.
 *
 * Every builder class will also have a corresponding data generator. The
 * generator for this builder is GenericDataGenerator
 *
 * @see GenericDataGenerator
 */

namespace GenericQuery {
/**
 * @brief Registers a new data source and gives is an alias name
 *
 * @param name
 * @param source
 */
void GenericQueryBuilder::registerDataSource(std::string name,
                                             DataSource source) {
    data_sources[name] = source;
}

/**
 * @brief Set the Base Source and the columns required in the query result
 *
 * @param name of data source
 * @param columns required in result
 */
void GenericQueryBuilder::setBaseSource(std::string name,
                                        std::vector<std::string> columns) {
    baseSource = name;
    baseColumns = columns;

    for (std::string column : columns) {
        // Ensures that there are no duplicate columns
        assert(columnMap.find(column) == columnMap.end());
        columnMap[column] = name;
    }
}

/**
 * @brief Creates a join from base source to a join source
 *
 * @param name of join source
 * @param columns of join source required in result
 * @param foreignKey from base source
 * @param primaryKey of join source
 */
void GenericQueryBuilder::join(std::string name,
                               std::vector<std::string> columns,
                               std::string foreignKey, std::string primaryKey) {
    Join join = {
        .foreignKey = foreignKey, .primaryKey = primaryKey, .columns = columns};

    joins.emplace_back(name, join);

    for (std::string column : columns) {
        // Ensures that there are no duplicate columns
        assert(columnMap.find(column) == columnMap.end());
        columnMap[column] = name;
    }
}

/**
 * @brief Sets the query object
 *
 * @param query
 */
void GenericQueryBuilder::where(Query query) {
    assert(query->getValidator(generateMetadata()));
    this->query = query;
}

/**
 * @brief Generates metadata of result records using join and query
 * information
 *
 * Columns are ordered as follows - Base Source Columns, followed by
 * individual join columns. This ordering is important as it allows the
 * generator to implicitely generate DataRecord without explicit ordering of
 * DataValue
 *
 * @return Metadata of builder
 */
Metadata GenericQueryBuilder::generateMetadata() {
    std::vector<Column> columns;

    // Extracts column information from data source

    // column index is set here in a first come manner - implicit ordering
    // happens here

    auto helper = [&](std::string source, std::vector<std::string> _columns) {
        for (auto const& name : _columns) {
            // Extract type information
            DataType type =
                data_sources[source]->getMetadata()->getColumn(name).type;

            // Construct column information
            Column c(name, type, columns.size());

            columns.push_back(c);
        }
    };

    // Extract columns from base source followed by join sources
    helper(baseSource, baseColumns);
    for (const auto& [source, join] : joins) {
        helper(source, join.columns);
    }

    return Metadata(new DataRecordMetadata(columns));
}

/**
 * @brief creates record
 *
 * @return RecordValidator
 */
RecordValidator GenericQueryBuilder::generateRecordValidator() {
    return query->getValidator(generateMetadata());
}

DataSource GenericQueryBuilder::build() {
    if (groupBys.size() && aggregations.empty())
        throw std::runtime_error("Group By doesnt have aggregations");

    if (groupBys.size()) return DataSource(new GenericGroupByAggregator(*this));

    if (aggregations.size())
        return DataSource(new GenericDataAggregator(*this));

    return DataSource(new GenericDataGenerator(*this));
}
};  // namespace GenericQuery