#pragma once

#include <string>

namespace Parser {
    struct join_index {
        std::string from;
        std::string projection;
        std::string to;
    };
}