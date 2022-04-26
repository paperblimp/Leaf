#include "preview.hpp"

#include <array>
#include <optional>
#include <vector>

#include "glm/detail/qualifier.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <fmt/core.h>
#include <glm/fwd.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include "utils/asserts.hpp"
#include "utils/math_utils.hpp"
#include "node_tree.hpp"
#include "graphical/sprite.hpp"
#include "graphical/opengl/render.hpp"
#include "graphical/custom_widgets.hpp"


#undef min
#undef max


void Preview::render_subsection(const ImVec2& size)
{
    //Render the texture of the selected_node with the user properties

    ImGui::BeginChild("Preview",size,true);
    CustomImGui::SubTitle("Preview");

    if(node_tree->selected_nodes.empty())
    {
        ImGui::Text("Nothing selected");
        ImGui::EndChild();
        return;
    }

    if(!node_tree->selected_nodes[0]->texture_path.has_value())
    {
        ImGui::EndChild();
        return;
    }

    static bool strech = false;
    ImGui::Checkbox("strech", &strech);


    auto node = node_tree->selected_nodes[0];

    // resize framebuffer if needed
    const auto win_size = this->get_current_available_window_size();
    const auto current_size = this->framebuffer.get_size();
    if ((glm::vec2)current_size.x != win_size)
        this->framebuffer.resize(win_size.x, win_size.y);


    this->framebuffer.clear({0, 0, 0, 0});

    
    const auto& texture = sprite_manager.get_sprite(node->texture_path.value());
    

    // calculate render size
    glm::vec2 texture_render_size = (glm::vec2)texture.size * node->scale;
    if (strech || (texture_render_size.x > win_size.x || texture_render_size.y > win_size.y))
    {
        if (texture_render_size.x > texture_render_size.y && texture_render_size.x < win_size.x)
            texture_render_size = this->scale(win_size.x, texture_render_size);
        else if (texture_render_size.x < texture_render_size.y && texture_render_size.y < win_size.y)
            texture_render_size = this->scale(win_size.y, texture_render_size);
        else
            texture_render_size = this->scale(std::min(win_size.x, win_size.y), texture_render_size);
    }


    const auto texture_render_position = win_size / glm::vec2{2, 2};

    render_sprite(
        texture,
        texture_render_position,
        texture_render_size,
        glm::degrees(node->rotation),
        this->framebuffer
    );
    
    ImGui::Image((void*)(uintptr_t)this->framebuffer.get_texture_id(), {win_size.x, win_size.y});
    ImGui::EndChild();

}


glm::vec2 Preview::get_current_available_window_size()
{
    ImVec2 window_available_size = ImGui::GetWindowSize();
    window_available_size.x -= ImGui::GetCursorPosX();
    window_available_size.y -= ImGui::GetCursorPosY() + 10;

    return {window_available_size.x, window_available_size.y};
}

glm::vec2 Preview::scale(float max_size, glm::vec2 size)
{
    if (size.x > size.y)
    {
        auto proportion = size.y / size.x;
        return {max_size, max_size * proportion};
    }
    else
    {
        auto proportion = size.x / size.y;
        return {max_size * proportion, max_size};
    }
}