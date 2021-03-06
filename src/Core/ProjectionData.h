#pragma once

#include <stdio.h>
#include <string.h>

#include <experimental/filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "ColumnStoreInitializer/StoreInitializer.h"
#include "Parser/Projection.h"
#include "Parser/SchemaLoader.h"
#include "interfaces/DataRecord.h"

#ifdef __cpp_lib_filesystem
namespace fs = std::filesystem;
#elif __cpp_lib_experimental_filesystem
namespace fs = std::experimental::filesystem;
#else
#error "no filesystem support ='("
#endif

using nlohmann::json;

namespace ColumnStore {

struct ColumnStoreData {
    std::vector<unsigned char> data;
    ColumnStoreData(std::vector<unsigned char> &data) : data(data) {}
    // ColumnStoreData(std::vector<unsigned char> &&data) : data(data) {}
    // ColumnStoreData(ColumnStoreData &&data) : data(data.data) {}
    ColumnStoreData(int size=0) : data(size) {}

    ColumnStoreData(DataRecord &record, std::vector<Parser::Column> &columns)
        : ColumnStoreData(columns) {
        set(record, columns);
    }

    ColumnStoreData(std::vector<Parser::Column> &columns) {
        int size = 0;
        for (auto &c : columns) {
            size += c;
            // std::cout << c.type.size << std::endl;
        }
        // std::cout << size << std::endl;
        data.resize(size);
    }

    float getFloat(int i) {
        return *reinterpret_cast<float *>(data.data() + i);
    }

    int getInt(int i) { return *reinterpret_cast<int *>(data.data() + i); }

    std::string getString(int i, int length) {
        return std::string(data.data() + i, data.data() + i + length);
    }

    void setFloat(int i, float value) {
        *reinterpret_cast<float *>(data.data() + i) = value;
    }

    void setInt(int i, int value) {
        *reinterpret_cast<int *>(data.data() + i) = value;
    }

    void setString(int i, std::string &string) {
        memcpy(data.data() + i, string.c_str(), string.size());
    }

    void setString(int i, std::string string) {
        memcpy(data.data() + i, string.c_str(), string.size());
    }

    int size() const { return data.size(); }

    operator const char *() const { return (const char *)data.data(); }
    operator char *() { return (char *)data.data(); }

    friend std::ostream &operator<<(std::ostream &output,
                                    const ColumnStoreData &data) {
        output.write(data, data.size());
        return output;
    }

    friend std::istream &operator>>(std::istream &input,
                                    ColumnStoreData &data) {
        input.read(data, data.size());
        return input;
    }

    void read(std::istream &input) { input.read(*this, data.size()); }

    void write(std::ostream &output) { output.write(*this, data.size()); }

    DataRecord get(std::vector<Parser::Column> &columns) {
        std::vector<DataValue> values;
        values.reserve(columns.size());

        int offset = 0;
        for (int i = 0; i < columns.size(); ++i) {
            Parser::DataType type = columns[i];
            if (type == ColumnStore::DataType::INT) {
                values.push_back(DataValue(getInt(offset)));
                offset += 4;
            } else if (type == ColumnStore::DataType::FLOAT) {
                values.push_back(DataValue(getFloat(offset)));
                offset += 4;
            } else if (type == ColumnStore::DataType::STRING) {
                values.push_back(DataValue(getString(offset, type.size)));
                offset += type.size;
            } else
                throw std::runtime_error("Unknown DataType");
        }
        return DataRecord(values);
    }

    void set(DataRecord &record, std::vector<Parser::Column> &columns) {
        int offset = 0;
        for (int i = 0; i < columns.size(); ++i) {
            Parser::DataType type = columns[i];
            if (type == ColumnStore::DataType::INT) {
                setInt(offset, record[i].as<int>());
                offset += 4;
            } else if (type == ColumnStore::DataType::FLOAT) {
                setFloat(offset, record[i].as<float>());
                offset += 4;
            } else if (type == ColumnStore::DataType::STRING) {
                setString(offset, record[i].as<std::string>());
                offset += type.size;
            } else
                throw std::runtime_error("Unknown DataType");
        }
    }
};

class MetadataManager {
    fs::path file;
    json metadata;
    Parser::SchemaMetaData schemaMetadata;

    static std::mutex lock;
    static std::map<fs::path, std::shared_ptr<MetadataManager>> metaMap;

    MetadataManager(fs::path file) : file(file) {
        std::ifstream inp(file);
        inp >> metadata;
        inp.close();

        Parser::SchemaExtractor schema_extractor(metadata["schema_path"]);
        schemaMetadata = schema_extractor.get_meta_data();
    }

   public:
    static std::shared_ptr<MetadataManager> getInstance(
        std::string column_store_path) {
        fs::path file = getMetaDataPath(column_store_path);

        if (MetadataManager::metaMap.count(file))
            return MetadataManager::metaMap[file];

        std::lock_guard guard(MetadataManager::lock);

        // Using Double Checked Locking
        if (!MetadataManager::metaMap.count(file))
            MetadataManager::metaMap[file] =
                std::shared_ptr<MetadataManager>(new MetadataManager(file));

        return MetadataManager::metaMap[file];
    }

    void save() {
        std::ofstream o(file);
        o << metadata.dump(4) << std::endl;
        o.close();
    }

    operator json &() { return metadata; }
    operator Parser::SchemaMetaData &() { return schemaMetadata; }
    json &getFileMetadata() { return metadata; }
    Parser::SchemaMetaData &getSchemaMetadata() { return schemaMetadata; }

    json &getProjectionFileInfo(std::string projection) {
        assert(metadata["projections"].count(projection));
        return metadata["projections"][projection];
    }

    Parser::Projection &getProjectionSchemaInfo(std::string projection) {
        return schemaMetadata.get_projection(projection);
    }
};

std::mutex MetadataManager::lock;
std::map<fs::path, std::shared_ptr<MetadataManager>> MetadataManager::metaMap;

typedef std::shared_ptr<MetadataManager> MetadataManagerSingleton;

class ColStoreLoader {
    MetadataManagerSingleton manager;

   public:
    ColStoreLoader(std::string column_store_path)
        : manager(MetadataManager::getInstance(column_store_path)) {}

    void update(std::string projection_name) {
        std::cout << "Updating " << projection_name << std::endl;
        auto &fileData = manager->getProjectionFileInfo(projection_name);
        int tuplesMoved = fileData["tuples_move_count"];
        auto &schema = manager->getProjectionSchemaInfo(projection_name);
        auto &db_metadata = manager->getFileMetadata();
        fs::path file = manager->getProjectionFileInfo(projection_name)["file"];
        Postgres::PostgreSQLMetaData postgresql_metadata(db_metadata["source_db_config"]["db_name"],
                                                        db_metadata["source_db_config"]["db_user"],
                                                        db_metadata["source_db_config"]["db_password"],
                                                        manager->getSchemaMetadata());
        Postgres::PostgreSQLDataSource postgresql_data_source(postgresql_metadata, projection_name);
        postgresql_data_source.advance(tuplesMoved);
        std::vector<Parser::Column> projection_columns = schema.get_metadata_columns();
        ColumnStoreData projection_data(projection_columns);

        std::ofstream o(file, std::ofstream::out | std::ofstream::app);
        int c = 0;
        while (postgresql_data_source.hasNext()) {
                auto record = postgresql_data_source.next();
                projection_data.set(record, projection_columns);
                projection_data.write(o);
                c++;
                if(c % 1000 == 0)
                    std::cout << c << " records processed" << std::endl;
                
        }
        std::cout << c << " records processed" << std::endl;
        o.close();

        int newTotal = tuplesMoved + c;

        fileData["tuples_move_count"] = newTotal;
        manager->save();
    }

    void updateAll() {
        for (auto &[projection_name, _] :
             manager->getFileMetadata()["projections"].items()) {
            update(projection_name);
        }

        manager->save();
    }
};

};  // namespace ColumnStore