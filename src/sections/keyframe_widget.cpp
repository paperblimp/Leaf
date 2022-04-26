
// header
#include "keyframe_widget.hpp"

// local
#include "animation/easings.hpp"
#include "node_tree.hpp"
#include "utils/math_utils.hpp"
#include "sections/node_history.hpp"
#include "graphical/custom_widgets.hpp"
#include "history.hpp"

// builtin
#include <algorithm>

// extern
#include <fmt/core.h>
#include <imgui.h>
#include <optional>



bool is_scrolling_x()
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiID active_id = ImGui::GetActiveID();
    return  active_id && (active_id == ImGui::GetWindowScrollbarID(window, ImGuiAxis_X));
}

void KeyframeWidget::handle_timeline_clicks()
{   
    const ImVec2 timeline_bottom
    {
        timeline_corners[1].x,
        track_positions[3].y + HALF_SIZE_TRACK_TEXT
    };
    
    if(ImGui::IsMouseClicked(0) && !click_captured && !key_menu_open && !is_scrolling_x() && 
         is_inside_rect(ImGui::GetMousePos(),timeline_corners[0],timeline_bottom) )
        {
            is_dragging_time = true;
        }
    else if( ImGui::IsMouseReleased(0))
    {
        is_dragging_time = false;
    }

    if(is_dragging_time)
    {
        //Timeline index clicked
        const double click_time = normalize(
            ImGui::GetMousePos().x + scroll_x ,
            timeline_corners[0].x  + TIMELINE_BORDER_LEFT , 
            timeline_corners[0].x  + TIMELINE_BORDER_LEFT + (float)(current_collum_size*anim_data.length) 
        ) * anim_data.length ; 

        anim_data.set_time( (int)(click_time*10 + 0.5)/10.0); //Value rounded up
    }
}


void KeyframeWidget::render_controls()
{
    //Time related
    ImGui::PushItemWidth(100);

    ImGui::Text("Time:");

    double temp_time = anim_data.get_time();
    if( ImGui::SameLine(); ImGui::InputDouble("##KeyframeTime",  &temp_time,0.0,0.0,"%.3f"))
    {
        anim_data.set_time(temp_time);
    }

    ImGui::SameLine(); ImGui::Text("Length");
    ImGui::SameLine(); ImGui::InputDouble("##KeyframeLength",&anim_data.length,0.0,0.0,"%.3f");
    if(anim_data.length>MAX_ANIMATION_LENGTH) anim_data.length = MAX_ANIMATION_LENGTH;

    ImGui::SameLine(); ImGui::SliderInt("Zoom", &zoom,-4,30);


    //ImGui::SameLine(); ImGui::InputDouble("Step##KeyframeStep",  &step);
    ImGui::PopItemWidth();

    //Other
    ImGui::SameLine(); ImGui::Checkbox("Loop",&anim_data.loop);

}

void KeyframeWidget::render_track_lines()
{
    //Lines
    const ImVec2 win_pos = ImGui::GetWindowPos();
    for(size_t j = 0;j<track_names.size();j++)
    {   
        ImVec2 line_point1
        (
            win_pos.x ,
            track_positions[j].y
        );

        //Add a TIMELINE_BORDER_LEFTpx border when there's no scroll
        if(scroll_x < TIMELINE_BORDER_LEFT) line_point1.x += TIMELINE_BORDER_LEFT - scroll_x;

        const ImVec2 line_point2
        (
            win_pos.x + TIMELINE_BORDER_LEFT + (current_collum_size*anim_data.length) - scroll_x,
            track_positions[j].y
        );
        draw_list->AddLine(line_point1, line_point2, TRACK_LINE_COLOR);
    }
}

void KeyframeWidget::render_time_columns()
{   
    //Columns
    const ImVec2 win_pos  = ImGui::GetWindowPos();
    const ImVec2 win_size = ImGui::GetWindowSize();

    //Distance between collumns according to zoom
    current_collum_size     = TIMELINE_COLLUM_BASE_SIZE + (zoom * TIMELINE_COLLUM_ZOOM_STEP);

    //Render a invisible button in order to increase the size of the window and get scroll, as custom render doesn't increase
    const int total_length = win_pos.x + TIMELINE_BORDER_LEFT + current_collum_size*anim_data.length;
    ImGui::InvisibleButton("KeyframeScroll", ImVec2(total_length,1));
    scroll_x = ImGui::GetScrollX();

    //Start point for all timeline rendering
    const float timeline_origin_x = win_pos.x + TIMELINE_BORDER_LEFT - scroll_x;

    for(size_t i = 0;i<=anim_data.length;i++)
    {   
        //Base positions
        const ImVec2 col_point1 = ImVec2
        (
            timeline_origin_x + current_collum_size*i,
            win_pos.y 
        );
        const ImVec2 col_point2 = ImVec2
        (
            timeline_origin_x + current_collum_size*i,
            tracks_bottom
        );

        //Skip rendering before current scroll level
        if(col_point1.x < (win_pos.x - current_collum_size) ) continue; 

        //Stop rendering after  current scroll level     
        if(col_point1.x > (win_pos.x + win_size.x)) break; 

        //Minor divs rendering 
        if(zoom >= TIMELINE_MIN_ZOOM_REQUIRED_FOR_DIVS){
            const int minor_collum_size = current_collum_size / TIMELINE_BASE_PRECISION;
            for (int m = 0;m <= current_precision_level && i < anim_data.length;m++)
            {
                ImVec2 minor_col_point1
                (
                    col_point1.x + (minor_collum_size * m) ,
                    col_point1.y + TIMELINE_MINOR_COLLUM_BORDER_UP - scroll_y
                );

                const ImVec2 minor_col_point2
                (
                    col_point2.x + (minor_collum_size * m),
                    col_point2.y 
                );

                //Higher divs on half timeline_precision_level
                if(current_precision_level != 1  && m != current_precision_level && m%(current_precision_level/2) == 0)
                {
                    minor_col_point1.y -= (float)TIMELINE_MINOR_COLLUM_BORDER_UP / 3; 
                }

                draw_list->AddLine(minor_col_point1, minor_col_point2, MINOR_COLLUM_COLOR);

            }
        }

        //Major divs rendering
        draw_list->AddLine(col_point1, col_point2, MAJOR_COLLUM_COLOR);

        //Major number rendering
        const ImVec2 font_pos = ImVec2
        (
            col_point1.x + TIMELINE_NUMBER_PADDING,
            col_point1.y
        );
        draw_list->AddText(font_pos,MAJOR_COLLUM_COLOR,fmt::format("{}",i).c_str());


    }
}

void KeyframeWidget::render_time_indicator()
{
    //Time indicator
    const ImVec2 win_pos  = ImGui::GetWindowPos();

    const ImVec2 ind_point1 = ImVec2
    (
        win_pos.x + TIMELINE_BORDER_LEFT - scroll_x + (current_collum_size*anim_data.get_time()) - ((float)TIME_INDICATOR_SIZE / 2),
        win_pos.y 
    );
    const ImVec2 ind_point2 = ImVec2
    (
        win_pos.x + TIMELINE_BORDER_LEFT - scroll_x + (current_collum_size*anim_data.get_time()) + ((float)TIME_INDICATOR_SIZE / 2),
        tracks_bottom
    );

    const ImVec2 ind_triangle_point1 = ImVec2
    (
        ind_point1.x - TIME_INDICATOR_SIZE,
        win_pos.y
    );

    const ImVec2 ind_triangle_point2 = ImVec2
    (
        ind_point2.x + TIME_INDICATOR_SIZE,
        win_pos.y
    );

    const ImVec2 ind_triangle_point3 = ImVec2
    (
        ind_point1.x + (float)TIME_INDICATOR_SIZE / 2,
        win_pos.y    + TIME_INDICATOR_SIZE * 2
    );

    draw_list->AddTriangleFilled(ind_triangle_point1, ind_triangle_point2, ind_triangle_point3, TIME_IND_COLOR);
    draw_list->AddRectFilled(ind_point1, ind_point2, TIME_IND_COLOR);
}



bool KeyframeWidget::is_selected_key(const Track track, const double time)
{
    return selected_key.has_value() && selected_key.value().first == track && selected_key.value().second == time;
}

void render_key_menu_input(Vector2Instant& instant)
{
    float temp_vec[2]{instant.vector.x,instant.vector.y};
    if(ImGui::InputFloat2("##KeyMenuVec2Input", temp_vec))
    {
        instant.vector.x = temp_vec[0];
        instant.vector.y = temp_vec[1];
    }
}

void render_key_menu_input(DoubleInstant& instant)
{
    double temp_value = instant.value;
    if(ImGui::InputDouble("##KeyMenuDoubleInput", &temp_value))
    {
        instant.value = temp_value;
    }
}

template <typename instant_T, Track track>
bool KeyframeWidget::render_key_menu(instant_T& instant)
{   
    //Render the key menu from a given Instant key
    if(ImGui::BeginPopup(fmt::format("##KeyMenu{}{}", get_track_type<track>::name, instant.time).c_str()))
    {

        key_menu_open = true; //Used to ignore clicks on the timeline while open

        ImGui::Text("Property: %s", get_track_type<track>::name);
        ImGui::Text("Instant : %f", instant.time);

        render_key_menu_input(instant);

        if(ImGui::BeginCombo("##KeyMenuCombo", easings_ref.at(instant.easing).c_str()))
        {
            for(const auto& easing : easings) 
            {
                if(ImGui::Selectable(easing.first.c_str()))
                {
                    instant.easing = easing.second;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
        return true;
    }

    return false;
}

template <Track track>
void KeyframeWidget::render_property_keys(KeyFrame& keyframe)
{

    const auto& property_keys = keyframe.get_track<track>();

    for(size_t key = 0; key < property_keys.size();key++)
    {

        //Calculate rhombus key position
        const double time = property_keys[key].time;
        const float norm_time = normalize(time, 0.0, anim_data.length);

        const ImVec2 center_pos
        {
            interpolate(
                norm_time,
                timeline_corners[0].x  + TIMELINE_BORDER_LEFT , 
                timeline_corners[0].x  + TIMELINE_BORDER_LEFT + (float)(current_collum_size*anim_data.length),
                &Easings::linear
            ),
            track_positions[(size_t)track].y
        };

        const ImVec2 point1{center_pos.x                           , center_pos.y - TIMELINE_KEY_HALF_SIZE_Y };
        const ImVec2 point2{center_pos.x + TIMELINE_KEY_HALF_SIZE_X, center_pos.y                            };
        const ImVec2 point3{center_pos.x                           , center_pos.y + TIMELINE_KEY_HALF_SIZE_Y };
        const ImVec2 point4{center_pos.x - TIMELINE_KEY_HALF_SIZE_X, center_pos.y                            };

        //Rhombus input
        bool selected = selected_key.has_value() && is_selected_key(track, time);

        if(IO.MouseClicked[0] &&  !click_captured &&
            is_inside_rhombus(IO.MouseClickedPos[0], center_pos, TIMELINE_KEY_HALF_SIZE_X, TIMELINE_KEY_HALF_SIZE_Y))
            {
                click_captured = true;
                selected = !selected;

                //Unselect
                if(is_selected_key(track, time)) 
                {
                    selected_key = std::nullopt;
                }
                //Select
                else
                {
                    selected_key = std::pair<Track,double>{track,time};
                    ImGui::OpenPopup(fmt::format("##KeyMenu{}{}", get_track_type<track>::name, time).c_str());
                }

            }

        //render_key_menu returns true if the popup is still open
        //Used to catch unselection event when the user clicks outside of the popup
        selected = render_key_menu<get_track_type_t<track>, track>(*keyframe.get_instant<track>(property_keys[key].time).value());

        //Selected key menu rendering
        if(selected)
        {
            draw_list->AddQuadFilled(point1,point2,point3,point4,SELECTED_KEY_COLOR);
            if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
            {
                auto instant = node_tree->selected_nodes[0]->keyframe.remove_key<track>(selected_key.value().second).value();
                history->push_action(std::make_unique<NodeKeyframeRemove<track>>(node_tree->selected_nodes[0], instant));
            }
        }
        else draw_list->AddQuadFilled(point1,point2,point3,point4,KEY_COLOR);

    }
}

void KeyframeWidget::render_keys()
{

    if(node_tree->selected_nodes.empty()) return;

    KeyFrame keyframe = node_tree->selected_nodes[0]->keyframe;


    //Position keys
    render_property_keys<Track::POSITION>(keyframe);

    //Scale keys
    render_property_keys<Track::SCALE>(keyframe);

    //Rotation  keys
    render_property_keys<Track::ROTATION>(keyframe);

    //Pivot keys
    render_property_keys<Track::PIVOT>(keyframe);
}

void KeyframeWidget::render_timeline()
{
    ImGui::Separator();
    
    ImGui::BeginChild("Tracks",ImVec2(75,0), false);

        //Track texts

        ImGui::Text("Time    ");
        for(size_t i = 0;i<track_names.size();i++)
        {
            ImGui::Text("%s", track_names[i].c_str());

            //Saves the text track position, half height down
            track_positions[i].x = ImGui::GetCursorScreenPos().x;
            track_positions[i].y = ImGui::GetCursorScreenPos().y - HALF_SIZE_TRACK_TEXT;



            tracks_bottom = track_positions[i].y; //Y bottom position of the last rendered track
        }
        
        scroll_y = ImGui::GetScrollY();

    ImGui::EndChild();

    const auto old_style = ImGui::GetStyle();
    ImGui::GetStyle().ChildRounding = 0;

    ImGui::SameLine();
    ImGui::BeginChild("Timeline",ImVec2(ImGui::GetContentRegionAvail().x,0), true,ImGuiWindowFlags_HorizontalScrollbar);


        //Effective render and interaction are for the timeline
        timeline_corners[0]   = ImGui::GetWindowPos();

        timeline_corners[1].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        timeline_corners[1].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - TIMELINE_SCROLLBAR_RESERVED_HEIGHT;


        draw_list->PushClipRect(timeline_corners[0], timeline_corners[1],true);

        render_track_lines();
        render_time_columns();
        render_time_indicator();

        render_keys();

        handle_timeline_clicks();//Handled after keys are rendered in order to ignore clicks already captured there

        draw_list->PopClipRect();

    ImGui::EndChild();

    auto& style = ImGui::GetStyle();
    style = old_style;
}

void KeyframeWidget::render()
{
    

    ImGui::Begin("##Keyframes",NULL,SECTION_FLAGS);
    CustomImGui::Title("Keyframe Editor");
    draw_list = ImGui::GetWindowDrawList();
    IO        = ImGui::GetIO();
    if(IO.MouseReleased[0]) click_captured = false;
    key_menu_open = false;

    render_controls();
    render_timeline();
    

    ImGui::End();

    if(!anim_data.paused) node_tree->run_on_nodes([](Node& node){ animate(node, anim_data.get_time()); });

}
