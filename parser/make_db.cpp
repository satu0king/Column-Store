// #include "../libs/pugixml-1.10/src/pugixml.hpp"
#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

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
        c.disconnect();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}