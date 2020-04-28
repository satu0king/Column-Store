#pragma once

#include <assert.h>

#include <unordered_map>
#include <vector>

#include "Projection.h"
#include "Table.h"

namespace Parser {
using std::unordered_map;

class SchemaMetaData {
    vector<Table> tables;
    unordered_map<string, int> table_map;
    vector<Projection> projections;
    unordered_map<string, int> projection_map;

   public:
    void add_table(Table table) {
        table_map[table.get_table_name()] = tables.size();
        tables.push_back(table);
    }
    void add_projection(Projection projection) {
        projection_map[projection.get_projection_name()] = projections.size();
        projections.push_back(projection);
    }
    vector<Table> &get_tables() { return tables; }
    Table &get_table(string table_name);
    vector<Projection> &get_projections() { return projections; }
    Projection &get_projection(string projection_name);

};
}  // namespace Parser