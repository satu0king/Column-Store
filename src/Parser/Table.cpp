#include "Table.h"

using namespace std;

void Table::add_column(string column_name, string data_type) {
    DataType dt;
    if (data_type == "string") {
        dt = DataType::STRING;
    } else if (data_type == "int") {
        dt = DataType::INT;
    } else if (data_type == "float") {
        dt = DataType::FLOAT;
    }

    Column column = {column_name, dt, int(columns.size())};
    column_map[column_name] = columns.size();
    columns.push_back(column);
}

void Table::add_foreign_key(string from, string table, string to) {
    foreign_key fk = {from, table, to};
    foreign_keys.push_back(fk);
}