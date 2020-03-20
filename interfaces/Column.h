#pragma once;

#include <string>

enum class DataType{
    INT,
    FLOAT,
    STRING
};

struct Column{
    std::string name;
    DataType type;
};