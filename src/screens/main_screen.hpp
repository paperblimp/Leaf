#pragma once


// local
#include "config.hpp"
#include "graphical/graphics.hpp"



std::tuple<bool, void*, nlohmann::json> main_screen(nlohmann::json config);
