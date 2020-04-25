#include "PostgreSQLMetaData.h"
#include "PostgreSQLDataGenerator.h"

int main() { 
    SchemaExtractor schema_extractor("../schema.xml");
    SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
    PostgreSQLMetaData postgresql_meta_data("column_store", "test", "test", schema_meta_data);
    vector<string> column_names = {"id", "name", "age1"};
    PostgreSQLDataSource postgresql_data_source(postgresql_meta_data, "EMPLOYEE", column_names);
    DataRecord data_record = postgresql_data_source.next();
    return 0;
}