#pragma once
#include <string>
#include <cstddef>

namespace cppx86 {

struct SourceLocation {
    std::string file;
    int line = 1;
    int column = 1;
    std::size_t offset = 0;

    SourceLocation() = default;
    SourceLocation(std::string file, int line, int column, std::size_t offset)
        : file(std::move(file)), line(line), column(column), offset(offset) {}
};

} // namespace cppx86
