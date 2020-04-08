#pragma once;
#include <memory>
#include <string>
#include <unordered_map>

#include "ConditionsInterfaces/ConditionInterface.h"
#include "DataGeneratorInterface.h"

struct Join {
    std::string foreignKey;
    std::string primaryKey;
    std::vector<std::string> columns;
};

class ConditionQuery {
   public:
    virtual ConditionChecker getChecker(Metadata metadata) = 0;
    virtual ~ConditionQuery() {}
};
typedef std::shared_ptr<ConditionQuery> Query;

class GenericDataGenerator;

class GenericQueryBuilder {
    std::unordered_map<std::string, DataSource> data_sources;

    std::string baseSource;
    std::vector<std::string> baseColumns;

    std::vector<std::pair<std::string, Join>> joins;
    std::unordered_map<std::string, std::string> columnMap;

    Query query;

    friend class GenericDataGenerator;

   public:
    void registerDataSource(std::string name, DataSource source) {
        data_sources[name] = source;
    }

    void setBaseSource(std::string name, std::vector<std::string> columns) {
        baseSource = name;
        baseColumns = columns;

        for (std::string column : columns) {
            assert(columnMap.find(column) != columnMap.end());
            columnMap[column] = name;
        }
    }

    void join(std::string name, std::vector<std::string> columns,
              std::string foreignKey, std::string primaryKey) {
        Join join = {foreignKey, primaryKey, columns};

        joins.emplace_back(name, join);

        for (std::string column : columns) {
            assert(columnMap.find(column) != columnMap.end());
            columnMap[column] = name;
        }
    }

    void where(Query query) { this->query = query; }

    Metadata generateMetadata() {
        std::vector<Column> columns;

        auto helper = [&](std::string source,
                          std::vector<std::string> _columns) {
            for (auto const& name : _columns) {
                DataType type =
                    data_sources[source]->getMetadata()->getColumn(name).type;
                Column c = {
                    .name = name, .type = type, .index = columns.size()};

                columns.push_back(c);
            }
        };

        helper(baseSource, baseColumns);
        for (const auto& [source, join] : joins) {
            helper(source, join.columns);
        }

        return Metadata(new DataRecordMetadata(columns));
    }

    ConditionChecker generateConditionChecker() {
        return query->getChecker(generateMetadata());
    }
};

class GenericDataGenerator : public DataGeneratorInterface {
    GenericQueryBuilder builder;
    Metadata metadata;
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

        for (int i : baseIndices) values.push_back(baseRecord[i]);

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

            int PKIndex = join_metadata->getColumn(_join.foreignKey).index;

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
                join.map[record[PKIndex].as<int>()] = std::move(record);
            }

            joins.push_back(join);
        }
    }

    bool hasNext() const { return _hasNext; }

    DataRecord next() {
        DataRecord result = std::move(nextRecord);
        _setNext();
        return result;
    }
};

class AndQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    AndQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    ConditionChecker getConditionObject(Metadata m) {
        return ConditionChecker(
            new AndCondition(query1->getChecker(m), query2->getChecker(m)));
    }
};

class OrQuery : public ConditionQuery {
    Query query1;
    Query query2;

   public:
    OrQuery(Query query1, Query query2) : query1(query1), query2(query2) {}
    ConditionChecker getConditionObject(Metadata m) {
        return ConditionChecker(
            new OrCondition(query1->getChecker(m), query2->getChecker(m)));
    }
};

class NotQuery : public ConditionQuery {
    Query query1;

   public:
    NotQuery(Query query1) : query1(query1) {}
    ConditionChecker getConditionObject(Metadata m) {
        return ConditionChecker(new NotCondition(query1->getChecker(m)));
    }
};

class EqualQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
    EqualQuery(std::string name, std::any value) : name(name), value(value) {}

    ConditionChecker getConditionObject(Metadata m) {
        return ConditionChecker(new EqualCondition(name, value, m));
    }
};

class LessThanQuery : public ConditionQuery {
    std::string name;
    std::any value;

   public:
    LessThanQuery(std::string name, std::any value)
        : name(name), value(value) {}

    ConditionChecker getConditionObject(Metadata m) {
        return ConditionChecker(new LessThanCondition(name, value, m));
    }
};
