// header
#include "source_location.hpp"

// extern
#include <fmt/core.h>



SourceLocation::SourceLocation(uint64_t _line, std::string_view _file): line(_line), filename(_file) {};

SourceLocation SourceLocation::get_source_location(uint64_t _line, std::string_view _file)
{
    return {_line, _file};
}

std::string SourceLocation::to_string()
{
    return fmt::format("{}:{}", this->filename, this->line);
}

