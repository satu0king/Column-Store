#include "Table.h"

using namespace std;

void Parser::Table::add_column(string column_name, string data_type, int size) {
    ColumnStore::DataType dt;
    if (data_type == "string") {
        dt = ColumnStore::DataType::STRING;
    } else if (data_type == "int") {
        dt = ColumnStore::DataType::INT;
    } else if (data_type == "float") {
        dt = ColumnStore::DataType::FLOAT;
    }
    Parser::DataType parser_data_type(dt, size);

    ColumnStore::Column column = {column_name, dt, int(columns.size())};
    column_map[column_name] = columns.size();
    columns.push_back(column);
}

void Parser::Table::add_foreign_key(string from, string table, string to) {
    Parser::foreign_key fk = {from, table, to};
    foreign_keys.push_back(fk);
}