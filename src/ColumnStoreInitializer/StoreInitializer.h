#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Parser/SchemaMetaData.h"

#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#elif __cpp_lib_experimental_filesystem
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "no filesystem support ='("
#endif

using nlohmann::json;

namespace ColumnStore {

fs::path getProjectionPath(std::string column_store_path,
                           std::string projection_name) {
    return fs::absolute(fs::path(column_store_path) / "projections" /
                        (projection_name + ".projection"));
}

fs::path getMetaDataPath(std::string column_store_path) {
    return fs::absolute(fs::path(column_store_path) / "metadata.json");
}

fs::path getSchemaPath(std::string column_store_path) {
    return fs::absolute(fs::path(column_store_path) / "schema.xml");
}

fs::path getJoinIndexPath(std::string column_store_path,
                          std::string source_projection_name,
                          Parser::join_index join_index) {
    std::string fileName =
        source_projection_name + "_" + join_index.projection + ".index";
    auto path = fs::path(column_store_path) / "joins" / fileName;
    return fs::absolute(path);
}

void createFile(fs::path path) {
    fs::create_directories(path.parent_path());

    std::ofstream ofs(path);
    ofs.close();
}

class Initializer {
    void _createProjection(std::string column_store_path,
                           Parser::Projection projection) {
        std::string name = projection.get_projection_name();
        fs::path path = getProjectionPath(column_store_path, name);

        createFile(path);

        for (auto &index : projection.get_join_indexes()) {
            fs::path path = getJoinIndexPath(column_store_path, name, index);
            createFile(path);
        }
    }

    json generateMetadataProjection(Parser::Projection &projection) {
        std::string name = projection.get_projection_name();

        json projection_metadata = {
            {"name", name},
            {"file", getProjectionPath(column_store_path, name)},
            {"tuples_move_count", 0},
            {"joins", json::object()},
        };

        for (auto &index : projection.get_join_indexes()) {
            projection_metadata["joins"][index.projection] =
                getJoinIndexPath(column_store_path, name, index);
        }
        return projection_metadata;
    }

    json metadata;
    std::string column_store_path;

   public:
    Initializer(std::string column_store_path)
        : column_store_path(column_store_path) {}

    void saveMetadata() {
        // Print MetaData
        std::cout << metadata.dump(4) << std::endl;

        // Store Metadata
        std::ofstream o(getMetaDataPath(column_store_path));
        o << metadata.dump(4) << std::endl;
        o.close();
    }

    void createProjections(Parser::SchemaMetaData schema_metadata) {
        metadata["projections"] = json::object();

        for (auto &projection : schema_metadata.get_projections()) {
            _createProjection(column_store_path, projection);
        }

        for (auto &projection : schema_metadata.get_projections()) {
            std::string name = projection.get_projection_name();

            metadata["projections"][name] =
                generateMetadataProjection(projection);
        }
    }

    void configureSourceDB(std::string db_name, std::string db_user,
                           std::string db_password) {
        metadata["source_db_config"] = {{"db_name", db_name},
                                        {"db_user", db_user},
                                        {"db_password", db_password}};
    }

    void configureSchema(std::string schame_path) {
        fs::path path = getSchemaPath(column_store_path);
        fs::copy_file(schame_path, path, fs::copy_options::overwrite_existing);
        metadata["schema_path"] = path;
    }
};
}  // namespace ColumnStore