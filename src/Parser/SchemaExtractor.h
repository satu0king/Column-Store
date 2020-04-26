#pragma once

#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>
#include <vector>

#include "../interfaces/Column.h"
#include "SchemaMetaData.h"
#include "pugixml-1.10/src/pugixml.hpp"

namespace Parser {
    class SchemaExtractor {
        const char* xml_file_path;
        pugi::xml_document xml_dom;
        SchemaMetaData schema_meta_data;

        void create_tables();
        void create_projections();

    public:
        SchemaExtractor(std::string xml_fpath);
        SchemaMetaData get_meta_data() { return schema_meta_data; }
    };
}