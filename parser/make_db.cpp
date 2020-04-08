#include "../libs/pugixml-1.10/src/pugixml.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include <string.h>

using namespace std;

void create_db(pqxx::connection &c);

int main() {
    try {
        pqxx::connection c("dbname = column_store user = test password = test hostaddr = 127.0.0.1 port = 5432");
        if(c.is_open()) {
            cout << "Opened the database successfully: " << c.dbname() << endl;
            create_db(c);
        }
        else {
            cout << "Can't open the database" << endl;
            return 1;
        }
        c.disconnect();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

void create_db(pqxx::connection &c) {
    string table_name = "test";
    string sql = "CREATE TABLE " + table_name + " (ID INT PRIMARY KEY NOT NULL);";
    pqxx::work W(c);
    W.exec(sql);
    W.commit();
    cout << "Table " + table_name + " created successfully" << endl;
}