#pragma once
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

#include "DataGeneratorInterface.h"

/** @file
 * @brief Generic Condition Builder
 *
 * This file contains structures required to construct the condition query any
 * data source.
 */

namespace ColumnStore {
/**
 * @brief Base Condition Query Structure
 *
 * ConditionQuery structures are used to create the query. ConditionInterface
 * structures are used to validate the records
 *
 * see @ConditionInterface
 */

struct Aggregator {
    std::string name;

    int columnIndex;
    DataType type;

    Aggregator(std::string name) : name(name) {}

    void initialize(Metadata metadata) {
        columnIndex = (*metadata)[name].index;
        type = (*metadata)[name].type;
    }

    virtual void addRecord(DataRecord &record) = 0;
    virtual float getValue() = 0;

    /**
     * @brief Destroy the Condition Query object
     *
     */
    virtual ~Aggregator() {}
    virtual std::string aggregatorType() = 0;
    std::string getColumnName() { return aggregatorType() + "(" + name + ")"; }

    virtual std::shared_ptr<Aggregator> clone() = 0;
};

/**
 * @brief Shared pointer to Query Structure
 *
 */
typedef std::shared_ptr<Aggregator> AggregatorQuery;

struct AverageAggregator : public Aggregator {
    int count;
    double sum;

    AverageAggregator(std::string name) : Aggregator(name), count(0), sum(0) {}

    std::string aggregatorType() { return "Avg"; }

    void addRecord(DataRecord &record) {
        if (type == DataType::INT)
            sum += record[columnIndex].as<int>();
        else if (type == DataType::FLOAT)
            sum += record[columnIndex].as<float>();
        else
            throw std::runtime_error("Unknown DataType");
        count++;
    }

    AggregatorQuery clone() {
        return AggregatorQuery(new AverageAggregator(*this));
    }

    float getValue() {
        assert(count);
        return sum / count;
    }
};

struct SumAggregator : public Aggregator {
    double sum;

    SumAggregator(std::string name) : Aggregator(name), sum(0) {}

    std::string aggregatorType() { return "Sum"; }

    void addRecord(DataRecord &record) {
        if (type == DataType::INT)
            sum += record[columnIndex].as<int>();
        else if (type == DataType::FLOAT)
            sum += record[columnIndex].as<float>();
        else
            throw std::runtime_error("Unknown DataType");
    }

    AggregatorQuery clone() {
        return AggregatorQuery(new SumAggregator(*this));
    }

    float getValue() { return sum; }
};

struct CountAggregator : public Aggregator {
    int count;
    CountAggregator(std::string name) : Aggregator(name), count(0) {}

    std::string aggregatorType() { return "Count"; }

    void addRecord(DataRecord &record) { count++; }

    AggregatorQuery clone() {
        return AggregatorQuery(new CountAggregator(*this));
    }

    float getValue() { return count; }
};

struct MaxAggregator : public Aggregator {
    float mx;

    MaxAggregator(std::string name)
        : Aggregator(name), mx(std::numeric_limits<float>::min()) {}

    std::string aggregatorType() { return "Max"; }

    void addRecord(DataRecord &record) {
        if (type == DataType::INT)
            mx = std::max(mx, (float)record[columnIndex].as<int>());
        else if (type == DataType::FLOAT)
            mx = std::max(mx, record[columnIndex].as<float>());
        else
            throw std::runtime_error("Unknown DataType");
    }

    float getValue() { return mx; }

    AggregatorQuery clone() {
        return AggregatorQuery(new MaxAggregator(*this));
    }
};

struct MinAggregator : public Aggregator {
    float mn;

    MinAggregator(std::string name)
        : Aggregator(name), mn(std::numeric_limits<float>::max()) {}

    std::string aggregatorType() { return "Min"; }

    void addRecord(DataRecord &record) {
        if (type == DataType::INT)
            mn = std::min(mn, (float)record[columnIndex].as<int>());
        else if (type == DataType::FLOAT)
            mn = std::min(mn, record[columnIndex].as<float>());
        else
            throw std::runtime_error("Unknown DataType");
    }

    float getValue() { return mn; }

    AggregatorQuery clone() {
        return AggregatorQuery(new MinAggregator(*this));
    }
};

};  // namespace ColumnStore
