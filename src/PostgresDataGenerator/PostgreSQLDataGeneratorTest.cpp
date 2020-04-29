#include "PostgreSQLDataGenerator.h"
#include "PostgreSQLMetaData.h"

int main() {
    Parser::SchemaExtractor schema_extractor("../schema.xml");
    Parser::SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
    Postgres::PostgreSQLMetaData postgresql_meta_data("column_store", "test", "test",
                                            schema_meta_data);
    vector<std::string> column_names = {"id", "name", "age"};
    Postgres::PostgreSQLDataSource postgresql_data_source(postgresql_meta_data,
                                                "EMPLOYEE", column_names);
    postgresql_data_source.advance(4900);
    postgresql_data_source.print();
    return 0;
}