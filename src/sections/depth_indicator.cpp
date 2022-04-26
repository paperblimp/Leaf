// header
#include "depth_indicator.hpp"

// local
#include "graphical//opengl/render.hpp"



const size_t PADDING = 10;



DepthIndicator::DepthIndicator(): framebuffer{0, 0}
{
    this->last_update_time = std::chrono::steady_clock::now() - std::chrono::seconds(666);
}

void DepthIndicator::render()
{
    ImGui::Begin("Depth Indicator");


    bool should_update = false;
    leaf_assert(current_node_in_focus < SIZE_MAX);

    static int v = 0;
    if (ImGui::InputInt("layer", &v))
    {   
        this->current_layer = v;
        should_update = true;

        this->current_node_in_focus = this->focus_in_layer[this->current_layer];
    }

    ImGui::SameLine();
    
    if (ImGui::Button("left"))
    {
        if (current_node_in_focus > 0)
        {
            should_update = true;
            this->current_node_in_focus -= 1;
            this->focus_in_layer[this->current_layer] = this->current_node_in_focus;
        }
    }

    ImGui::SameLine();
    
    if (ImGui::Button("right"))
    {
        if ((current_node_in_focus) + 1 < this->current_nodes_paths.size())
        {
            should_update = true;
            this->current_node_in_focus += 1;
            this->focus_in_layer[this->current_layer] = this->current_node_in_focus;
        }
    }

    ImGui::Checkbox("strech", &this->strech);


    if (std::chrono::steady_clock::now() - this->last_update_time > std::chrono::milliseconds(500))
    {
        this->last_update_time = std::chrono::steady_clock::now();
        should_update = true;
    }

    if (should_update)
        this->update_current_nodes();

    if (this->current_node_in_focus >= this->current_nodes_paths.size())
        this->current_node_in_focus = this->current_nodes_paths.size() / 2;



    auto current_framebuffer_size = this->framebuffer.get_size();
    auto win_size = this->get_current_available_window_size();
    if ((glm::vec2)current_framebuffer_size != win_size)    
        this->framebuffer.resize(win_size.x, win_size.y);

    this->framebuffer.clear({0, 0, 0, 0});


    if (this->current_nodes_paths.size() > 0)
    {
        auto& center_sprite = sprite_manager.get_sprite(this->current_nodes_paths[this->current_node_in_focus]);
        glm::vec2 render_size = DepthIndicator::calculate_render_size(center_sprite, {win_size.x, win_size.y - (PADDING * 2)}, this->strech);

        render_sprite(
            center_sprite,
            win_size / glm::vec2{2, 2},
            render_size,
            0,
            this->framebuffer
        );

        this->draw_left((size_t)((win_size / glm::vec2{2, 2}) - (render_size / glm::vec2{2, 2})).x);
        this->draw_right((size_t)((win_size / glm::vec2{2, 2}) + (render_size / glm::vec2{2, 2})).x);
    }    

    auto texture = this->framebuffer.get_texture_id();
    ImGui::Image((void*)(uintptr_t)texture,  {(float)win_size.x, (float)win_size.y});

    ImGui::End();
}


void DepthIndicator::draw_left(size_t start)
{
    int64_t texture_index = this->current_node_in_focus - 1;
    auto framebuffer_size = this->framebuffer.get_size();

    while (true)
    {
        if (texture_index < 0)
            break;

        auto& texture = sprite_manager.get_sprite(this->current_nodes_paths[texture_index]);
        auto render_size = DepthIndicator::calculate_render_size(texture, glm::vec2{start - PADDING, framebuffer_size.y - (PADDING * 2)}, this->strech);
        render_size *= glm::vec2{0.75, 0.75};
        
        auto render_position = glm::vec2{start - (((render_size.x / 2) + PADDING)), framebuffer_size.y / 2};

        render_sprite(
            texture,
            render_position,
            render_size,
            0,
            this->framebuffer
        );

        start = start - (render_size.x + PADDING);
        texture_index -= 1;
    }
}

void DepthIndicator::draw_right(size_t start)
{
    uint64_t texture_index = this->current_node_in_focus + 1;
    auto framebuffer_size = this->framebuffer.get_size();

    while (true)
    {
        if (texture_index >= this->current_nodes_paths.size())
            break;

        auto& texture = sprite_manager.get_sprite(this->current_nodes_paths[texture_index]);
        auto render_size = DepthIndicator::calculate_render_size(texture, glm::vec2{start + PADDING, framebuffer_size.y - (PADDING * 2)});
        render_size *= glm::vec2{0.75, 0.75};
        
        auto render_position = glm::vec2{start + (((render_size.x / 2) + PADDING)), framebuffer_size.y / 2};

        render_sprite(
            texture,
            render_position,
            render_size,
            0,
            this->framebuffer
        );

        start = start + (render_size.x + PADDING);
        texture_index += 1;
    }
}



void DepthIndicator::update_current_nodes()
{
    std::vector<std::shared_ptr<Node>> nodes;


    node_tree->run_on_nodes([&](Node& node){

        if (node.texture_path.has_value() && node.layer == this->current_layer)
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

    this->current_nodes_paths.clear();

    for (auto node: nodes)
        this->current_nodes_paths.push_back(node->texture_path.value());
}

glm::vec2 DepthIndicator::get_current_available_window_size()
{
    ImVec2 window_available_size = ImGui::GetWindowSize();
    window_available_size.x -= ImGui::GetCursorPosX();
    window_available_size.y -= ImGui::GetCursorPosY() + 10;

    return {window_available_size.x, window_available_size.y};
}

glm::vec2 DepthIndicator::scale(const glm::vec2 max_size, glm::vec2 size)
{
    const auto size_proportion = size.x / size.y;
    const auto max_size_proportion = max_size.x / max_size.y;

    if (size_proportion < max_size_proportion)
        return {max_size.y * size_proportion, max_size.y};
    else
        return {max_size.x, max_size.x * size_proportion};
}

glm::vec2 DepthIndicator::calculate_render_size(const Sprite& sprite, glm::vec2 max_size, bool strech)
{
    glm::vec2 render_size = sprite.size;
    if (strech || render_size.x > max_size.x || render_size.y > max_size.y)
        render_size = DepthIndicator::scale(max_size, render_size);

    return render_size;
}
