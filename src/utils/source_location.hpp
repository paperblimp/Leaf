#pragma once


// builtin
#include <string_view>



struct SourceLocation
{
    uint64_t line;
    std::string_view filename;

    SourceLocation(uint64_t _line, std::string_view _file);

    static SourceLocation get_source_location(uint64_t _line = __builtin_LINE(), std::string_view _file = __builtin_FILE());

    std::string to_string();
};