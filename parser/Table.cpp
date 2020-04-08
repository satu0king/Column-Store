#include "Table.h"

using namespace std;

void Table::add_column(string column_name, string data_type) {
    columns[column_name] = data_type;
}

void Table::add_foreign_key(string from, string table, string to) {
    foreign_key fk = {from, table, to};
    foreign_keys.push_back(fk);
}