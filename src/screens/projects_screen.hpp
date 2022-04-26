#pragma once


// extern
#include <nlohmann/json.hpp>




std::tuple<bool, void*, nlohmann::json> projects_screen(nlohmann::json config);
void create_project(const std::string& name,const std::string& folder);
void import_project(const std::string& path);
bool render_create_popup();