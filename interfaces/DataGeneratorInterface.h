#pragma once;

#include <memory>
#include <vector>

#include "Column.h"
#include "DataRecord.h"

class DataGeneratorInterface {
   protected:
    Metadata metadata;

   public:
    virtual DataRecord next() = 0;
    virtual bool hasNext() = 0;

    Metadata getMetadata() const { return metadata; }

    virtual std::vector<DataRecord> nextBatch(int records) {
        assert(records >= 0);

        std::vector<DataRecord> results;
        for (int i = 0; i < records && hasNext(); ++i) {
            results.push_back(next());
        }
        return results;
    }
    virtual std::vector<Column> getColumns() = 0;

    virtual ~DataGeneratorInterface(){};
};

typedef std::shared_ptr<DataGeneratorInterface> DataSource;