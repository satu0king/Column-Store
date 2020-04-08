#include "../libs/pugixml-1.10/src/pugixml.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include <string.h>

using namespace std;

void create_table(pqxx::connection &c);
void parse_xml(const char* file_path);

int main() {
    try {
        pqxx::connection c("dbname = column_store user = test password = test hostaddr = 127.0.0.1 port = 5432");
        if(c.is_open()) {
            cout << "Opened the database successfully: " << c.dbname() << endl;
        }
        else {
            cout << "Can't open the database" << endl;
            return 1;
        }

        parse_xml("../schema.xml");

        c.disconnect();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

void parse_xml(const char* file_path) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(file_path);
    if (!result) {
        throw "XML file not found.";
    }
    
    pugi::xpath_node_set tables = doc.select_nodes("/schema/tables/table/table_name");

    for (pugi::xpath_node table : tables) {
        pugi::xml_node tool = table.node();
        cout << tool.child_value() << endl;
    }
}

void create_table(pqxx::connection &c) {
    string table_name = "test";
    string sql = "CREATE TABLE " + table_name + " (ID INT PRIMARY KEY NOT NULL);";
    pqxx::work W(c);
    W.exec(sql);
    W.commit();
    cout << "Table " + table_name + " created successfully" << endl;
}