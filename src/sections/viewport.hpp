#pragma once


// local
#include "imgui_internal.h"
#include "graphical/opengl/framebuffer.hpp"
#include "node_tree.hpp"
#include "utils/math_utils.hpp"
#include "sections/node_renderer.hpp"



struct MoveNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        glm::vec2 new_position;
        glm::vec2 old_position;

    public:

        MoveNode(std::shared_ptr<Node> node, glm::vec2 new_position, glm::vec2 old_position);

        virtual void apply() const override;
        virtual void revert() const override;
};        


struct RotateNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        double new_rotation;
        double old_rotation;

    public:

        RotateNode(std::shared_ptr<Node> node, double new_rotation, double old_rotation);
        virtual void apply() const override;
        virtual void revert() const override;
};

struct ScaleNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        glm::vec2 new_scale;
        glm::vec2 old_scale;

    public:

        ScaleNode(std::shared_ptr<Node> node, glm::vec2 new_scale, glm::vec2 old_scale);
        virtual void apply() const override;
        virtual void revert() const override;
};



const auto NODE_CORNER_SIDE_SIZE = glm::vec2{20, 20};
const float NODE_SIDE_THICKNESS = 10;
const auto NODE_GRABBER_SIZE = glm::vec2{25, 25};
const auto NODE_GRABBER_DISTANCE = 15;

struct VRectangle
{
    glm::vec2 position;
    glm::vec2 size;
    double angle;
};

struct SelectedNode
{
    public:
    
        std::shared_ptr<Node> node;
        std::optional<glm::vec2> initial_position;
        std::optional<double> initial_rotation;
        std::optional<glm::vec2> initial_scale;

    public:

        SelectedNode(std::shared_ptr<Node> node);
        void reset();
        
};

struct Viewport
{
    private:

        enum class DragMode
        {
            Move,
            Rotate,
            Scale,
            VerticalScale,
            HorizontalScale
        };

        bool is_focused = false;
        bool is_mouse_inside_imgui_window = false;

        Framebuffer framebuffer;
        
        glm::vec2 viewport_position;
        bool mouse_pressed = false;
        std::vector<SelectedNode> selected_nodes;
        std::optional<std::shared_ptr<Node>> current_node = std::nullopt;
        std::optional<glm::vec2> mouse_pressed_grabber_offset = std::nullopt;
        std::optional<glm::vec2> mouse_pressed_corner_offset = std::nullopt;
        std::optional<DragMode> mouse_drag_mode = std::nullopt;

        NodeMenu node_menu{"node_viewport_menu"};

    public:

        Viewport();

        void render();
        void process_input(const int button, const int action);
        void stop_current_action();

    private:

        void process();
        void dispatch_input(glm::u64vec2 position);
        void gen_event();

        void draw_node(Node& node);
        void draw_grabber(VRectangle rectangle, VRectangle node_rectangle, glm::u8vec4 color = {255, 255, 0, 150});

        void new_selection(std::shared_ptr<Node> node);
        void add_selection(std::shared_ptr<Node> node);
        void reset_selection();

        VRectangle get_node_position(Node& node);
        std::optional<glm::vec2> point_inside_rectangle(VRectangle rectangle, glm::vec2 point, std::optional<glm::vec2> pivot = std::nullopt);
        std::optional<std::shared_ptr<Node>> get_node_at_position(glm::u64vec2 position);
        glm::vec2 rectangle_offset(VRectangle rectangle, glm::vec2 point, std::optional<glm::vec2> pivot = std::nullopt);
        
        VRectangle get_node_up_left_corner(VRectangle rectangle);
        VRectangle get_node_up_right_corner(VRectangle rectangle);
        VRectangle get_node_down_left_corner(VRectangle rectangle);
        VRectangle get_node_down_right_corner(VRectangle rectangle);

        VRectangle get_node_up_side(VRectangle rectangle);
        VRectangle get_node_right_side(VRectangle rectangle);
        VRectangle get_node_down_side(VRectangle rectangle);
        VRectangle get_node_left_side(VRectangle rectangle);

        VRectangle get_node_rotation_grabber(VRectangle rectangle);


        glm::vec2 get_current_available_window_size();
        glm::dvec2 get_mouse_viewport_position();
        glm::vec2 calculate_corner(glm::vec2 position, glm::vec2 center);
        glm::vec2 calculate_horizontal_side(glm::vec2 position, glm::vec2 center);
        glm::vec2 calculate_vertical_side(glm::vec2 position, glm::vec2 center);

};
