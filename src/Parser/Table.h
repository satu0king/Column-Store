#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Column.h"
#include "ForeignKey.h"

namespace Parser {
using std::exception;
using std::string;
using std::vector;
using std::runtime_error;

class TableNotFoundException : public exception {
    std::string msg;

   public:
    TableNotFoundException(std::string& t_name)
        : msg("Table " + t_name + " not found") {}
    virtual const char* what() const throw() { return msg.c_str(); }
};

class Table {
    std::string table_name;
    std::unordered_map<std::string, int> column_map;
    std::vector<Parser::Column> columns;
    std::string primary_key;
    std::vector<foreign_key> foreign_keys;

   public:
    void set_table_name(std::string table_n) { table_name = table_n; }
    void set_primary_key(std::string pk) { primary_key = pk; }
    void add_column(std::string column_name, std::string data_type,
                    int size = 0);
    void add_foreign_key(std::string from, std::string table, std::string to);
    std::string get_table_name() { return table_name; }
    std::string get_primary_key() { return primary_key; }
    std::vector<foreign_key>& get_foreign_keys() { return foreign_keys; }
    std::vector<Parser::Column>& get_columns() { return columns; }
    Parser::Column& operator[](int i) {
        if (i >= int(columns.size())) {
            throw runtime_error("Index out of Bounds in Table:" + table_name);
        }
        return columns[i];
    }
    Parser::Column& get_column(std::string name) { return (*this)[name]; }
    Parser::Column& operator[](std::string name) {
        if (column_map.find(name) == column_map.end()) {
            throw runtime_error("Table " + table_name +
                                " does not have column " + name);
        }
        return columns[column_map[name]];
    }
};
}  // namespace Parser