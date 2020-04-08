#include "SchemaLoader.h"
#include "Table.h"

using namespace std;

SchemaLoader::SchemaLoader(string db_name, string username, string password, SchemaMetaData schema_meta_data) : db_name(db_name), username(username), password(password), schema_meta_data(schema_meta_data) {
    try {
        conn = new pqxx::connection("dbname = " + db_name + " user = " + username + " password = " + password + " hostaddr = 127.0.0.1 port = 5432");
        if(conn->is_open()) {
            cout << "Opened DATABASE " << conn->dbname() << " successfully." << endl;
        }
        else {
            throw runtime_error("Can't open the database");
        }

        setup_database();

    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        exit(1);
    }
}

void SchemaLoader::setup_database() {
    create_tables();
}

void SchemaLoader::create_tables() {
    pqxx::work W(*conn);
    vector<Table> tables = schema_meta_data.get_tables();
    for(Table table : tables) {
        string sql = "CREATE TABLE " + table.get_table_name() + "(";
        unordered_map<string, string> columns = table.get_columns();
        for(unordered_map<string, string>::iterator it = columns.begin(); it != columns.end(); it++) {
            sql += (it->first + " " + it->second + ",");
        }

        sql.pop_back();
        sql += ");";
        cout << sql << endl;
        W.exec(sql);

        sql = "ALTER TABLE " + table.get_table_name() + " ADD PRIMARY KEY (" + table.get_primary_key() + ");";
        cout << sql << endl;
        W.exec(sql);
    }
        
    for(Table table : tables) {
        vector<foreign_key> foreign_keys = table.get_foreign_keys();
        for(vector<foreign_key>::iterator it = foreign_keys.begin(); it != foreign_keys.end(); it++) {
            string sql = "ALTER TABLE " + table.get_table_name() + " ADD CONSTRAINT " + \
                table.get_table_name() + "_" + it->from + "_TO_" + it->table + "_" + it->to + " FOREIGN KEY (" + \
                it->from + ") REFERENCES " + it->table + " (" + it->to + ");";
            cout << sql << endl;
            W.exec(sql);
        }
    }

    W.commit();
}

int main() {
    SchemaExtractor schema_extractor("../schema.xml");
    SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
    SchemaLoader schema_loader("column_store", "test", "test", schema_meta_data);
    return 0;
}