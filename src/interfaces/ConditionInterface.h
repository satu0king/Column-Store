#pragma once

#include <memory>

#include "DataRecord.h"

/** @file
 * @brief RecordValidator Interfaces and Implementations for Query Processing
 */

/**
 * @brief Base RecordValidator Interface
 *
 */
class RecordValidatorInterface {
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
     * @brief Destroy the Record Validator Interface object
     *
     */
    virtual ~RecordValidatorInterface(){};
};

/**
 * @brief Shared Pointer for RecordValidator Interface
 *
 * RecordValidator are transfered between objects using this type
 */
typedef std::shared_ptr<RecordValidatorInterface> RecordValidator;

/**
 * @brief Extended RecordValidator Interface which handles Metadata
 *
 * Use this class if you need access to metadata information of columns
 *
 * @see Metadata
 *
 */
class ReccordValidatorMetadataInterface : public RecordValidatorInterface {
   protected:
    /** @brief metadata object
     * Use this object to extract column information such as column index and
     * column type
     */
    Metadata metadata;

   public:
    /**
     * @brief Construct a new Condition Metadata Interface object
     *
     * @param metadata
     */
    ReccordValidatorMetadataInterface(Metadata metadata) : metadata(metadata) {}
};

/**
 * @brief Inverts RecordValidator
 *
 */
class NotRecordValidator : public RecordValidatorInterface {
    RecordValidator condition;

   public:
    /**
     * @brief Construct a new Not Record Validator object
     *
     * @param condition
     */
    NotRecordValidator(RecordValidator condition) : condition(condition) {}

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
 * @brief Logical AND RecordValidator
 *
 */
class AndRecordValidator : public RecordValidatorInterface {
    RecordValidator validator1;
    RecordValidator validator2;

   public:
    /**
     * @brief Construct a new And Record Validator object
     *
     * @param validator1
     * @param validator2
     */
    AndRecordValidator(RecordValidator validator1, RecordValidator validator2)
        : validator1(validator1), validator2(validator2) {}

    /**
     * @brief Applies logical AND on validator1 and validator2
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        return validator1->validate(record) && validator2->validate(record);
    }
};

/**
 * @brief Logical OR RecordValidator
 *
 */
class OrRecordValidator : public RecordValidatorInterface {
    RecordValidator validator1;
    RecordValidator validator2;

   public:
    /**
     * @brief Construct a new Or Record Validator object
     *
     * @param validator1
     * @param validator2
     */
    OrRecordValidator(RecordValidator validator1, RecordValidator validator2)
        : validator1(validator1), validator2(validator2) {}

    /**
     * @brief Applies logical OR condition to validator1 and validator2
     *
     * @param record
     * @return true
     * @return false
     */
    bool validate(DataRecord &record) {
        return validator1->validate(record) || validator2->validate(record);
    }
};

/**
 * @brief Checks equality of a data field
 *
 */
class EqualRecordValidator : public ReccordValidatorMetadataInterface {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    /**
     * @brief Construct a new Equal Record Validator object
     *
     * @param columnName
     * @param value
     * @param _metadata
     */
    EqualRecordValidator(std::string columnName, std::any value,
                         Metadata _metadata)
        : ReccordValidatorMetadataInterface(_metadata), value(value) {
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
class LessThanRecordValidator : public ReccordValidatorMetadataInterface {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    /**
     * @brief Construct a new Less Than Record Validator object
     *
     * @param columnName
     * @param value
     * @param _metadata
     */
    LessThanRecordValidator(std::string columnName, std::any value,
                            Metadata _metadata)
        : ReccordValidatorMetadataInterface(_metadata), value(value) {
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
