#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "ConditionsInterfaces/ConditionInterface.h"
#include "DataGeneratorInterface.h"

/** @file
 * @brief Generic Query Builder
 *
 * This file contains structures to query any data source. Currently supported
 * query features are join and filter
 */

/**
 * @brief Base Condition Query Structure
 *
 * ConditionQuery structures are used to create the query. ConditionInterface
 * structures are used to validate the records
 *
 * see @ConditionInterface
 */
class ConditionQuery {
   public:
    /**
     * @brief Get the Checker object which complies to ConditionInterface
     *
     * This is required because the record validators need metadata information
     * to perform validation efficiently. Therefore record validators needs to
     * be created once the query is built.
     *
     * @param metadata
     * @return ConditionChecker
     *
     * see @ConditionInterface
     */
    virtual ConditionChecker getChecker(Metadata metadata) {
        return ConditionChecker(new ConditionInterface());
    };

    /**
     * @brief Destroy the Condition Query object
     *
     */
    virtual ~ConditionQuery() {}
};

/**
 * @brief Shared pointer to Query Structure
 *
 */
typedef std::shared_ptr<ConditionQuery> Query;

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
        std::string foreignKey;
        std::string primaryKey;
        /**
         * @brief List of columns required in result from joined table
         *
         */
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
    void registerDataSource(std::string name, DataSource source) {
        data_sources[name] = source;
    }

    /**
     * @brief Set the Base Source and the columns required in the query result
     *
     * @param name of data source
     * @param columns required in result
     */
    void setBaseSource(std::string name, std::vector<std::string> columns) {
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
    void join(std::string name, std::vector<std::string> columns,
              std::string foreignKey, std::string primaryKey) {
        Join join = {.foreignKey = foreignKey,
                     .primaryKey = primaryKey,
                     .columns = columns};

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
    void where(Query query) {
        assert(query->getChecker(generateMetadata()));
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
    Metadata generateMetadata() {
        std::vector<Column> columns;

        /**
         * @brief Extracts column information from data source
         *
         * column index is set here in a first come manner - implicit ordering
         * happens here
         */
        auto helper = [&](std::string source,
                          std::vector<std::string> _columns) {
            for (auto const& name : _columns) {
                /** Extract type information*/
                DataType type =
                    data_sources[source]->getMetadata()->getColumn(name).type;

                /** Construct column information*/
                Column c = {.name = name,
                            .type = type,
                            .index = static_cast<int>(columns.size())};

                columns.push_back(c);
            }
        };

        /**
         * Extract columns from base source followed by join sources
         */
        helper(baseSource, baseColumns);
        for (const auto& [source, join] : joins) helper(source, join.columns);

        return Metadata(new DataRecordMetadata(columns));
    }

    /**
     * @brief creates record
     * 
     * @return ConditionChecker 
     */
    ConditionChecker generateConditionChecker() {
        return query->getChecker(generateMetadata());
    }
};

class GenericDataGenerator : public DataGeneratorInterface {
    GenericQueryBuilder builder;
    ConditionChecker checker;
    DataSource baseSource;
    std::vector<int> baseIndices;

    struct DataGeneraterJoin {
        std::vector<int> indices;
        int FKIndex;
        std::unordered_map<int, DataRecord> map;
    };

    bool _hasNext = false;
    DataRecord nextRecord;

    std::vector<DataGeneraterJoin> joins;

    DataRecord _getNext() {
        DataRecord candidate = _getCandidateNext();

        while (!checker->validate(candidate)) {
            candidate = _getCandidateNext();
        }
        return candidate;
    }

    void _setNext() {
        _hasNext = false;

        try {
            nextRecord = _getNext();
            _hasNext = true;
        } catch (const std::exception&) {
            _hasNext = false;
        }
    }

    DataRecord _getCandidateNext() {
        std::vector<DataValue> values;

        if (!baseSource->hasNext()) {
            throw std::runtime_error("No more Records Left");
        }

        DataRecord baseRecord = baseSource->next();

        for (int i : baseIndices) {
            values.push_back(baseRecord[i]);
        }

        for (DataGeneraterJoin& join : joins) {
            DataRecord& record = join.map[baseRecord[join.FKIndex].as<int>()];
            for (int index : join.indices) values.push_back(record[index]);
        }

        return DataRecord(values);
    }

   public:
    GenericDataGenerator(GenericQueryBuilder builder) : builder(builder) {
        metadata = builder.generateMetadata();
        checker = builder.generateConditionChecker();

        baseSource = builder.data_sources[builder.baseSource];
        Metadata baseSource_metadatda = baseSource->getMetadata();

        for (std::string name : builder.baseColumns) {
            baseIndices.push_back(baseSource_metadatda->getColumn(name).index);
        }

        // Generating Join Metadata and data
        for (auto const& [source, _join] : builder.joins) {
            std::vector<int> indices;
            DataSource joinSource = builder.data_sources[source];
            Metadata join_metadata = joinSource->getMetadata();

            DataGeneraterJoin join;

            // Extracting Metadata
            join.FKIndex =
                baseSource_metadatda->getColumn(_join.foreignKey).index;

            int PKIndex = join_metadata->getColumn(_join.primaryKey).index;

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
                join.map[record[PKIndex].as<int>()] = record;
            }

            joins.push_back(join);
        }

        _setNext();
    }

    bool hasNext() { return _hasNext; }

    DataRecord next() {
        DataRecord result = nextRecord;
        _setNext();
        return result;
    }
};

class AndQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    AndQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    ConditionChecker getChecker(Metadata m) {
        return ConditionChecker(
            new AndCondition(query1->getChecker(m), query2->getChecker(m)));
    }
};

class OrQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    OrQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    ConditionChecker getChecker(Metadata m) {
        return ConditionChecker(
            new OrCondition(query1->getChecker(m), query2->getChecker(m)));
    }
};

class NotQuery : public ConditionQuery {
    Query query1;

   public:
    NotQuery(Query query1) : query1(query1) {}
    ConditionChecker getChecker(Metadata m) {
        return ConditionChecker(new NotCondition(query1->getChecker(m)));
    }
};

class EqualQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
    EqualQuery(std::string name, std::any value) : name(name), value(value) {}

    ConditionChecker getChecker(Metadata m) {
        return ConditionChecker(new EqualCondition(name, value, m));
    }
};

class LessThanQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
    LessThanQuery(std::string name, std::any value)
        : name(name), value(value) {}

    ConditionChecker getChecker(Metadata m) {
        return ConditionChecker(new LessThanCondition(name, value, m));
    }
};
