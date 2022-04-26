#pragma once

//extern
#include <imgui.h>
#include <fmt/core.h>


//local
#include "utils/log.hpp"
#include "graphical/graphics.hpp"

class Section
{

    const uint8_t TOP_MARGIN = 50;
    const float FACTOR_OCCUPIED_AREA_X = 1/4.5;

    const float CENTER_FACTOR_OCCUPIED_AREA_X = 1/2.0;
    const float CENTER_FACTOR_OCCUPIED_AREA_Y = 1/3.5;

    private:
        bool is_open = false;
        ImVec2 position{0,0};
        ImVec2 pivot   {0,0};


    public:

        const ImGuiWindowFlags SECTION_FLAGS = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

        virtual void render();
        void render_section();

        bool is_section_open();
        void update_pos(const ImVec2& _position,const ImVec2& _pivot);

        void open();
        void close();


};