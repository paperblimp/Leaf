#pragma once

// extern
#include "imgui.h"
#include "fmt/core.h"

// local
#include "node_tree.hpp"
#include "sections/property_editor.hpp"
#include "sections/section.hpp"

// builtin
#include <optional>
#include <unordered_set>



struct NodeMenu
{

    private:

        TextInput add_child_input{"Create node", "Name"};
        TextInput rename_input{"Rename", "Name"};

        const std::string id;
        std::optional<std::shared_ptr<Node>> node;

    public:

        NodeMenu(const std::string id);

        void open(std::shared_ptr<Node> node);
        void render();
        const std::optional<std::shared_ptr<Node>>& get_current_node();

    private:

        void close();

};


class NodeRenderer : public Section
{

    NodeMenu node_menu = NodeMenu{"node_renderer_menu"};

    const ImGuiTreeNodeFlags TREE_FLAGS = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    const ImGuiTreeNodeFlags LEAF_FLAGS = TREE_FLAGS | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::optional<std::shared_ptr<Node>> selected_to_reaparent;
    bool dragging = false;
    std::unordered_set<std::shared_ptr<Node>> drag_check_list;

    bool captured_mouse_click;

    void get_mouse_click(bool mouse_click[2]);
    void handle_input(Node& node,bool mouse_click[2]);

    void render_reorder_separator(std::shared_ptr<Node> father,int index);
    void render_node(Node& node);

    void node_drag_source(Node& node);
    void drop_target(Node& node);
    
    public:

        void render();
};
