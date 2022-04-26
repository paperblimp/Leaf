#pragma once

#ifdef _WIN32
    #include <windows.h>
#endif

// extern
#include <glad/gl.h>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// local
#include "config.hpp"

// builtin
#include <optional>


inline void glfw_error_callback(int error, const char* description)
{
    panic(fmt::format("GLFW error: {}:{}", error, description));
}

class GraphicContext
{
    public:

        GLFWwindow* window = nullptr;
        GLFWwindow* export_window = nullptr;
        std::optional<std::string_view> glsl_version;

        ImGuiIO* imgui_io = nullptr;
        
        bool initialized = false;

    public:

        void init();
        void destroy();

        void start_frame();
        void end_frame(ImVec4 clear_color);
        void display_frame();

        void make_current_main_context();
        void make_current_export_context();

        ~GraphicContext();
};

inline GraphicContext graphic_context;