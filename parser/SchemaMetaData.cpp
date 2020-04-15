#include "SchemaMetaData.h"

using namespace std;

Table SchemaMetaData::get_table(string table_name) {
    assert(table_map.find(table_name) != table_map.end());
    return tables[table_map[table_name]];
}

Projection SchemaMetaData::get_projection(string projection_name) {
    assert(projection_map.find(projection_name) != projection_map.end());
    return projections[projection_map[projection_name]];
}