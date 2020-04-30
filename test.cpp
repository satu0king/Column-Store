#include <iostream>

#include "src/CSVDataGenerator/CSVDataGenerator.h"
#include "src/GenericQuery/GenericGenerator.h"
#include "src/GenericQuery/GenericQueryBuilder.h"
#include "src/interfaces/DataRecord.h"

using namespace ColumnStore;
using namespace GenericQuery;
using namespace CSV;

int main(int argc, char **argv) {
    DataSource sampleCSV = DataSource(new CSVDataSource("../data/sample.csv"));
    DataSource studentCSV =
        DataSource(new CSVDataSource("../data/students.csv"));
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
        // builder.where(Query(new AndQuery(
        //     Query(new LessThanQuery("CGPA", 3.4f)),
        //     Query(new NotQuery(Query(new LessThanQuery("CGPA", 3.2f)))))));

        {
            // builder.groupBy("Name");
            // builder.groupBy("Roll_Number");
            // builder.groupBy("CGPA");
            // builder.aggregate(AggregatorQuery(new AverageAggregator("Score")));
            // builder.aggregate(AggregatorQuery(new MaxAggregator("Score")));
            // builder.aggregate(AggregatorQuery(new MinAggregator("Score")));
        }

        {
            builder.groupBy("Course_Name");
            builder.groupBy("MaxScore");
            builder.aggregate(AggregatorQuery(new AverageAggregator("Score")));
            builder.aggregate(AggregatorQuery(new MaxAggregator("Score")));
            builder.aggregate(AggregatorQuery(new MinAggregator("Score")));
        }

        // builder.aggregate(AggregatorQuery(new MaxAggregator("CGPA")));
        // builder.aggregate(AggregatorQuery(new MinAggregator("CGPA")));

        auto generator = builder.build();

        generator->print();
    }
    return 0;
}