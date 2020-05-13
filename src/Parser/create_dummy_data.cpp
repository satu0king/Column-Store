#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void create_dummy_data(string file_name) {
    ofstream my_file(file_name);
    int num_cities = 100;
    int num_departments = 50;
    int num_employees = 500000;
    string to_write = "INSERT INTO CITY (id, name) VALUES ";
    for (int i = 1; i <= num_cities; i++) {
        to_write += ("(" + to_string(i) + ",'city_" + to_string(i) + "'),");
    }

    to_write.pop_back();
    to_write += ";\n";
    my_file << to_write;

    to_write = "INSERT INTO DEPARTMENT (id, name) VALUES ";
    for (int i = 1; i <= num_departments; i++) {
        to_write +=
            ("(" + to_string(i) + ",'department_" + to_string(i) + "'),");
    }

    to_write.pop_back();
    to_write += ";\n";
    my_file << to_write;

    to_write =
        "INSERT INTO EMPLOYEE (id, name, age, salary, gender, dept_id, "
        "city_id) VALUES ";
    vector<float> salaries = {20000, 30000, 35000, 40000, 50000};
    for (int i = 1; i <= num_employees; i++) {
        to_write += ("(" + to_string(i) + ",'employee_" + to_string(i) + "'");
        int age = rand() % 30 + 25;
        float salary = salaries[rand() % 5];
        int gender = rand() % 2;
        int dept_id = rand() % num_departments + 1;
        int city_id = rand() % num_cities + 1;
        to_write += ("," + to_string(age) + "," + to_string(salary) + "," +
                     to_string(gender) + "," + to_string(dept_id) + "," +
                     to_string(city_id));
        to_write += "),";
    }

    to_write.pop_back();
    to_write += ";\n";
    my_file << to_write;
}

int main() {
    create_dummy_data("../../data/dummy_data_large.sql");
    return 0;
}