# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(generic_query GenericGenerator.cpp GenericQueryBuilder.cpp)

target_compile_options(generic_query PRIVATE -Wall)

target_include_directories(generic_query 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}  
    ${CMAKE_CURRENT_SOURCE_DIR}/.. 
)

target_link_libraries(generic_query PUBLIC fort)
