// header
#include "viewport.hpp"

// local
#include "node_tree.hpp"
#include "graphical/sprite.hpp"
#include "graphical/opengl/render.hpp"
#include "config.hpp"
#include "key_map.hpp"



glm::vec2 rotate_point(glm::vec2 point, glm::vec2 pivot, double angle)
{
    std::array<glm::tvec3<float>, 1> v{glm::tvec3<float>{point.x, point.y, 1}};
    rotate_figure(v, (float)angle, glm::tvec3<float>{pivot.x, pivot.y, 1});
    return v[0];
}



SelectedNode::SelectedNode(std::shared_ptr<Node> _node): node{std::move(_node)} {}

void SelectedNode::reset()
{
    this->initial_position = std::nullopt;
    this->initial_scale = std::nullopt;
    this->initial_rotation = std::nullopt;
}


Viewport::Viewport(): framebuffer{0, 0} {}

void Viewport::render()
{
    this->process();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    auto current_framebuffer_size = this->framebuffer.get_size();
    auto current_available_window_size = this->get_current_available_window_size();
    auto camera_size = get_camera_area();
    if ((glm::vec2)current_framebuffer_size != camera_size)    
        this->framebuffer.resize(camera_size.x, camera_size.y);

    this->framebuffer.clear({255, 255, 255, 255});

    node_tree->run_on_nodes_ordered_reverse([this](Node& node)
    {
        if (node.texture_path.has_value() == false)
            return;

        if (node.visible == false)
            return;

        this->draw_node(node);
    });

    auto cursor_pos = glm::vec2{0, 0};

    if (camera_size.x < current_available_window_size.x)
        cursor_pos.x = (current_available_window_size.x - camera_size.x) * 0.5f;

    if (camera_size.y < current_available_window_size.y)
        cursor_pos.y = (current_available_window_size.y - camera_size.y) * 0.5f;

    ImGui::SetCursorPos(ImVec2{cursor_pos.x, cursor_pos.y});
    
    auto cursor_screen_pos = ImGui::GetCursorScreenPos();
    this->viewport_position = {std::round(cursor_screen_pos.x), std::round(cursor_screen_pos.y)};

    auto texture = this->framebuffer.get_texture_id();
    ImGui::Image((void*)(uintptr_t)texture, {(float)camera_size.x, (float)camera_size.y});

    this->is_focused = ImGui::IsWindowFocused();
    this->is_mouse_inside_imgui_window = is_inside_rect_by_size(ImGui::GetMousePos(),ImGui::GetWindowPos(),ImGui::GetWindowSize());
    ImGui::End();

    this->node_menu.render();
}

void Viewport::process()
{
    if (this->mouse_pressed == false)
        return;
    
    leaf_assert(this->selected_nodes.empty() == false);
    leaf_assert(this->mouse_pressed_grabber_offset.has_value());

    auto& node = *this->current_node.value();
    auto node_rectangle = this->get_node_position(node);
    auto position = (glm::vec2)this->get_mouse_viewport_position();

    switch (this->mouse_drag_mode.value())
    {
        case (DragMode::Move):
        {
            auto offset = (position - this->mouse_pressed_grabber_offset.value()) - node.position;

            for (auto& node: this->selected_nodes)
                node.node->position += offset;

            break;
        }
        case (DragMode::Scale):
        {
            auto& sprite = sprite_manager.get_sprite(node.texture_path.value());

            // remove mouse position angle
            position = rotate_point(position, node_rectangle.position, glm::radians(-node_rectangle.angle));
            
            // get next corner position
            auto corner = position + this->mouse_pressed_corner_offset.value();
            
            auto new_scale = (abs(corner - node_rectangle.position) * glm::vec2{2, 2}) / (glm::vec2)sprite.size;
            auto scale_increase = new_scale / node.scale;

            for (auto& node: this->selected_nodes)
                node.node->scale *= scale_increase;

            break;
        }
        case (DragMode::Rotate):
        {
            position -= this->mouse_pressed_grabber_offset.value();
            position -= node_rectangle.position;

            auto angle = atan2(position.y, position.x);
            angle -= glm::radians(double(90));

            auto offset = angle - node.rotation;

            for (auto& node: this->selected_nodes)
                node.node->rotation += offset;

            break;
        }
        case (DragMode::VerticalScale):
        {
            auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
            
            // remove mouse position angle
            position = rotate_point(position, node_rectangle.position, glm::radians(-node_rectangle.angle));

            // get next corner position
            auto side = position + this->mouse_pressed_corner_offset.value();

            auto new_scale = (abs(side.y - node_rectangle.position.y) * 2) / sprite.size.y;
            auto ratio_increase = new_scale / node.scale.y;

            for (auto& node: this->selected_nodes)
            {
                node.node->scale.y *= ratio_increase;

                if (key_map_get_ctrl())
                    node.node->scale.x *= ratio_increase;
            }

            break;
        }
        case (DragMode::HorizontalScale):
        {
            auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
            
            // remove mouse position angle
            position = rotate_point(position, node_rectangle.position, glm::radians(-node_rectangle.angle));

            // get next corner position
            auto side = position + this->mouse_pressed_corner_offset.value();

            auto new_scale = (abs(side.x - node_rectangle.position.x) * 2) / sprite.size.x;
            auto ratio_increase = new_scale / node.scale.x;

            for (auto& node: this->selected_nodes)
            {
                node.node->scale.x *= ratio_increase;

                if (key_map_get_ctrl())
                    node.node->scale.y *= ratio_increase;
            }

            break;
        }
    }
}

void Viewport::draw_node(Node& node)
{
    leaf_assert(node.texture_path.has_value());

    auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
    render_sprite(
        sprite,
        node.position,
        (glm::dvec2)sprite.size * (glm::dvec2)node.scale,
        glm::degrees(node.rotation),
        {node.rotation_pivot[0] + node.position.x, node.rotation_pivot[1] + node.position.y},
        this->framebuffer
    );

    auto node_ptr = node.shared_from_this();
    if (std::find_if(this->selected_nodes.begin(), this->selected_nodes.end(), [&](auto& snode){ return snode.node == node_ptr; }) == this->selected_nodes.end())
        return;

    auto node_rectangle = this->get_node_position(node);


    auto grabbers = std::vector<VRectangle>{
        this->get_node_up_left_corner(node_rectangle),
        this->get_node_up_right_corner(node_rectangle),
        this->get_node_down_left_corner(node_rectangle),
        this->get_node_down_right_corner(node_rectangle),
        this->get_node_up_side(node_rectangle),
        this->get_node_right_side(node_rectangle),
        this->get_node_down_side(node_rectangle),
        this->get_node_left_side(node_rectangle),
        this->get_node_rotation_grabber(node_rectangle)
    };


    auto mouse_position = this->get_mouse_viewport_position();

    for (auto& grabber: grabbers)
    {
        if (this->point_inside_rectangle(grabber, mouse_position, node_rectangle.position))
            this->draw_grabber(grabber, node_rectangle, {255, 255, 0, 255});
        else
            this->draw_grabber(grabber, node_rectangle);
    }
}

void Viewport::draw_grabber(VRectangle rectangle, VRectangle node_rectangle, glm::u8vec4 color)
{
    render_color(
        color,
        rectangle.position,
        rectangle.size,
        rectangle.angle,
        node_rectangle.position,
        this->framebuffer
    );
}


void Viewport::process_input(const int button, const int action)
{
    glm::dvec2 position = this->get_mouse_viewport_position();

    if (action == GLFW_PRESS && this->is_mouse_inside_imgui_window)
    {
        auto viewport_size = this->framebuffer.get_size();
        if (position.x < 0 || position.y < 0 || position.x > viewport_size.x || position.y > viewport_size.y)
            return;

        if (this->node_menu.get_current_node().has_value())
            return;

        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            this->dispatch_input(position);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (auto node = this->get_node_at_position(position); node.has_value())
            {
                this->node_menu.open(node.value());
                this->new_selection(node.value());
            }
        }
    }
    else if (this->mouse_pressed == true)
    {
        leaf_assert(action == GLFW_RELEASE);
        
        this->gen_event();
        
        this->mouse_pressed = false;
        this->current_node = std::nullopt;
        this->mouse_pressed_grabber_offset = std::nullopt;
        this->mouse_pressed_corner_offset = std::nullopt;
        this->mouse_drag_mode = std::nullopt;
    }
}

void Viewport::stop_current_action()
{
    this->mouse_pressed = false;
    this->current_node = std::nullopt;
    this->mouse_pressed_grabber_offset = std::nullopt;
    this->mouse_pressed_corner_offset = std::nullopt;
    this->mouse_drag_mode = std::nullopt;

    for (auto& node: this->selected_nodes)
        node.reset();
}

void Viewport::dispatch_input(glm::u64vec2 position)
{
    const auto get_corner = [](glm::vec2 position, glm::vec2 center, glm::vec2 node_size)
    {
        auto half_node_size = node_size / glm::vec2{2, 2};
    
        position.x += half_node_size.x * ((position.x > center.x) ? 1: -1);
        position.y += half_node_size.y * ((position.y > center.y) ? 1: -1);
    
        return position;
    };

    auto possible_node = this->get_node_at_position(position);

    if (possible_node.has_value() == false)
    {
        if (this->is_focused == true)
            this->reset_selection();

        return;
    }

    auto node = possible_node.value();
    auto node_rectangle = this->get_node_position(*node);
    auto corner = get_corner(position, node_rectangle.position, node_rectangle.size) - this->rectangle_offset(node_rectangle, position);
    this->mouse_pressed_corner_offset = corner - (glm::vec2)position;

    auto new_selection = false;

    this->mouse_pressed = true;
    this->current_node = node;

    if (std::find_if(this->selected_nodes.begin(), this->selected_nodes.end(), [&](auto& snode){ return snode.node == node; }) == this->selected_nodes.end())
    {
        if (key_map_get_ctrl())
        {
            this->add_selection(node);
        }
        else
        {
            this->new_selection(node);
            new_selection = true;
        }
    }


    if (new_selection == true)
        goto move;

    {
        auto corners = std::vector<VRectangle>{
            this->get_node_up_left_corner(node_rectangle),
            this->get_node_up_right_corner(node_rectangle),
            this->get_node_down_left_corner(node_rectangle),
            this->get_node_down_right_corner(node_rectangle),
        };
        
        for (auto& corner: corners)
        {
            if (auto offset = this->point_inside_rectangle(corner, position, {node_rectangle.position}); offset.has_value())
            {
                this->mouse_drag_mode = DragMode::Scale;
                this->mouse_pressed_grabber_offset = offset.value();

                for (auto& node: this->selected_nodes)
                    node.initial_scale = node.node->scale;

                goto end;
            }
        }
    }

    {
        auto vertical_sides = std::vector<VRectangle>{
            this->get_node_up_side(node_rectangle),
            this->get_node_down_side(node_rectangle)
        };

        for (auto& side: vertical_sides)
        {
            if (auto offset = this->point_inside_rectangle(side, position, {node_rectangle.position}); offset.has_value())
            {
                this->mouse_drag_mode = DragMode::VerticalScale;
                this->mouse_pressed_grabber_offset = offset.value();

                for (auto& node: this->selected_nodes)
                    node.initial_scale = node.node->scale;

                goto end;
            }
        }
    }

    {
        auto horizontal_sides = std::vector<VRectangle>{
            this->get_node_right_side(node_rectangle),
            this->get_node_left_side(node_rectangle)
        };

        for (auto& side: horizontal_sides)
        {
            if (auto offset = this->point_inside_rectangle(side, position, {node_rectangle.position}); offset.has_value())
            {
                this->mouse_drag_mode = DragMode::HorizontalScale;
                this->mouse_pressed_grabber_offset = offset.value();

                for (auto& node: this->selected_nodes)
                    node.initial_scale = node.node->scale;

                goto end;
            }
        }
    }

    {
        auto rotation_grabber = this->get_node_rotation_grabber(node_rectangle);
        if (auto offset = this->point_inside_rectangle(rotation_grabber, position, {node_rectangle.position}); offset.has_value())
        {
            this->mouse_drag_mode = DragMode::Rotate;
            this->mouse_pressed_grabber_offset = offset.value();

            for (auto& node: this->selected_nodes)
                    node.initial_rotation = node.node->rotation;

            goto end;
        }
    }

    move:;

    if (this->point_inside_rectangle(node_rectangle, position).has_value())
    {
        this->mouse_drag_mode = DragMode::Move;
        this->mouse_pressed_grabber_offset = ((glm::vec2)position - node_rectangle.position) + (node_rectangle.position - node->position);

        for (auto& node: this->selected_nodes)
            node.initial_position = node.node->position;
    }

    end:;
}

void Viewport::gen_event()
{
    std::vector<std::unique_ptr<Action>> actions;


    for (auto& node: this->selected_nodes)
    {
        switch (this->mouse_drag_mode.value())
        {
            case (DragMode::Scale):
            case (DragMode::VerticalScale):
            case (DragMode::HorizontalScale):
            {
                if (node.initial_scale.has_value() == false || node.initial_scale == node.node->scale)
                    break;

                actions.push_back(std::make_unique<ScaleNode>(node.node, node.node->scale, node.initial_scale.value()));
                node.initial_scale = std::nullopt;
                break;
            }

            case (DragMode::Move):
            {
                if (node.initial_position.has_value() == false || node.initial_position == node.node->position)
                    break;

                actions.push_back(std::make_unique<MoveNode>(node.node, node.node->position, node.initial_position.value()));
                node.initial_position = std::nullopt;
                break;
            }

            case (DragMode::Rotate):
            {
                if (node.initial_rotation.has_value() == false || node.initial_rotation == node.node->rotation)
                    break;

                actions.push_back(std::make_unique<RotateNode>(node.node, node.node->rotation, node.initial_rotation.value()));
                node.initial_rotation = std::nullopt;
                break;
            }

            default:
            {
                panic("leaf internal error");
            }
        }
    }

    if (actions.empty() == true)
        return;

    auto action = std::make_unique<ActionGroup>(std::move(actions));
    history->push_action(std::move(action));
}



glm::vec2 Viewport::get_current_available_window_size()
{
    ImVec2 window_available_size = ImGui::GetWindowSize();
    window_available_size.x -= ImGui::GetCursorPosX();
    window_available_size.y -= ImGui::GetCursorPosY();

    return {window_available_size.x, window_available_size.y};
}


std::optional<std::shared_ptr<Node>> Viewport::get_node_at_position(glm::u64vec2 position) {

    std::vector<std::shared_ptr<Node>> nodes;


    node_tree->run_on_nodes_ordered([&, this](Node& node){

        if (node.texture_path.has_value() == false)
            return;

        auto node_rectangle = this->get_node_position(node);
        if (this->point_inside_rectangle(node_rectangle, (glm::vec2)position) ||
            this->point_inside_rectangle(this->get_node_rotation_grabber(node_rectangle), (glm::vec2)position, node_rectangle.position)
            )
            nodes.push_back(node.shared_from_this());
    });

    std::sort(nodes.begin(), nodes.end(), [](std::shared_ptr<Node> a, std::shared_ptr<Node> b){
        
        if (a->layer < b->layer)
            return true;
        
        const auto compare_node = [](std::shared_ptr<Node> a, std::shared_ptr<Node> b, auto compare_node) -> bool {

            if (a->get_parent() == b->get_parent()) 
            {
                return a->get_idx() < b->get_idx();   
            }
            else if (a->is_rootless() == false && b->is_rootless() == false)
            {
                if (compare_node(a->get_parent(), b->get_parent(), compare_node))
                    return true;
            }
            else if (a->get_parent() == b)
            {
                return false;
            }
            else if (b->get_parent() == a)
            {
                return true;
            }
            else if (a->is_rootless() == false && b->is_rootless())
            {
                if (compare_node(a->get_parent(), b, compare_node))
                    return true;
            }
            else if (a->is_rootless() && b->is_rootless() == false)
            {
                if (compare_node(a, b->get_parent(), compare_node))
                    return true;
            }

            panic("leaf internal error");
        };

        return compare_node(a, b, compare_node);
    });

    if (nodes.empty())
        return std::nullopt;
    else
        return nodes.back();
}


glm::dvec2 Viewport::get_mouse_viewport_position()
{
    glm::dvec2 position;
    glfwGetCursorPos(graphic_context.window, &position.x, &position.y);

    position -= this->viewport_position;
    
    return position;
}


VRectangle Viewport::get_node_position(Node& node)
{
    glm::vec2 position;

    if ((node.rotation_pivot[0] == 0 && node.rotation_pivot[1] == 0))
        position = node.position;
    else
    {
        auto a = atan2(-node.rotation_pivot[1], -node.rotation_pivot[0]);
        a = a + node.rotation;

        glm::vec2 distance = glm::vec2{cos(a), sin(a)};
        auto magnitude = sqrt(pow(node.rotation_pivot[0], 2) + pow(node.rotation_pivot[1], 2));
        distance *= magnitude / (sqrt(pow(distance.x, 2) + pow(distance.y, 2)));

        position = glm::vec2{node.position.x + node.rotation_pivot[0], node.position.y + node.rotation_pivot[1]};
        position += distance;
    }

    auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
    return VRectangle{position, (glm::vec2)sprite.size * node.scale, glm::degrees(node.rotation)};
}


std::optional<glm::vec2> Viewport::point_inside_rectangle(VRectangle rectangle, glm::vec2 point, std::optional<glm::vec2> _pivot)
{
    auto pivot  = (_pivot.has_value()) ? _pivot.value() : rectangle.position;
    auto new_position = rectangle.position - (rectangle.size / glm::vec2{2, 2});
    auto new_point = (rectangle.angle != 0) ? rotate_point(point, pivot, glm::radians(-rectangle.angle)) : point;
    auto end = new_position + rectangle.size;

    if (new_point.x > new_position.x && new_point.y > new_position.y && new_point.x < end.x && new_point.y < end.y)
        return new_point - rectangle.position;
    else
        return std::nullopt;
}

glm::vec2 Viewport::rectangle_offset(VRectangle rectangle, glm::vec2 point, std::optional<glm::vec2> _pivot)
{
    auto pivot  = (_pivot.has_value()) ? _pivot.value() : rectangle.position;
    auto new_point = (rectangle.angle != 0) ? rotate_point(point, pivot, glm::radians(-rectangle.angle)) : point;

    return new_point - rectangle.position;
}

glm::vec2 Viewport::calculate_corner(glm::vec2 position, glm::vec2 center)
{
    auto half_corner_size = NODE_CORNER_SIDE_SIZE / glm::vec2{2, 2};

    position.x += half_corner_size.x * ((position.x > center.x) ? 1: -1);
    position.y += half_corner_size.y * ((position.y > center.y) ? 1: -1);

    return position;
}

glm::vec2 Viewport::calculate_horizontal_side(glm::vec2 position, glm::vec2 center)
{
    auto half_side_size = NODE_SIDE_THICKNESS / 2;

    position.x += half_side_size * ((position.x > center.x) ? 1 : -1);

    return position;
}

glm::vec2 Viewport::calculate_vertical_side(glm::vec2 position, glm::vec2 center)
{
    auto half_side_size = NODE_SIDE_THICKNESS / 2;

    position.y += half_side_size * ((position.y > center.y) ? 1 : -1);

    return position;
}

VRectangle Viewport::get_node_up_left_corner(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto corner = VRectangle{
        rectangle.position,
        NODE_CORNER_SIDE_SIZE,
        rectangle.angle
    };

    corner.position += corner.size / glm::vec2{2, 2};
    return corner;
}

VRectangle Viewport::get_node_up_right_corner(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto corner = VRectangle{
        rectangle.position + glm::vec2{rectangle.size.x, 0} - glm::vec2{NODE_CORNER_SIDE_SIZE.x, 0},
        NODE_CORNER_SIDE_SIZE,
        rectangle.angle
    };

    corner.position += corner.size / glm::vec2{2, 2};
    return corner;
}

VRectangle Viewport::get_node_down_left_corner(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};
    
    auto corner =  VRectangle{
        rectangle.position + glm::vec2{0, rectangle.size.y} - glm::vec2{0, NODE_CORNER_SIDE_SIZE.y},
        NODE_CORNER_SIDE_SIZE,
        rectangle.angle
    };

    corner.position += corner.size / glm::vec2{2, 2};
    return corner;
}

VRectangle Viewport::get_node_down_right_corner(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto corner =  VRectangle{
        rectangle.position + rectangle.size - NODE_CORNER_SIDE_SIZE,
        NODE_CORNER_SIDE_SIZE,
        rectangle.angle
    };

    corner.position += corner.size / glm::vec2{2, 2};
    return corner;
}

VRectangle Viewport::get_node_up_side(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto side = VRectangle{
        rectangle.position + glm::vec2{NODE_CORNER_SIDE_SIZE.x, 0},
        glm::vec2{rectangle.size.x - NODE_CORNER_SIDE_SIZE.x * 2, NODE_SIDE_THICKNESS},
        rectangle.angle
    };
    
    side.position += side.size / glm::vec2{2, 2};
    return side;
}

VRectangle Viewport::get_node_right_side(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto side = VRectangle{
        rectangle.position + glm::vec2{0, NODE_CORNER_SIDE_SIZE.y},
        glm::vec2{NODE_SIDE_THICKNESS, rectangle.size.y - NODE_CORNER_SIDE_SIZE.y * 2},
        rectangle.angle
    };
    
    side.position += side.size / glm::vec2{2, 2};
    return side;
}

VRectangle Viewport::get_node_down_side(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto side = VRectangle{
        rectangle.position + glm::vec2{NODE_CORNER_SIDE_SIZE.x, 0} + glm::vec2{0, rectangle.size.y - NODE_SIDE_THICKNESS},
        glm::vec2{rectangle.size.x - NODE_CORNER_SIDE_SIZE.x * 2, NODE_SIDE_THICKNESS},
        rectangle.angle
    };
    
    side.position += side.size / glm::vec2{2, 2};
    return side;
}

VRectangle Viewport::get_node_left_side(VRectangle rectangle)
{
    rectangle.position -= rectangle.size / glm::vec2{2, 2};

    auto side = VRectangle{
        rectangle.position + glm::vec2{rectangle.size.x - NODE_SIDE_THICKNESS, NODE_CORNER_SIDE_SIZE.y},
        glm::vec2{NODE_SIDE_THICKNESS, rectangle.size.y - NODE_CORNER_SIDE_SIZE.y * 2},
        rectangle.angle
    };
    
    side.position += side.size / glm::vec2{2, 2};
    return side;
}

VRectangle Viewport::get_node_rotation_grabber(VRectangle rectangle)
{
    auto side = VRectangle{
        rectangle.position + glm::vec2{0, (rectangle.size.y / 2) + NODE_GRABBER_DISTANCE + (NODE_GRABBER_SIZE.y / 2)},
        NODE_GRABBER_SIZE,
        rectangle.angle
    };

    return side;
}




MoveNode::MoveNode(std::shared_ptr<Node> _node, glm::vec2 _new_position, glm::vec2 _old_position): node{_node}, new_position{_new_position}, old_position{_old_position} {}

void MoveNode::apply() const
{
    this->node->position = this->new_position;
}

void MoveNode::revert() const
{
    this->node->position = this->old_position;
}


RotateNode::RotateNode(std::shared_ptr<Node> _node, double _new_rotation, double _old_rotation): node{_node}, new_rotation{_new_rotation}, old_rotation{_old_rotation} {}

void RotateNode::apply() const
{
    this->node->rotation = this->new_rotation;
}

void RotateNode::revert() const
{
    this->node->rotation = this->old_rotation;
}


ScaleNode::ScaleNode(std::shared_ptr<Node> _node, glm::vec2 _new_scale, glm::vec2 _old_scale): node{_node}, new_scale{_new_scale}, old_scale{_old_scale} {}

void ScaleNode::apply() const
{
    this->node->scale = this->new_scale;
}

void ScaleNode::revert() const
{
    this->node->scale = this->old_scale;
}



void Viewport::new_selection(std::shared_ptr<Node> node)
{
    this->reset_selection();
    this->add_selection(std::move(node));
}

void Viewport::add_selection(std::shared_ptr<Node> node)
{
    node_tree->add_selection(node);
    this->selected_nodes.push_back(SelectedNode{std::move(node)});
}

void Viewport::reset_selection()
{
    node_tree->reset_selection();
    this->selected_nodes.clear();
}
