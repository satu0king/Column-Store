#pragma once

#include <memory>

#include "../DataRecord.h"

class ConditionInterface {
   public:
    virtual bool validate(DataRecord &){ return true;};
    virtual ~ConditionInterface(){};
};

typedef std::shared_ptr<ConditionInterface> ConditionChecker;

class ConditionImpl : public ConditionInterface {
   protected:
    Metadata metadata;

   public:
    ConditionImpl(Metadata metadata)
        : metadata(metadata) {}
};

class NotCondition : public ConditionInterface {
    ConditionChecker condition;

   public:
    NotCondition(ConditionChecker condition)
        : condition(condition) {}
    bool validate(DataRecord &record) { return !condition->validate(record); }
};

class AndCondition : public ConditionInterface {
    ConditionChecker condition1;
    ConditionChecker condition2;

   public:
    AndCondition(ConditionChecker condition1,
                 ConditionChecker condition2)
        : condition1(condition1), condition2(condition2) {}
    bool validate(DataRecord &record) {
        return condition1->validate(record) && condition2->validate(record);
    }
};

class OrCondition : public ConditionInterface {
    ConditionChecker condition1;
    ConditionChecker condition2;

   public:
    OrCondition(ConditionChecker condition1,
                 ConditionChecker condition2)
        : condition1(condition1), condition2(condition2) {}
    bool validate(DataRecord &record) {
        return condition1->validate(record) || condition2->validate(record);
    }
};

class EqualCondition : public ConditionImpl {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    EqualCondition(std::string columnName, std::any value,
                   Metadata _metadata)
        : ConditionImpl(_metadata), value(value) {
        auto &metadata = *(this->metadata.get());
        columnIndex = metadata[columnName].index;
        dataType = metadata[columnName].type;
    }

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

class LessThanCondition : public ConditionImpl {
    int columnIndex;
    std::any value;
    DataType dataType;

   public:
    LessThanCondition(std::string columnName, std::any value,
                      Metadata _metadata)
        : ConditionImpl(_metadata), value(value) {
        columnIndex = metadata->operator[](columnName).index;
        dataType = (*metadata)[columnName].type;
    }

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
