# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(postgres_data_generator PostgreSQLMetaData.cpp)

target_compile_options(postgres_data_generator PRIVATE -Wall)

target_include_directories(postgres_data_generator 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${LIB_DIR}
)


target_link_libraries(postgres_data_generator PUBLIC parser)
