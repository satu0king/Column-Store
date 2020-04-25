#pragma once

#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>
#include <vector>

#include "SchemaExtractor.h"
#include "SchemaMetaData.h"

class SchemaLoader {
    string db_name, username, password;
    SchemaMetaData schema_meta_data;
    pqxx::connection *conn;
    void setup_database();
    void create_tables();
    void create_views();

   public:
    SchemaLoader(string db_name, string username, string password,
                 SchemaMetaData schema_meta_data);
};