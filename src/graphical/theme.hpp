#pragma once
#include <imgui.h>
#include <filesystem>
#include <fmt/core.h>



class StyleManager
{
    public:
        ImFont* main_font  = nullptr;
        ImFont* title_font = nullptr;
        ImFont* subtitle_font = nullptr;

        void load_fonts();
        void apply_main_style();
};

inline StyleManager style_manager{};