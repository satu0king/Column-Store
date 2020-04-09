#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "ForeignKey.h"

using namespace std;

class Table {
    string table_name;
    unordered_map<string, string> columns;
    string primary_key;
    vector<foreign_key> foreign_keys;

    public:
        void set_table_name(string table_n) {table_name = table_n;}
        void set_primary_key(string pk) {primary_key = pk;}
        void add_column(string column_name, string data_type);
        void add_foreign_key(string from, string table, string to);
        string get_table_name() {return table_name;}
        string get_primary_key() {return primary_key;}
        unordered_map<string, string> get_columns() {return columns;}
        vector<foreign_key> get_foreign_keys() {return foreign_keys;}
};