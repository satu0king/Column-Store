#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Column.h"
#include "ForeignKey.h"
#include "JoinIndex.h"

namespace Parser {
using std::exception;
using std::runtime_error;
using std::string;
using std::vector;
using std::unordered_map;
class ProjectionNotFoundException : public exception {
    std::string msg;

   public:
    ProjectionNotFoundException(std::string& p_name)
        : msg("Projection " + p_name + " not found") {}
    virtual const char* what() const throw() { return msg.c_str(); }
};

struct projection_column {
    std::string name;
    std::string table;
    std::string column_name;
    std::string encoding;
    DataType data_type;
    int index;
};

class Projection {
    std::string projection_name;
    std::string sort_key;
    vector<projection_column> columns;
    unordered_map<string, int> column_map;
    std::string base_table;
    vector<foreign_key> join_tables;
    vector<join_index> join_indexes;

   public:
    void set_projection_name(std::string projection_n) {
        projection_name = projection_n;
    }
    void set_base_table(std::string base_tab) { base_table = base_tab; }
    void set_sort_key(std::string sk) { sort_key = sk; }
    void add_column(std::string name, std::string table,
                    std::string column_name, std::string encoding,
                    DataType data_type);
    void add_join_table(std::string from, std::string table, std::string to);
    void add_join_index(std::string from, std::string other_projection,
                        std::string to);
    std::string get_projection_name() { return projection_name; }
    std::string get_base_table() { return base_table; }
    std::string get_sort_key() { return sort_key; }
    vector<foreign_key> get_join_tables() { return join_tables; }
    vector<projection_column> get_columns() { return columns; }
    vector<join_index> get_join_indexes() { return join_indexes; }
    projection_column& operator[](int i) {
        if (i >= columns.size()) {
            throw runtime_error("Index out of Bounds in Projection:" +
                                projection_name);
        }
        return columns[i];
    }
    projection_column& get_column(std::string name) { return (*this)[name]; }
    projection_column& operator[](std::string name) {
        if (column_map.find(name) == column_map.end()) {
            throw runtime_error("Projection " + projection_name +
                                " does not have column " + name);
        }
        return columns[column_map[name]];
    }
};
}  // namespace Parser