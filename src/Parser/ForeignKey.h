#pragma once

#include <string>

struct foreign_key {
    std::string from;
    std::string table;
    std::string to;
};