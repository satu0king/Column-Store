#include "SchemaLoader.h"

#include "DataLoader.h"
#include "Table.h"
#include "stdlib.h"

using namespace std;

Parser::SchemaLoader::SchemaLoader(string db_name, string username, string password,
                           SchemaMetaData schema_meta_data)
    : db_name(db_name),
      username(username),
      password(password),
      schema_meta_data(schema_meta_data) {
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

        setup_database();

    } catch (const exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

void Parser::SchemaLoader::setup_database() {
    create_tables();
    create_views();
}

void Parser::SchemaLoader::create_tables() {
    pqxx::work W(*conn);
    vector<Table> tables = schema_meta_data.get_tables();
    for (Table table : tables) {
        string sql = "CREATE TABLE " + table.get_table_name() + "(";
        vector<Parser::Column> columns = table.get_columns();
        for (vector<Parser::Column>::iterator it = columns.begin(); it != columns.end();
             it++) {
            sql += (it->name + " ");
            if (it->type.dataType == ColumnStore::DataType::STRING) {
                sql += "varchar";
            } else if (it->type.dataType == ColumnStore::DataType::INT) {
                sql += "int";
            } else if (it->type.dataType == ColumnStore::DataType::FLOAT) {
                sql += "float";
            }

            sql += ",";
        }

        sql.pop_back();
        sql += ");";
        cout << sql << endl;
        W.exec(sql);

        sql = "ALTER TABLE " + table.get_table_name() + " ADD PRIMARY KEY (" +
              table.get_primary_key() + ");";
        cout << sql << endl;
        W.exec(sql);
    }

    for (Table table : tables) {
        vector<foreign_key> foreign_keys = table.get_foreign_keys();
        for (vector<foreign_key>::iterator it = foreign_keys.begin();
             it != foreign_keys.end(); it++) {
            string sql = "ALTER TABLE " + table.get_table_name() +
                         " ADD CONSTRAINT " + table.get_table_name() + "_" +
                         it->from + "_TO_" + it->table + "_" + it->to +
                         " FOREIGN KEY (" + it->from + ") REFERENCES " +
                         it->table + " (" + it->to + ");";
            cout << sql << endl;
            W.exec(sql);
        }
    }

    W.commit();
}

void Parser::SchemaLoader::create_views() {
    pqxx::work W(*conn);
    vector<Projection> projections = schema_meta_data.get_projections();
    for (Projection projection : projections) {
        string sql = "CREATE VIEW " + projection.get_projection_name() + " (";
        for (projection_column col : projection.get_columns()) {
            sql += (col.name + ",");
        }

        sql.pop_back();
        sql += ") AS SELECT ";

        for (projection_column col : projection.get_columns()) {
            sql += (col.table + "." + col.column_name + ",");
        }

        sql.pop_back();
        sql += " FROM " + projection.get_base_table();

        for (foreign_key join_tab : projection.get_join_tables()) {
            sql += " INNER JOIN " + join_tab.table + " ON " +
                   projection.get_base_table() + "." + join_tab.from + " = " +
                   join_tab.table + "." + join_tab.to;
        }

        sql += ";";
        cout << sql << endl;
        W.exec(sql);
    }
    W.commit();
}

// int main() {
//     SchemaExtractor schema_extractor("../schema.xml");
//     SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
//     SchemaLoader schema_loader("column_store", "test", "test",
//     schema_meta_data); DataLoader data_loader("column_store", "test", "test",
//     "dummy_data.sql"); return 0;
// }
