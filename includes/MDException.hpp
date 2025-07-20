#pragma once
#include <stdexcept>
#include <string>

namespace MD {
    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string &msg) : std::runtime_error(msg) {}
    };
}
