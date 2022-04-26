#pragma once

// extern
#include <GLFW/glfw3.h>

// builtin
#include <array>



inline std::array<bool, GLFW_KEY_LAST> key_map{};

inline bool key_map_get_ctrl()
{
    return key_map[GLFW_KEY_LEFT_CONTROL] | key_map[GLFW_KEY_RIGHT_CONTROL];
}
