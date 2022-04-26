#pragma once


// builtin
#include <string>
#include <chrono>
#include <unordered_map>

// local
#include "node_tree.hpp"
#include "graphical/opengl/framebuffer.hpp"
#include "graphical/sprite.hpp"



class DepthIndicator
{

    private:

        Framebuffer framebuffer;

        std::vector<std::string> current_nodes_paths;
        size_t current_node_in_focus = 0;
        size_t current_layer = 0;
        std::unordered_map<size_t, size_t> focus_in_layer;
        bool strech = false;

        std::chrono::time_point<std::chrono::steady_clock> last_update_time;

    public:

        DepthIndicator();
        void render();

    private:

        void draw_left(size_t start);
        void draw_right(size_t start);

        void update_current_nodes();
        glm::vec2 get_current_available_window_size();
        
        static glm::vec2 calculate_render_size(const Sprite& sprite, glm::vec2 max_size, bool strech = false);
        static glm::vec2 scale(glm::vec2 max_size, glm::vec2 size);

};
