#pragma once 

#include <iostream>
#include <string>
#include <pqxx/pqxx>

#include "Parser/SchemaExtractor.h"
#include "Parser/SchemaMetaData.h"

class PostgreSQLMetaData {
    std::string db_name, username, password;
    pqxx::connection *conn;
    SchemaMetaData schema_meta_data;

    public:
        PostgreSQLMetaData(std::string db_name, std::string username, std::string password, SchemaMetaData schema);
        pqxx::connection *get_connection() const { return conn; }
        std::string get_db_name() const { return db_name; }
        std::string get_username() const { return username; }
        std::string get_password() const { return password; }
        SchemaMetaData get_schema_meta_data() const { return schema_meta_data; }
};