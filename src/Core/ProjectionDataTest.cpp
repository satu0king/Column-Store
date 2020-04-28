#include <iostream>

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
}
