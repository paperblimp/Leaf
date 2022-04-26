#pragma once

#include <optional>

#include <imgui.h>
#include <fmt/core.h>
#include <glm/vec2.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>

#include "utils/log.hpp"
#include "node_tree.hpp"
#include "graphical/opengl/framebuffer.hpp"
#include "sections/section.hpp"



class Preview
{

    private:

        Framebuffer framebuffer;

    public:

        Preview(): framebuffer{0, 0} {}
        void render_subsection(const ImVec2& size);
    
    private:

        glm::vec2 get_current_available_window_size();
        glm::vec2 scale(float max_size, glm::vec2 size);

};
