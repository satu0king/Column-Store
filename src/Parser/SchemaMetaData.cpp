#include "SchemaMetaData.h"

using namespace std;

Parser::Table Parser::SchemaMetaData::get_table(string table_name) {
    if (table_map.find(table_name) == table_map.end()) {
        TableNotFoundException t_exp(table_name);
        throw t_exp;
    }
    return tables[table_map[table_name]];
}

Parser::Projection Parser::SchemaMetaData::get_projection(string projection_name) {
    if (projection_map.find(projection_name) == projection_map.end()) {
        ProjectionNotFoundException p_exp(projection_name);
        throw p_exp;
    }
    return projections[projection_map[projection_name]];
}