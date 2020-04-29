#include "PostgreSQLMetaData.h"

#include "PostgreSQLDataGenerator.h"

using namespace std;

using Postgres::PostgreSQLMetaData;

PostgreSQLMetaData::PostgreSQLMetaData(string db_name, string username,
                                       string password, SchemaMetaData schema) {
    schema_meta_data = schema;
    try {
        conn = new pqxx::connection(
            "dbname = " + db_name + " user = " + username +
            " password = " + password + " hostaddr = 127.0.0.1 port = 5432");
        if (conn->is_open()) {
            cout << "Opened DATABASE " << conn->dbname() << " successfully."
                 << endl;
        } else {
            throw runtime_error("Can't open the database");
        }
    } catch (const exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

// int main() {
//     SchemaExtractor schema_extractor("../schema.xml");
//     SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
//     PostgreSQLMetaData postgresql_meta_data("column_store", "test", "test",
//     schema_meta_data); vector<string> column_names = {"id", "name", "age1"};
//     PostgreSQLDataSource postgresql_data_source(postgresql_meta_data,
//     "EMPLOYEE", column_names); DataRecord data_record =
//     postgresql_data_source.next(); return 0;
// }