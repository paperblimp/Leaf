#pragma once

// builtin
#include <optional>
#include <string>

// extern
#include <imgui.h>
#include "glm/ext/scalar_constants.hpp"

// local
#include "history.hpp"
#include "utils/log.hpp"
#include "utils/math_utils.hpp"
#include "node_tree.hpp"
#include "animation/easings.hpp"
#include "animation/keyframe.hpp"
#include "sections/section.hpp"



class Node;


class PropertyEditor
{   
    private:
        
        //Rotation drag options
        const float rot_rad = 2 * glm::pi<float>();
        const float rot_deg = 360;
        const float step_rad = rot_rad / 720;
        const float step_deg = rot_deg / 720;

        std::optional<std::shared_ptr<Node>> current_node = std::nullopt;
        std::optional<glm::vec2> current_rotation_pivot;
        std::optional<double> current_rotation;
        std::optional<double> current_rotation_degrees;
        std::optional<glm::vec2> current_scale;
        std::optional<glm::vec2> current_position;
        std::optional<size_t> current_layer;

    public:

        void render(const ImVec2& size);

    private:

        void reset_status();
        void init_status(Node& node);
    
};
