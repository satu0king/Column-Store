# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(parser SchemaLoader.cpp SchemaExtractor.cpp SchemaMetaData.cpp Table.cpp Projection.cpp DataLoader.cpp create_dummy_data.cpp)

target_compile_options(parser PRIVATE -Wall)

target_include_directories(parser 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR} ${LIB_DIR}
    /usr/local/include
)

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

target_link_libraries(parser PUBLIC ${PQXX_LIB} ${PQ_LIB})
