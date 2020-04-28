#include <iostream>

#include "PostgresDataGenerator/PostgreSQLDataGenerator.h"
#include "PostgresDataGenerator/PostgreSQLMetaData.h"
#include "ProjectionData.h"

using namespace ColumnStore;

int main() {
    ColumnStoreData data(16);

    // Testing ColumnStoreData setters
    data.setInt(0, 26);
    data.setFloat(4, 50.235);
    data.setString(8, "Hello");

    // Testing ColumnStoreData getters
    std::cout << data.getInt(0) << std::endl;
    std::cout << data.getFloat(4) << std::endl;
    std::cout << data.getString(8, 8) << std::endl;

    // Testing Save Operation type 1
    {
        std::ofstream wf("temp.dat", std::ios::out | std::ios::binary);
        wf << data;
        wf.close();
    }

    // Testing read operation type 1
    ColumnStoreData newData(16);
    {
        std::ifstream rf("temp.dat", std::ios::out | std::ios::binary);

        rf >> newData;
        std::cout << newData.getInt(0) << std::endl;
        std::cout << newData.getFloat(4) << std::endl;
        std::cout << newData.getString(8, 8) << std::endl;
    }

    std::vector<Parser::Column> columns = {{
        Parser::Column("integer column", "int"),
        Parser::Column("float column", "float"),
        Parser::Column("string column", "string8"),
    }};

    // Testing ColumnStoreData -> DataRecord
    DataRecord record = newData.get(columns);

    std::cout << record[0].as<int>() << std::endl;
    std::cout << record[1].as<float>() << std::endl;
    std::cout << record[2].as<std::string>() << std::endl;

    record[0] = 69;
    record[1] = 3.4f;
    record[2] = std::string("World");

    // Testing DataRecord -> ColumnStoreData
    data.set(record, columns);

    std::cout << data.getInt(0) << std::endl;
    std::cout << data.getFloat(4) << std::endl;
    std::cout << data.getString(8, 8) << std::endl;

    // Testing Write type 2
    std::ofstream wf("temp.dat", std::ios::out | std::ios::binary);
    data.write(wf);
    wf.close();

    // Testing Read type 2
    std::ifstream rf("temp.dat", std::ios::out | std::ios::binary);

    newData.read(rf);
    std::cout << newData.getInt(0) << std::endl;
    std::cout << newData.getFloat(4) << std::endl;
    std::cout << newData.getString(8, 8) << std::endl;

    // Creating Projection Test

    std::vector<Parser::Column> projection_columns = {{
        Parser::Column("emp_id", "int"),
        Parser::Column("fage", "int"),
        Parser::Column("city_name", "string8"),
    }};

    ColumnStoreData projection_data(projection_columns);
    MetadataManager manager("../store");

    PostgreSQLMetaData postgresql_meta_data("column_store", "test", "test",
                                            manager);
    vector<string> column_names = {"emp_id", "age", "city_name"};
    PostgreSQLDataSource postgresql_data_source(
        postgresql_meta_data, "EMPLOYEE_CITY_PROJECTION", column_names);
    // postgresql_data_source.print(10);

    fs::path file =
        manager.getProjectionFileInfo("EMPLOYEE_CITY_PROJECTION")["file"];

    std::ofstream o(file);
    int c = 2;
    while (postgresql_data_source.hasNext()) {
        {
        auto record = postgresql_data_source.next();
        projection_data.set(record, projection_columns);
        projection_data.write(o);
        }
    }
    o.close();

    // Reading Projection Test 

    std::ifstream i(file);
    while(i >> projection_data) {
        std::cout << projection_data.getInt(0) << "\t" 
        << projection_data.getInt(4) << "\t"
        << projection_data.getString(8, 8) << "\n";
    }


}
