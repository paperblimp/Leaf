#include "section.hpp"
#include "fmt/core.h"

void Section::render_section()
{
    //Call the render function on the child class and set the render position
    if(is_open)
    {

        ImVec2 next_window_size;

        const float CENTER_SLOT_PIVOT = 0.5f;
        //Center slot size
        if(pivot.x == CENTER_SLOT_PIVOT)
        {
            next_window_size = ImVec2
            {
                ImGui::GetIO().DisplaySize.x * CENTER_FACTOR_OCCUPIED_AREA_X,
                (position.y - TOP_MARGIN) * CENTER_FACTOR_OCCUPIED_AREA_Y
            };
        }
        //Sides slot size
        else
        {
            next_window_size = ImVec2
            {
                ImGui::GetIO().DisplaySize.x * FACTOR_OCCUPIED_AREA_X,
                position.y - TOP_MARGIN
            };
        }

        ImGui::SetNextWindowSize(next_window_size);
        ImGui::SetNextWindowPos(position,0,pivot);
        render();
    }
}

void Section::render()
{
    panic("This should not be running. A render function must be defined for all Section subclasses");
}

bool Section::is_section_open()
{
    return is_open;
}

void Section::update_pos(const ImVec2& _position,const ImVec2& _pivot)
{
    position = _position;
    pivot    = _pivot;
}

void Section::open()
{
    is_open = true;
}

void Section::close()
{
    is_open = false;
}