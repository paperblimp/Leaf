#pragma once

#include <filesystem>
#include <stdint.h>
#include <unordered_map>

#include <imgui.h>
#include <fmt/core.h>
#include <boost/dll.hpp>

#include "graphical/graphics.hpp"
#include "graphical/sprite.hpp"
#include "utils/math_utils.hpp"
#include "sections/keyframe_widget.hpp"
#include "sections/section.hpp"
#include "history.hpp"
#include "dialogs/export.hpp"
#include "dialogs/frame_export.hpp"


enum Slot{LEFT_SLOT,RIGHT_SLOT,CENTER_SLOT};
enum ButtonName{ASSETS,LAYERS,ANIMATION_CONTROL,BACKWARD,FORWARD,PAUSE,PLAY,SAVE_KEYS,EXPORT,PRINT,UNDO,REDO};

struct SectionButton
{
    ButtonName name;
    std::filesystem::path icon;
    std::unordered_map<Slot,Section*> attached_sections;
    
    bool pressed = false;

    SectionButton(){};
    SectionButton(ButtonName name,std::string icon,std::unordered_map<Slot,Section*> attached_sections);
};

class ActionBar
{

    const ImVec2 ICON_SIZE{40,40};
    const ImVec2 TIME_SLIDER_SIZE{400,15};

    const uint8_t  ACTION_BAR_ROUNDING_FACTOR = 12;
    const uint16_t ACTION_BAR_HEIGHT = 70;
    const uint16_t ACTION_BAR_WINDOW_HORIZONTAL_MARGIN = 20;
    const uint16_t ACTION_BAR_WINDOW_VERTICAL_MARGIN = 10;

    const uint16_t TIME_SLIDER_VERTICAL_MARGIN = (uint16_t)(ICON_SIZE.y - TIME_SLIDER_SIZE.y)/2;
    const uint16_t TIME_SLIDER_HORIZONTAL_MARGIN = 20;
    const uint16_t BALL_RADIUS = 19;


    const ImU32 TIME_SLIDER_COLOR = IM_COL32(76,77,86,255);
    const ImU32 INTERNAL_BALL_COLOR = IM_COL32(255,255,255,255);

    std::unordered_map<Slot,std::optional<SectionButton*>> screen_slots
    {
        {LEFT_SLOT  ,std::nullopt},
        {RIGHT_SLOT ,std::nullopt},
        {CENTER_SLOT,std::nullopt},
    };

    std::unordered_map<ButtonName,SectionButton> action_buttons;

    std::filesystem::path program_path = boost::dll::program_location().parent_path().string();

    std::unordered_map<ButtonName, std::filesystem::path> button_icons
    {
        {ASSETS, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"assets.png"}},
        {LAYERS, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"layers.png"}},
        {ANIMATION_CONTROL, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"properties.png"}},
        {PLAY, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"play.png"}},
        {PAUSE, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"pause.png"}},
        {FORWARD, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"forward.png"}},
        {BACKWARD, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"back.png"}},
        {SAVE_KEYS, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"keyframe.png"}},
        {EXPORT, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"export.png"}},
        {PRINT, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"print.png"}},
        {UNDO, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"undo.png"}},
        {REDO, program_path / std::filesystem::path{"icons"} / std::filesystem::path{"redo.png"}},

        
    };

    std::optional<ExportDialog> export_dialog = std::nullopt;
    FrameExportDialog frame_export_dialog;


    bool is_sliding_time = false;

    void set_style();
    void handle_time_slider_clicks(const ImVec2 indicator_pos,const ImVec2 start,const ImVec2 end);
    
    void close_sections_from_button(SectionButton* section_button);
    void  open_sections_from_button(SectionButton* section_button);

    void update_sections_position(const ImVec2 action_bar_pos,const ImVec2 action_bar_size);
    void render_section_button(SectionButton* section_button); 
    bool render_icon_button(const ButtonName section);

    void render_time_controls(const ImVec2 action_bar_size);
    void render_right_buttons(const ImVec2 action_bar_size);
    void render_time_slider();


    public:
        ActionBar(std::unordered_map<ButtonName,SectionButton> action_buttons);

        void render();
};