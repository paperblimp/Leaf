#pragma once

//builtin
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// extern
#include <imgui.h>
#include <imgui_internal.h>
#include <fmt/core.h>

//local
#include "animation/animation.hpp"
#include "utils/math_utils.hpp"
#include "node_tree.hpp"



class KeyframeWidget : public Section
{   
    
    //Timeline endering constants
    const ImU32 MAJOR_COLLUM_COLOR = IM_COL32(  0,  0,  0,255);
    const ImU32 MINOR_COLLUM_COLOR = IM_COL32( 94, 98,108,150);
    const ImU32 TRACK_LINE_COLOR   = IM_COL32( 94, 98,108,255);
    const ImU32 TIME_IND_COLOR     = IM_COL32(188,  0,  0,255);
    const ImU32 KEY_COLOR          = IM_COL32(0  ,  0,255,255);
    const ImU32 SELECTED_KEY_COLOR = IM_COL32( 14,199, 58,255);
    
    const int TIMELINE_SCROLLBAR_RESERVED_HEIGHT = 15;
    const int TIMELINE_NUMBER_PADDING = 5;
    const int TIMELINE_MINOR_COLLUM_BORDER_UP = 9;
    const int TIMELINE_BORDER_LEFT = 5;

    const int TIMELINE_COLLUM_BASE_SIZE = 50;
    const int TIMELINE_COLLUM_ZOOM_STEP = 10;
    const int TIME_INDICATOR_SIZE = 4;

    const int TIMELINE_BASE_PRECISION = 10;
    const int TIMELINE_MIN_ZOOM_REQUIRED_FOR_DIVS = -2;

    const int TIMELINE_KEY_HALF_SIZE_X = 5;
    const int TIMELINE_KEY_HALF_SIZE_Y = 5;

    const int HALF_SIZE_TRACK_TEXT = 11;
    



    //Timeline rendering variables
    int current_precision_level = TIMELINE_BASE_PRECISION;
    int current_collum_size     = TIMELINE_COLLUM_BASE_SIZE;

    ImVec2 timeline_corners[2]{};

    float tracks_bottom = 0;
    int scroll_x = 0; //Collected from timeline
    int scroll_y = 0; //Collected from tracks

    ImDrawList* draw_list;
    ImGuiIO IO;

    bool click_captured = false;
    bool key_menu_open  = false;

    //Timeline properties

    //Values bigger than this will overflow the size for the number label
    const uint32_t MAX_ANIMATION_LENGTH = 1000000; // ~~277h of animation

    bool is_dragging_time = false;

    int zoom = 0;

    //Property tracks

    const  std::array<std::string, 4> track_names //Order need to be the same of keyframe::tracks enum
    {
        "Position",
        "Scale   ",
        "Rotation",
        "Pivot   "
    };

    std::array<ImVec2,4> track_positions {};

    //Holds a index and a time value in order to identify the selected key
    std::optional<std::pair<Track,double>> selected_key; 

    void render_controls();

    void render_track_lines();
    void render_time_indicator();
    void render_time_columns();
    void render_timeline();

    void render_keys();

    template <Track track>
    void render_property_keys(KeyFrame& keyframe);

    template <typename instant_T, Track track>
    bool render_key_menu(instant_T& instant);

    bool is_selected_key(const Track track, const double time);

    
    void handle_timeline_clicks();

    public:
        void render();
};
