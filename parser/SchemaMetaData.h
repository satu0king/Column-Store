#pragma once

#include <vector>
#include "Table.h"

class SchemaMetaData {
    vector<Table> tables;

    public:
        void add_table(Table table) {tables.push_back(table);}
        vector<Table> get_tables() {return tables;}
};