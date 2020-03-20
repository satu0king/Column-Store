#pragma once;

#include <vector>

#include "DataRecord.h"
#include "Column.h"

class DataGeneratorInterface {
   public:
    virtual DataRecord next() = 0;
    virtual std::vector<DataRecord> nextBatch(int records) {
        assert(records >= 0);

        std::vector<DataRecord> results;
        for (int i = 0; i < records; ++i) {
            results.push_back(next());
        }
        return results;
    }
    virtual std::vector<Column> getColumns() = 0;
};