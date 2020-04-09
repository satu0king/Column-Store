#pragma once

#include <memory>

#include "../DataRecord.h"

/** @file
 * @brief Condition Interfaces and Implementations for Query Processing
 */

/**
 * @brief Base Condition Interface
 *
 */
class ConditionInterface {
   public:
    /**
     * @brief Validates record as per query conditions
     *
     * @param record
     * @return true
     * @return false
     */
    virtual bool validate(DataRecord &record) { return true; }

    /**
     * @brief Destroy the Condition Interface object
     *
     */
    virtual ~ConditionInterface(){};
};

/**
 * @brief Shared Pointer for Condition Interface
 *
 * Conditions are transfered between objects using this type
 */
typedef std::shared_ptr<ConditionInterface> ConditionChecker;

/**
 * @brief Extended Condition Interface which handles Metadata
 *
 * Use this class if you need access to metadata information of columns
 *
 * @see Metadata
 *
 */
class ConditionMetadataInterface : public ConditionInterface {
   protected:
    Metadata metadata;

   public:
    /**
     * @brief Construct a new Condition Metadata Interface object
     *
     * @param metadata
     */
    ConditionMetadataInterface(Metadata metadata) : metadata(metadata) {}
};

/**
 * @brief Inverts condition
 *
 */
class NotCondition : public ConditionInterface {
    ConditionChecker condition;

   public:
    /**
     * @brief Construct a new Not Condition object
     *
     * @param condition
     */
    NotCondition(ConditionChecker condition) : condition(condition) {}

    /**
     * @brief inverts the value of the condition
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) { return !condition->validate(record); }
};

/**
 * @brief Logical AND condition
 *
 */
class AndCondition : public ConditionInterface {
    ConditionChecker condition1;
    ConditionChecker condition2;

   public:
    /**
     * @brief Construct a new And Condition object
     *
     * @param condition1
     * @param condition2
     */
    AndCondition(ConditionChecker condition1, ConditionChecker condition2)
        : condition1(condition1), condition2(condition2) {}

    /**
     * @brief Applies logical AND on condition1 and condition2
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        return condition1->validate(record) && condition2->validate(record);
    }
};

/**
 * @brief Logical OR condition
 *
 */
class OrCondition : public ConditionInterface {
    ConditionChecker condition1;
    ConditionChecker condition2;

   public:
    /**
     * @brief Construct a new Or Condition object
     *
     * @param condition1
     * @param condition2
     */
    OrCondition(ConditionChecker condition1, ConditionChecker condition2)
        : condition1(condition1), condition2(condition2) {}

    /**
     * @brief Applies logical OR condition to condition1 and condition2
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        return condition1->validate(record) || condition2->validate(record);
    }
};

/**
 * @brief Checks equality of a data field
 *
 */
class EqualCondition : public ConditionMetadataInterface {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    /**
     * @brief Construct a new Equal Condition object
     *
     * @param columnName of column to compare
     * @param value of the column
     * @param _metadata
     */
    EqualCondition(std::string columnName, std::any value, Metadata _metadata)
        : ConditionMetadataInterface(_metadata), value(value) {
        auto &metadata = *(this->metadata.get());
        columnIndex = metadata[columnName].index;
        dataType = metadata[columnName].type;
    }

    /**
     * @brief Compares the equality of the column
     *
     * Type information and column index is extracted from the metadata
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        if (dataType == DataType::INT) {
            return std::any_cast<int>(value) == record[columnIndex].as<int>();
        } else if (dataType == DataType::FLOAT) {
            return std::any_cast<float>(value) ==
                   record[columnIndex].as<float>();
        } else if (dataType == DataType::STRING) {
            return std::any_cast<std::string>(value) ==
                   record[columnIndex].as<std::string>();
        }

        throw "Unknown DataType";
    }
};

/**
 * @brief Checks if value of a field is less than given value
 *
 */
class LessThanCondition : public ConditionMetadataInterface {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    /**
     * @brief Construct a new Less Than Condition object
     *
     * @param columnName
     * @param value
     * @param _metadata
     */
    LessThanCondition(std::string columnName, std::any value,
                      Metadata _metadata)
        : ConditionMetadataInterface(_metadata), value(value) {
        columnIndex = metadata->operator[](columnName).index;
        dataType = (*metadata)[columnName].type;
    }

    /**
     * @brief Compares the value with given value
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        if (dataType == DataType::INT) {
            return std::any_cast<int>(value) > record[columnIndex].as<int>();
        } else if (dataType == DataType::FLOAT) {
            return std::any_cast<float>(value) >
                   record[columnIndex].as<float>();
        } else if (dataType == DataType::STRING) {
            return std::any_cast<std::string>(value) >
                   record[columnIndex].as<std::string>();
        }

        throw "Unknown DataType";
    }
};
