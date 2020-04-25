# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(csv_data_generator CSVparser.cpp CSVDataGenerator.cpp)

target_compile_options(csv_data_generator PRIVATE -Wall)

target_include_directories(csv_data_generator 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(csv_data_generator PUBLIC fort)
