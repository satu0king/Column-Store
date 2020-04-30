#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <nlohmann/json.hpp>

#include "interfaces/DataGeneratorInterface.h"

using ColumnStore::DataRecord;
using ColumnStore::DataRecordMetadata;
using ColumnStore::DataSource;
using ColumnStore::DataType;
using ColumnStore::DataValue;
using ColumnStore::Metadata;
using std::vector;
using std::queue;
using std::string;

namespace ColumnStore {
class ColStoreDataSource : public ColumnStore::DataGeneratorInterface {
    MetadataManagerSingleton manager;
    std::ifstream file;
    string projection_name;
    int count, tuples_move_count;
    ColumnStoreData column_store_data;
    vector<Parser::Column> columns;

    public:
        ColStoreDataSource(string column_store_path, string p_name)
            : manager(MetadataManager::getInstance(column_store_path)), projection_name(p_name) {
                count = 0;
                json &projection_json = manager->getProjectionFileInfo(projection_name);
                file = std::ifstream((std::string)projection_json["file"]);
                tuples_move_count = projection_json["tuples_move_count"];
                Parser::Projection &projection = manager->getProjectionSchemaInfo(projection_name);
                columns = projection.get_metadata_columns();
                column_store_data = ColumnStoreData(columns);
                metadata = Metadata(projection.get_metadata());
            }
            
        DataRecord next() {
            count++;
            column_store_data.read(file);
            return column_store_data.get(columns);
        }

        bool hasNext() { return count < tuples_move_count; }
};
}