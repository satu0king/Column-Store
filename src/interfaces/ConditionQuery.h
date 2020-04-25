#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Validators.h"
#include "DataGeneratorInterface.h"

/** @file
 * @brief Generic Query Builder
 *
 * This file contains structures required to construct the condition query any data source.
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
