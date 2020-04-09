#pragma once

#include <vector>
#include "Table.h"
#include "Projection.h"

class SchemaMetaData {
    vector<Table> tables;
    vector<Projection> projections;

    public:
        void add_table(Table table) {tables.push_back(table);}
        void add_projection(Projection projection) {projections.push_back(projection);}
        vector<Table> get_tables() {return tables;}
        vector<Projection> get_projections() {return projections;}
};