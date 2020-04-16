#pragma once

#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <fstream>

class DataLoader {
    std::string db_name, username, password, sql_file_name;
    pqxx::connection *conn;
    void load_data();

    public:
        DataLoader(std::string db_name, std::string username, std::string password, std::string sql_file_name);
};