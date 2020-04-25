#pragma once

#include "pugixml-1.10/src/pugixml.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>
#include <vector>
#include "SchemaMetaData.h"
#include "../interfaces/Column.h"

class SchemaExtractor {
    const char* xml_file_path;
    pugi::xml_document xml_dom;
    SchemaMetaData schema_meta_data;

    void create_tables();
    void create_projections();
    
    public:
        SchemaExtractor(std::string xml_fpath);
        SchemaMetaData get_meta_data() {return schema_meta_data;}
};