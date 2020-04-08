#include <iostream>
#include <queue>
#include <vector>

#include "CSVparser.hpp"
#include "DataGeneratorInterface.h"
#include "DataRecord.h"
#include "GenericQueryBuilder.h"

// https://github.com/rsylvian/CSVparser

std::string trim(const std::string &str,
                 const std::string &whitespace = " \t\r\n") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

class CSVDataSource : public DataGeneratorInterface {
    std::queue<DataRecord> data;

   public:
    CSVDataSource(std::string filename) {
        csv::Parser file(filename);
        int columnCount = file.columnCount();
        int rowCount = file.rowCount();

        std::vector<Column> columns(columnCount);

        auto header = file.getHeader();
        for (int i = 0; i < columnCount; i++) {
            columns[i].name = trim(header[i]);
            // std::cout << columns[i].name << " " << columns[i].name.size() <<
            // std::endl;
            columns[i].index = i;
        }

        for (int i = 0; i < columnCount; i++) {
            std::string type = trim(std::string(file[0][i]));

            if (type == "INT")
                columns[i].type = DataType::INT;
            else if (type == "STRING")
                columns[i].type = DataType::STRING;
            else if (type == "FLOAT")
                columns[i].type = DataType::FLOAT;
            else
                throw std::runtime_error("Unknown DataType");
        }

        metadata = Metadata(new DataRecordMetadata(columns));

        for (int i = 1; i < rowCount; i++) {
            std::vector<DataValue> values;
            for (int j = 0; j < columnCount; j++) {
                auto type = columns[j].type;
                if (type == DataType::INT)
                    values.push_back(DataValue((int)stoi(file[i][j])));
                else if (type == DataType::FLOAT)
                    values.push_back(DataValue((float)stof(file[i][j])));
                else if (type == DataType::STRING)
                    values.push_back(DataValue(trim(file[i][j])));
            }
            data.push(values);
        }
    }

    DataRecord next() {
        auto d = data.front();
        data.pop();
        return d;
    }

    bool hasNext() { return !data.empty(); }
};

int main(int argc, char **argv) {
    DataSource sampleCSV = DataSource(new CSVDataSource("data/sample.csv"));
    DataSource studentCSV = DataSource(new CSVDataSource("data/students.csv"));
    DataSource gradesCSV = DataSource(new CSVDataSource("data/grades.csv"));

    {
        GenericQueryBuilder builder;

        builder.registerDataSource("base", sampleCSV);
        builder.setBaseSource("base", {"Year", "Make", "Model"});

        builder.where(Query(new LessThanQuery("Year", 1998)));

        GenericDataGenerator generator(builder);

        generator.print();
    }

    {

        GenericQueryBuilder builder;
        builder.registerDataSource("students", studentCSV);
        builder.registerDataSource("grades", gradesCSV);
        builder.setBaseSource("grades", {"Course_Name", "Score", "MaxScore"});

        builder.join("students", {"Id", "Name", "Roll_Number", "CGPA"},
                     "Student_Id", "Id");

        builder.where(Query(new AndQuery(
            Query(new LessThanQuery("CGPA", 3.4f)),
            Query(new NotQuery(Query(new LessThanQuery("CGPA", 3.2f)))))));

        GenericDataGenerator generator(builder);

        generator.print();
    }
    return 0;
}