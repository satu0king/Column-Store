#include "DataLoader.h"

using namespace std;

DataLoader::DataLoader(string db_name, string username, string password, string sql_file_name) : db_name(db_name), username(username), password(password), sql_file_name(sql_file_name) {
    try {
        conn = new pqxx::connection("dbname = " + db_name + " user = " + username + " password = " + password + " hostaddr = 127.0.0.1 port = 5432");
        if(conn->is_open()) {
            cout << "Opened DATABASE " << conn->dbname() << " successfully." << endl;
        } else {
            throw runtime_error("Can't open the database");
        }

        load_data();

    } catch (const exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

void DataLoader::load_data() {
    pqxx::work W(*conn);
    string line;
    try {
        ifstream myfile(sql_file_name); 
        if (myfile.is_open()) {
                while(getline(myfile, line)) {
                W.exec(line);
            }

            myfile.close();
        } else {
            throw runtime_error("Unable to open file " + sql_file_name);
        }
    } catch (const exception &e) {
        cerr << e.what() << endl;
        exit(1);
    }

    cout << "Loaded Data successfully." << endl;
    W.commit();
}