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
     * @brief Get the Valdiator object which complies to ConditionInterface
     *
     * This is required because the record validators need metadata information
     * to perform validation efficiently. Therefore record validators needs to
     * be created once the query is built.
     *
     * @param metadata
     * @return RecordValidator
     *
     * see @ConditionInterface
     */
    virtual RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(new RecordValidatorInterface());
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
    Metadata generateMetadata() {
        std::vector<Column> columns;

        // Extracts column information from data source

        // column index is set here in a first come manner - implicit ordering
        // happens here

        auto helper = [&](std::string source,
                          std::vector<std::string> _columns) {
            for (auto const& name : _columns) {
                // Extract type information
                DataType type =
                    data_sources[source]->getMetadata()->getColumn(name).type;

                // Construct column information
                Column c = {.name = name,
                            .type = type,
                            .index = static_cast<int>(columns.size())};

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
    RecordValidator generateRecordValidator() {
        return query->getValidator(generateMetadata());
    }
};

/**
 * @brief Data Generator for GenericQueryBuilder
 *
 * @see GenericQueryBuilder
 *
 */
class GenericDataGenerator : public DataGeneratorInterface {
    /** @brief  builder object from which we are generating data */
    GenericQueryBuilder builder;
    RecordValidator recordValidator;
    DataSource baseSource;

    /** @brief  List of column indices to extract values from base source records */
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
    DataRecord _getNext() {
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
    void _setNext() {
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
    DataRecord _getCandidateNext() {
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
    GenericDataGenerator(GenericQueryBuilder builder) : builder(builder) {
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
            join.FKIndex =
                baseSource_metadatda->getColumn(_join.foreignKey).index;

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
    bool hasNext() { return _hasNext; }

    /**
     * @brief generates a new record
     *
     * @return DataRecord
     */
    DataRecord next() {
        DataRecord result = nextRecord;
        _setNext();
        return result;
    }  //< Base
};

/**
 * @brief Logical AND query
 *
 */
class AndQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    /**
     * @brief Construct a new And Query object
     *
     * @param query1
     * @param query2
     */
    AndQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    /**
     * @brief Get the Validator object
     *
     * @param metadata
     * @return RecordValidator
     */
    RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(new AndRecordValidator(
            query1->getValidator(metadata), query2->getValidator(metadata)));
    }
};

/**
 * @brief Logical OR query
 *
 */
class OrQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    /**
     * @brief Construct a new Or Query object
     *
     * @param query1
     * @param query2
     */
    OrQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    /**
     * @brief Get the Validator object
     *
     * @param metadata
     * @return RecordValidator
     */
    RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(new OrRecordValidator(
            query1->getValidator(metadata), query2->getValidator(metadata)));
    }
};

/**
 * @brief Logical NOT query
 *
 */
class NotQuery : public ConditionQuery {
    Query query1;

   public:
    /**
     * @brief Construct a new Not Query object
     *
     * @param query1
     */
    NotQuery(Query query1) : query1(query1) {}
    /**
     * @brief Get the Validator object
     *
     * @param metadata
     * @return RecordValidator
     */
    RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(
            new NotRecordValidator(query1->getValidator(metadata)));
    }
};

/**
 * @brief Equality Query
 *
 */
class EqualQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
    /**
     * @brief Construct a new Equal Query object
     *
     * @param name
     * @param value
     */
    EqualQuery(std::string name, std::any value) : name(name), value(value) {}
    /**
     * @brief Get the Validator object
     *
     * @param metadata
     * @return RecordValidator
     */
    RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(new EqualRecordValidator(name, value, metadata));
    }
};

/**
 * @brief Less Than Query
 *
 */
class LessThanQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
   /**
    * @brief Construct a new Less Than Query object
    * 
    * @param name 
    * @param value 
    */
    LessThanQuery(std::string name, std::any value)
        : name(name), value(value) {}

    /**
     * @brief Get the Validator object
     * 
     * @param metadata 
     * @return RecordValidator 
     */
    RecordValidator getValidator(Metadata metadata) {
        return RecordValidator(new LessThanRecordValidator(name, value, metadata));
    }
};
