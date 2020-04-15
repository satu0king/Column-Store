#include "Projection.h"

using namespace std;

void Projection::add_column(string name, string table, string column_name, string encoding, DataType data_type) {
    projection_column column = {name, table, column_name, encoding, data_type};
    columns.push_back(column);
}

void Projection::add_join_table(string from, string table, string to) {
    foreign_key join_table = {from, table, to};
    join_tables.push_back(join_table);
}

void Projection::add_join_index(string from, string other_projection, string to) {
    join_index join_ind = {from, other_projection, to};
    join_indexes.push_back(join_ind);
}