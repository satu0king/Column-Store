#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "ForeignKey.h"
#include "Column.h"

using namespace std;

class TableNotFoundException : public exception {
    std::string msg;
    public:
        TableNotFoundException(std::string &t_name) : msg("Table " + t_name + " not found") {}
        virtual const char* what() const throw() {
            return msg.c_str(); 
        }
};

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
            if(i >= columns.size()) {
                throw runtime_error("Index out of Bounds in Table:" + table_name);
            }
            return columns[i];
        }
        Column& get_column(std::string name) {return (*this)[name];}
        Column& operator[](std::string name) {
            if(column_map.find(name) == column_map.end()) {
                throw runtime_error("Table " + table_name + " does not have column " + name);
            }
            return columns[column_map[name]];
        }
};