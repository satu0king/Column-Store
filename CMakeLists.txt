cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(ColumnStore VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

find_package(Boost 1.72.0 COMPONENTS program_options REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
# find_package(Catch2 REQUIRED)

add_subdirectory(src/PostgresDataGenerator)
add_subdirectory(src/Parser)
add_subdirectory(src/CSVDataGenerator)
add_subdirectory(src/Fort)
add_subdirectory(src/GenericQuery)
add_subdirectory(src/interfaces)
# add_subdirectory(src/ColumnStoreInitializer)
# add_subdirectory(src/gossip)
# add_subdirectory(src/membership_list)

add_executable(schema_loader src/Parser/SchemaLoaderTester.cpp)
target_link_libraries(schema_loader PRIVATE parser)

add_executable(postgres_data_test src/PostgresDataGenerator/PostgreSQLDataGeneratorTest.cpp)
target_link_libraries(postgres_data_test PRIVATE postgres_data_generator)

add_executable(main_test test.cpp)
target_link_libraries(main_test PRIVATE csv_data_generator generic_query interfaces)

add_executable(column_store_init src/ColumnStoreInitializer/Driver.cpp)
target_link_libraries(column_store_init PRIVATE parser Boost::program_options nlohmann_json::nlohmann_json)

# add_executable(cache_server src/cache/cache_server.cpp)
# target_link_libraries(cache_server PRIVATE Boost::program_options cache basic_data_connector postgres_data_connector gossip membership_list)

# add_executable(cache_policy_lru_test src/cache/LRU_cache_policy_test.cpp)
# target_link_libraries(cache_policy_lru_test PRIVATE cache Catch2::Catch2)

# add_executable(mock_database src/mock_database.cpp)
# target_link_libraries(mock_database PRIVATE Boost::program_options)