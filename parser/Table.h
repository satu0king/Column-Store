#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "ForeignKey.h"
#include "../interfaces/Column.h"

using namespace std;

class Table {
    string table_name;
    unordered_map<string, int> column_map;
    vector<Column> columns;
    string primary_key;
    vector<foreign_key> foreign_keys;

    public:
        void set_table_name(string table_n) {table_name = table_n;}
        void set_primary_key(string pk) {primary_key = pk;}
        void add_column(string column_name, string data_type);
        void add_foreign_key(string from, string table, string to);
        string get_table_name() {return table_name;}
        string get_primary_key() {return primary_key;}
        vector<foreign_key>& get_foreign_keys() {return foreign_keys;}
        vector<Column>& get_columns() {return columns;}
        Column& operator[](int i) {
            assert(i < columns.size());
            return columns[i];
        }
        Column& get_column(std::string name) {return (*this)[name];}
        Column& operator[](std::string name) {
            assert(column_map.find(name) != column_map.end());
            return columns[column_map[name]];
        }
};