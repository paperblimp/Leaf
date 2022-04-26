#pragma once

// local
#include "utils/source_location.hpp"

// builtin
#include <iostream>

// extern
#include <fmt/core.h>
#include <fmt/format.h>
#include <termcolor/termcolor.hpp>
#include <glm/vec2.hpp>


template<>
struct fmt::formatter<glm::vec2>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec2 v, FormatContext &ctx)
    {
        return format_to(ctx.out(), "x: {}, y: {}", v.x, v.y);
    }
};

namespace Log
{
    void print_log(const std::string& message, SourceLocation location, const std::string& log_type, std::ostream&(*color)(std::ostream&));
}

[[noreturn]]
void panic(const std::string& message, SourceLocation location = SourceLocation::get_source_location());
void warn(const std::string& message, SourceLocation location = SourceLocation::get_source_location());
void notice(const std::string& message, SourceLocation location = SourceLocation::get_source_location());