#pragma once

#include "../libs/pugixml-1.10/src/pugixml.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>
#include <vector>
#include "SchemaMetaData.h"

using namespace std;

class SchemaExtractor {
    const char* xml_file_path;
    pugi::xml_document xml_dom;
    SchemaMetaData schema_meta_data;

    void create_tables();
    
    public:
        SchemaExtractor(string xml_fpath);
        SchemaMetaData get_meta_data() {return schema_meta_data;}
};