#pragma once

#include <memory>
#include <vector>
#include "Column.h"
#include "DataRecord.h"
#include "fort.hpp"

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

    void print() {
        auto m = getMetadata();
        fort::char_table table;
        table << fort::header;

        auto columns = m->getColumns();

        for (int i = 0; i < columns.size(); i++) table << columns[i].name;

        table << fort::endr;

        while (hasNext()) {
            auto record = next();
            for (int i = 0; i < columns.size(); i++) {
                auto type = columns[i].type;
                if (type == DataType::INT)
                    table << record[i].as<int>();
                else if (type == DataType::FLOAT)
                    table << record[i].as<float>();
                else if (type == DataType::STRING)
                    table << record[i].as<std::string>();
                else
                    throw std::runtime_error("Unknown DataType");
            }
            table << fort::endr;
        }

        std::cout << table.to_string() << std::endl;
    }

    virtual ~DataGeneratorInterface(){};
};

typedef std::shared_ptr<DataGeneratorInterface> DataSource;