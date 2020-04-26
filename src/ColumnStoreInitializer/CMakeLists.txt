# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(column_store_init Initializer.cpp)

target_compile_options(column_store_init PRIVATE -Wall)

target_include_directories(column_store_init 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${LIB_DIR}
)

target_link_libraries(column_store_init PUBLIC parser)
