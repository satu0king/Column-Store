#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "ConditionQuery.h"
#include "DataGeneratorInterface.h"
#include "Validators.h"

/** @file
 * @brief Generic Query Builder
 *
 * This file contains structures to query any data source. Currently supported
 * query features are join and filter
 */

class GenericDataGenerator;

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
class GenericQueryBuilder {
    /**
     * @brief Helper struct to represent join infromation
     *
     */

    struct Join {
        /** @brief Foreign key from base source */
        std::string foreignKey;
        /** @brief Primary key in join source */
        std::string primaryKey;
        /** @brief List of columns required in result from joined source */
        std::vector<std::string> columns;
    };

    /**
     * @brief map of data source name and data generator
     *
     */
    std::unordered_map<std::string, DataSource> data_sources;

    /**
     * @brief Base data source
     *
     * All joins are done on FKs of the base table to PKs of joined table
     */
    std::string baseSource;

    /**
     * @brief list of columns from the base table required in result
     *
     */
    std::vector<std::string> baseColumns;

    /**
     * @brief List of joins
     *
     */
    std::vector<std::pair<std::string, Join>> joins;

    /**
     * @brief List of columns and their corresponding data source
     *
     */
    std::unordered_map<std::string, std::string> columnMap;

    /**
     * @brief Query object
     *
     */
    Query query = Query(new ConditionQuery);

    friend class GenericDataGenerator;

   public:
    /**
     * @brief Registers a new data source and gives is an alias name
     *
     * @param name
     * @param source
     */
    void registerDataSource(std::string name, DataSource source);

    /**
     * @brief Set the Base Source and the columns required in the query result
     *
     * @param name of data source
     * @param columns required in result
     */
    void setBaseSource(std::string name, std::vector<std::string> columns);

    /**
     * @brief Creates a join from base source to a join source
     *
     * @param name of join source
     * @param columns of join source required in result
     * @param foreignKey from base source
     * @param primaryKey of join source
     */
    void join(std::string name, std::vector<std::string> columns,
              std::string foreignKey, std::string primaryKey);

    /**
     * @brief Sets the query object
     *
     * @param query
     */
    void where(Query query);

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
    Metadata generateMetadata();

    /**
     * @brief creates record
     *
     * @return RecordValidator
     */
    RecordValidator generateRecordValidator();
};
