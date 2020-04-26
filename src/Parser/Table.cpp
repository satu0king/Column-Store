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
    else throw std::runtime_error("Unknown Type");
    Parser::DataType parser_data_type(dt, size);

    Parser::Column column = {column_name, parser_data_type, int(columns.size())};
    column_map[column_name] = columns.size();
    columns.push_back(column);
}

void Parser::Table::add_foreign_key(string from, string table, string to) {
    Parser::foreign_key fk = {from, table, to};
    foreign_keys.push_back(fk);
}