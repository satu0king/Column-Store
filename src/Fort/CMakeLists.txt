# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(fort fort.c)
target_compile_options(fort PRIVATE -Wall)
