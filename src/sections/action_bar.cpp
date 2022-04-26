#include "action_bar.hpp"
#include "graphical/graphics.hpp"
#include "graphical/sprite.hpp"
#include "utils/log.hpp"
#include "utils/math_utils.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <imgui.h>


SectionButton::SectionButton(ButtonName name,std::string icon,std::unordered_map<Slot,Section*> attached_sections)
    :name{name},icon{icon},attached_sections{std::move(attached_sections)}
    {
    }      

ActionBar::ActionBar(std::unordered_map<ButtonName,SectionButton> action_buttons)
    : action_buttons{std::move(action_buttons)}
    {
    }

void ActionBar::set_style()
{
    auto& style    = ImGui::GetStyle();
    style.WindowRounding = ACTION_BAR_ROUNDING_FACTOR;


    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(1.0f,1.0f,1.0f,1.0f);
    colors[ImGuiCol_Button]   = ImVec4(1.0f,1.0f,1.0f,1.0f);
};

void ActionBar::handle_time_slider_clicks(const ImVec2 indicator_pos,const ImVec2 start,const ImVec2 end)
{    
    
    const ImVec2 click_area_start
    {
        start.x,
        start.y - (BALL_RADIUS-TIME_SLIDER_SIZE.y)/2
    };
    const ImVec2 click_area_end
    {
        end.x,
        end.y + (BALL_RADIUS-TIME_SLIDER_SIZE.y)/2
    };


    if(ImGui::IsMouseClicked(0) && is_inside_rect(ImGui::GetMousePos(),click_area_start,click_area_end))
    {
        is_sliding_time = true;
    }

    else if(ImGui::IsMouseReleased(0))
    {
        is_sliding_time = false;
    }

    if(is_sliding_time)
    {
        //Timeslider circle click
        const double click_time = normalize( ImGui::GetMousePos().x, start.x, end.x ) * anim_data.length ; 

        anim_data.set_time(click_time);

    }
}

void ActionBar::render_time_slider()
{
    ImGui::SameLine();

    const ImVec2 win_pos = ImGui::GetWindowPos();
    const ImVec2 cursor_pos = ImGui::GetCursorPos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    //Timeline bar
    const ImVec2 start
    {
        win_pos.x + cursor_pos.x + TIME_SLIDER_HORIZONTAL_MARGIN,
        win_pos.y + cursor_pos.y +(float)TIME_SLIDER_VERTICAL_MARGIN
    };
    const ImVec2 end
    {
        win_pos.x + cursor_pos.x + TIME_SLIDER_HORIZONTAL_MARGIN + TIME_SLIDER_SIZE.x,
        win_pos.y + cursor_pos.y + (float)TIME_SLIDER_VERTICAL_MARGIN + TIME_SLIDER_SIZE.y
    };

    draw_list->AddRectFilled(start, end,TIME_SLIDER_COLOR,12.0f);
    

    //Timeline circular indicator
    const double pos_in_time = normalize(anim_data.get_time(),0.0, anim_data.length) * TIME_SLIDER_SIZE.x;
    const ImVec2 indicator_pos
    {
        start.x +(float)pos_in_time,
        start.y + TIME_SLIDER_SIZE.y/2
    };

    draw_list->AddCircleFilled(indicator_pos, BALL_RADIUS, INTERNAL_BALL_COLOR);
    draw_list->AddCircle(indicator_pos, BALL_RADIUS, TIME_SLIDER_COLOR);

    handle_time_slider_clicks(indicator_pos,start,end);


    //Dummy with the size of time slider in order to allign itens after it
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2*TIME_SLIDER_HORIZONTAL_MARGIN);
    ImGui::Dummy(TIME_SLIDER_SIZE);


}

void ActionBar::render_time_controls(const ImVec2 action_bar_size)
{   
    const int BUTTON_MARGIN = ImGui::GetStyle().ItemSpacing.x * 2;
    const int NUM_BUTTONS = 4;
    const int time_controls_size_x = 
        (NUM_BUTTONS*ICON_SIZE.x) + (NUM_BUTTONS+1)*BUTTON_MARGIN + TIME_SLIDER_SIZE.x + (2*TIME_SLIDER_HORIZONTAL_MARGIN);

    const int time_controls_start_pos_x = action_bar_size.x/2 - time_controls_size_x/2;

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(time_controls_start_pos_x-ImGui::GetCursorPosX(),0));

    if(render_icon_button(BACKWARD))
    {
        anim_data.move_time(-1);
    }

    if(render_icon_button(SAVE_KEYS) && !node_tree->selected_nodes.empty())
    {
        node_tree->selected_nodes[0]->save_all_properties(anim_data.get_time());
    }

    ImGui::SetCursorPosX(action_bar_size.x/2 - time_controls_size_x/2);
    render_time_slider();

    if(anim_data.paused) 
    {
        if(render_icon_button(PLAY))anim_data.paused = false;
    }
    else
    {
        if(render_icon_button(PAUSE)) anim_data.paused = true;
    }


    if(render_icon_button(FORWARD))
    {
        anim_data.move_time(+1);
    }


}

bool ActionBar::render_icon_button(const ButtonName section)
{
    //Render a image button with the given icon and return if it was pressed
    ImTextureID assets_id = (void*)(uintptr_t)sprite_manager.get_sprite(button_icons[section].string()).id.value();
    ImGui::SameLine();
    return ImGui::ImageButton(assets_id, ICON_SIZE);
}

void ActionBar::render_right_buttons(const ImVec2 action_bar_size)
{
    const int BUTTON_MARGIN = ImGui::GetStyle().ItemSpacing.x * 2;
    const int NUM_BUTTONS = 4;
    const int right_buttons_size = 
        (4*ICON_SIZE.x) + (4+1)*BUTTON_MARGIN;

    const int right_buttons_start_pos_x = action_bar_size.x - right_buttons_size;

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(right_buttons_start_pos_x-ImGui::GetCursorPosX(),0));

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,(IM_COL32(255,0,0,255)));
    if (render_icon_button(ButtonName::PRINT))
        this->frame_export_dialog.open();
    ImGui::PopStyleColor();

    this->frame_export_dialog.run();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (render_icon_button(ButtonName::EXPORT))
        this->export_dialog = ExportDialog{anim_data.length};
    
    if (this->export_dialog.has_value())
        if (this->export_dialog->run() == false)
            this->export_dialog = std::nullopt;


    ImGui::SameLine();
    if (render_icon_button(ButtonName::UNDO))
        history->undo();

    ImGui::SameLine();
    if (render_icon_button(ButtonName::REDO))
        history->redo();
}

void ActionBar::update_sections_position(const ImVec2 action_bar_pos,const ImVec2 action_bar_size)
{

    if(screen_slots[LEFT_SLOT].has_value())
    {
        const ImVec2 pivot{0,1.0f};
        const ImVec2 slot_pos
        {
            action_bar_pos.x + ACTION_BAR_ROUNDING_FACTOR,
            action_bar_pos.y
        };

        screen_slots[LEFT_SLOT].value()->attached_sections[LEFT_SLOT]->update_pos(slot_pos,pivot);
    }
    if(screen_slots[RIGHT_SLOT].has_value())
    {
        const ImVec2 pivot{1.0f,1.0f};
        const ImVec2 slot_pos
        {
            action_bar_pos.x + action_bar_size.x - ACTION_BAR_ROUNDING_FACTOR,
            action_bar_pos.y 
        };

        screen_slots[RIGHT_SLOT].value()->attached_sections[RIGHT_SLOT]->update_pos(slot_pos,pivot);
    }
    if(screen_slots[CENTER_SLOT].has_value())
    {
        const ImVec2 pivot{0.5f,1.0f};
        const ImVec2 slot_pos
        {
            action_bar_pos.x + action_bar_size.x/2,
            action_bar_pos.y 
        };

       screen_slots[CENTER_SLOT].value()->attached_sections[CENTER_SLOT]->update_pos(slot_pos,pivot);
    }
}

void ActionBar::close_sections_from_button(SectionButton* section_button)
{
    //Close the given section and remove the reference from the screen slot
    for(auto attached_section : section_button->attached_sections)
    {
        attached_section.second->close();
        screen_slots[attached_section.first] = std::nullopt;  
    }
    section_button->pressed = false;
}

void ActionBar::open_sections_from_button(SectionButton* section_button)
{
    for(auto attached_section : section_button->attached_sections)
    {
        attached_section.second->open();
        screen_slots[attached_section.first] = std::make_optional<SectionButton*>(section_button);
    }
    section_button->pressed = true;
}

void ActionBar::render_section_button(SectionButton* section_button)
{
    //Render the button of a given section and handle clicks


    if(render_icon_button(section_button->name))
    {
        //Close all attached sections if one was open
        if(section_button->pressed)
        {
            close_sections_from_button(section_button);
        }
        else
        {
            //Close the sections attached to slots this button occupies
            for (auto attached_section : section_button->attached_sections)
            {
                if(screen_slots[attached_section.first].has_value())
                {
                    SectionButton* open_section_button = screen_slots[attached_section.first].value();
                    close_sections_from_button(open_section_button);
                }
            }

            //Then open the button
            open_sections_from_button(section_button);
        }
    }
}

void ActionBar::render()
{
    auto old_style = ImGui::GetStyle();
    set_style();



    //Create this window on the bottom of the screen with the given size
    const ImVec2 action_bar_size
    {
        (float)ImGui::GetIO().DisplaySize.x - ACTION_BAR_WINDOW_HORIZONTAL_MARGIN*2,
        (float) ACTION_BAR_HEIGHT
    };

    const ImVec2 action_bar_pos
    {
        (float)ACTION_BAR_WINDOW_HORIZONTAL_MARGIN,
        (float)ImGui::GetIO().DisplaySize.y - ACTION_BAR_WINDOW_VERTICAL_MARGIN - ACTION_BAR_HEIGHT
    };

    ImGui::SetNextWindowSize(action_bar_size);
    ImGui::SetNextWindowPos (action_bar_pos);

    ImGui::Begin("Action Bar",NULL,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    //Section buttons
    render_section_button(&action_buttons[ASSETS]);

    render_section_button(&action_buttons[LAYERS]);
    render_section_button(&action_buttons[ANIMATION_CONTROL]);



    render_time_controls(action_bar_size);

    render_right_buttons(action_bar_size);

    //Section position
    update_sections_position(action_bar_pos,action_bar_size);




    ImGui::End();

    auto& style = ImGui::GetStyle(); 
    style = old_style;
}