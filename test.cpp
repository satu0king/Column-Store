#include <iostream>

#include "src/Complete.h"

using namespace ColumnStore;
using namespace GenericQuery;
using namespace CSV;

int main(int argc, char **argv) {
    auto getTime = [](){return std::chrono::high_resolution_clock::now();};
    auto calculate = [](auto start, auto end){return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();};

    DataSource sampleCSV = DataSource(new CSVDataSource("../data/sample.csv"));
    DataSource studentCSV =
        DataSource(new CSVDataSource("../data/students.csv"));
    DataSource gradesCSV = DataSource(new CSVDataSource("../data/grades.csv"));

    DataSource employee_city = DataSource(new ColumnStore::ColStoreDataSource(
        "../store", "EMPLOYEE_CITY_PROJECTION"));

    DataSource projection1 = DataSource(new ColumnStore::ColStoreDataSource(
        "../store2", "PROJECTION1"));
    DataSource employee_department =
        DataSource(new ColumnStore::ColStoreDataSource(
            "../store", "EMPLOYEE_DEPARTMENT_PROJECTION"));
;

    Parser::SchemaExtractor schema_extractor("../store/schema.xml");
    Parser::SchemaMetaData schema_meta_data = schema_extractor.get_meta_data();
    Postgres::PostgreSQLMetaData postgresql_meta_data("column_store", "test",
                                                      "test", schema_meta_data);
    vector<std::string> column_names = {"emp_id", "age", "city_name"};
    auto postgresql_data_source = DataSource(new Postgres::PostgreSQLDataSource(
        postgresql_meta_data, "EMPLOYEE_CITY_PROJECTION", column_names));

    {
        // GenericQueryBuilder builder;
        // builder.registerDataSource("base", employee_city);
        // builder.setBaseSource("base", {"age"});
        // builder.aggregate(AggregatorQuery(new AverageAggregator("age")));
       
        // auto s = getTime();
        // auto generator = builder.build();
        // generator->print();
        // auto e = getTime();
        // std::cout << calculate(s, e) << std::endl;
    }
    {
        GenericQueryBuilder builder;
        builder.registerDataSource("base", projection1);
        builder.setBaseSource("base", {"col0"});
        builder.aggregate(AggregatorQuery(new AverageAggregator("col0")));
       
        auto s = getTime();
        auto generator = builder.build();
        generator->print();
        auto e = getTime();
        std::cout << calculate(s, e) << std::endl;
    }
    // {
    //     GenericQueryBuilder builder;

    //     builder.registerDataSource("base", sampleCSV);
    //     builder.setBaseSource("base", {"Year", "Make", "Model"});

    //     builder.where(Query(new LessThanQuery("Year", 1998)));

    //     auto generator = builder.build();

    //     generator->print();
    // }

    // {
    //     GenericQueryBuilder builder;
    //     builder.registerDataSource("students", studentCSV);
    //     builder.registerDataSource("grades", gradesCSV);
    //     builder.setBaseSource("grades", {"Course_Name", "Score",
    //     "MaxScore"});

    //     builder.join("students", {"Id", "Name", "Roll_Number", "CGPA"},
    //                  "Student_Id", "Id");
    //     // builder.where(Query(new AndQuery(
    //     //     Query(new LessThanQuery("CGPA", 3.4f)),
    //     //     Query(new NotQuery(Query(new
    //     LessThanQuery("CGPA", 3.2f)))))));

    //     {
    //         // builder.groupBy("Name");
    //         // builder.groupBy("Roll_Number");
    //         // builder.groupBy("CGPA");
    //         // builder.aggregate(AggregatorQuery(new
    //         // AverageAggregator("Score")));
    //         // builder.aggregate(AggregatorQuery(new
    //         MaxAggregator("Score")));
    //         // builder.aggregate(AggregatorQuery(new
    //         MinAggregator("Score")));
    //     }

    //     {
    //         builder.groupBy("Course_Name");
    //         builder.groupBy("MaxScore");
    //         builder.aggregate(AggregatorQuery(new
    //         AverageAggregator("Score")));
    //         builder.aggregate(AggregatorQuery(new MaxAggregator("Score")));
    //         builder.aggregate(AggregatorQuery(new MinAggregator("Score")));
    //     }

    //     // builder.aggregate(AggregatorQuery(new MaxAggregator("CGPA")));
    //     // builder.aggregate(AggregatorQuery(new MinAggregator("CGPA")));

    //     auto generator = builder.build();

    //     generator->print();
    // }
    // {
    //     employee_city->print(10);
    //     employee_department->print(10);
    // }
    return 0;
}