#include <iostream>


#include "src/interfaces/DataRecord.h"
#include "GenericQueryBuilder.h"
#include "GenericGenerator.h"
#include "src/CSVDataGenerator/CSVDataGenerator.h"

using namespace ColumnStore;
using namespace GenericQuery;
using namespace CSV;

int main(int argc, char **argv) {
    DataSource sampleCSV = DataSource(new CSVDataSource("../data/sample.csv"));
    DataSource studentCSV = DataSource(new CSVDataSource("../data/students.csv"));
    DataSource gradesCSV = DataSource(new CSVDataSource("../data/grades.csv"));

    {
        GenericQueryBuilder builder;

        builder.registerDataSource("base", sampleCSV);
        builder.setBaseSource("base", {"Year", "Make", "Model"});

        builder.where(Query(new LessThanQuery("Year", 1998)));

        auto generator = builder.build();

        generator->print();
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

        auto generator = builder.build();

        generator->print();
    }
    return 0;
}