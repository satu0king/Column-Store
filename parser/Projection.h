#pragma once

#include <string>
#include <vector>
#include "ForeignKey.h"
#include "JoinIndex.h"

using namespace std;

struct projection_column {
    std::string name;
    std::string table;
    std::string column_name;
    std::string encoding;
};

class Projection {
    std::string projection_name;
    std::string sort_key;
    vector<projection_column> columns;
    std::string base_table;
    vector<foreign_key> join_tables;
    vector<join_index> join_indexes;

    public:
        void set_projection_name(std::string projection_n) {projection_name = projection_n;}
        void set_base_table(std::string base_tab) {base_table = base_tab;}
        void set_sort_key(std::string sk) {sort_key = sk;}
        void add_column(std::string name, std::string table, std::string column_name, std::string encoding);
        void add_join_table(std::string from, std::string table, std::string to);
        void add_join_index(std::string from, std::string other_projection, std::string to);
        std::string get_projection_name() {return projection_name;}
        std::string get_base_table() {return base_table;}
        std::string get_sort_key() {return sort_key;}
        vector<foreign_key> get_join_tables() {return join_tables;}
        vector<projection_column> get_columns() {return columns;}
        vector<join_index> get_join_indexes() {return join_indexes;}
};