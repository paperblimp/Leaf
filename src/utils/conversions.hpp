#pragma once


// builtin
#include <string>
#include <chrono>

// extern
#include <nlohmann/json.hpp>



// json steady_clock::time_point conversion
namespace nlohmann
{    
    template <>
    struct adl_serializer<std::chrono::steady_clock::time_point>
    {
        static void to_json(nlohmann::json& json, const std::chrono::steady_clock::time_point& time_point)
        {
            json = time_point.time_since_epoch().count();
        }

        static void from_json(const nlohmann::json& json, std::chrono::steady_clock::time_point& time_point)
        {
            uint64_t nanoseconds = json.get<uint64_t>();
            time_point = std::chrono::steady_clock::time_point{std::chrono::nanoseconds{nanoseconds}};
        }
    };
}
