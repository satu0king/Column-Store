#pragma once

#include <string>

namespace Parser {
    struct foreign_key {
        std::string from;
        std::string table;
        std::string to;
    };
}