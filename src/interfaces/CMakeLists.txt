# g++ -std=c++17 *.cpp -lpqxx -lpq

add_library(interfaces DataGeneratorInterface.cpp)

target_compile_options(interfaces PRIVATE -Wall)

target_include_directories(interfaces 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(interfaces PUBLIC fort)
