#pragma once

// extern
#include <nlohmann/json.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/filesystem.hpp>
#include <glm/vec2.hpp>

// builtin
#include <map>
#include <optional>
#include <functional>
#include <chrono>
#include <filesystem>

// local
#include "utils/conversions.hpp"
#include "utils/log.hpp"



struct ApplicationConfig
{
    struct Project
    {
        struct Header
        {
            std::string name;
            std::filesystem::path path;
            boost::posix_time::ptime last_access;
            glm::u64vec2 window_size;

            friend void to_json(nlohmann::json& json, const Header& header)
            {
                json["name"] = header.name;
                json["path"] = header.path.string();
                json["last_access"] = boost::posix_time::to_simple_string(header.last_access);
            }
            friend void from_json(const nlohmann::json& json, Header& header)
            {
                json.at("name").get_to(header.name);
                header.path = json.at("path").get<std::string>();
                auto time = json.at("last_access").get<std::string>();
                header.last_access = boost::posix_time::time_from_string(time);
            };  
        } header;

        struct Preferences
        {
            int video_resolution[2]{1280,720};

        } preferences;
    };

    struct
    {
        bool vsync;
        uint32_t max_framerate;

    } graphic_config;


    std::vector<Project::Header> projects;
    Project current_project;

    size_t max_history_length;


    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ApplicationConfig, graphic_config.vsync, graphic_config.max_framerate, projects, max_history_length);
};

inline ApplicationConfig config;



inline glm::vec2 get_camera_area()
{
    auto resolution = config.current_project.preferences.video_resolution;
    return glm::vec2{resolution[0], resolution[1]};
}
