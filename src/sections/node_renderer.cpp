#include "node_renderer.hpp"
#include "node_tree.hpp"
#include <fmt/core.h>
#include <imgui.h>
#include <optional>

#include "graphical/custom_widgets.hpp"



void NodeRenderer::render()
{
    ImGui::Begin("##Node Tree",NULL,SECTION_FLAGS);
    CustomImGui::Title("Node Tree");


    //Reset flags
    captured_mouse_click = false;
    dragging = false;

    //Start the render process on the root that will call on all nodes of the tree
    render_node(node_tree->get_root_node());
    this->drag_check_list.clear();

    ImGui::End();
}

void NodeRenderer::node_drag_source(Node& node)
{
    if (node.is_rootless() == false && ImGui::BeginDragDropSource())
    {
        auto ptr_size = sizeof(std::shared_ptr<Node>); // NOLINT
        ImGui::SetDragDropPayload("node", &node, ptr_size);
        selected_to_reaparent = node.shared_from_this();

        ImGui::Text("%s", node.name.c_str());
        ImGui::EndDragDropSource();
    }
}

void NodeRenderer::drop_target(Node& node)
{
    if (ImGui::BeginDragDropTarget())
    {   
        //Node
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("node");
        if (payload != nullptr && selected_to_reaparent.has_value())
        {
            auto source_node = selected_to_reaparent.value();
            selected_to_reaparent = std::nullopt;

            source_node->reaparent(node.shared_from_this());
        }

        //Sprite resource
        payload = ImGui::AcceptDragDropPayload("sprite");
        if (payload != nullptr)
        {
            auto sprite_path = std::string((char*)payload->Data, payload->DataSize);
            node.texture_path = sprite_path;

        }

        ImGui::EndDragDropTarget();
    }
}


void NodeRenderer::handle_input(Node& node,bool mouse_click[2])
{
    ImGuiIO io = ImGui::GetIO();
    ////Keyboard only

    ////Mouse
    //Return if an child already captured the click on this frame
    if(captured_mouse_click) return;

    if(mouse_click[0]) //Left button
    {   
        captured_mouse_click = true;

        //TODO: reset the selected_to_reaparent when button is pressed
        //Reaparent target
        /*
        if(selected_to_reaparent.has_value())
        {

            selected_to_reaparent.value()->reaparent(node);
            selected_to_reaparent = std::nullopt;
        }*/
        
        //Invert selection
        if(ImGui::GetIO().KeyCtrl) node_tree->invert_selection(node.shared_from_this());
        
        //Simple selection
        else node_tree->new_selection(node.shared_from_this());
    }

    else if(mouse_click[1]) //Right button
    {   
        captured_mouse_click = true;

        // open node action menu
        node_menu.open(node.shared_from_this());

        // update selection list
        node_tree->new_selection(node.shared_from_this());
    }

    if (this->node_menu.get_current_node() == node.shared_from_this())
        this->node_menu.render();
}

void NodeRenderer::get_mouse_click(bool mouse_click[2])
{
    //Check if there was a mouse click on the latest node rendered
    for (int c=0;c<2;c++)
    {
        mouse_click[c] = ImGui::IsItemClicked(c);
    }
}

void NodeRenderer::render_node(Node& node)
{
    

    bool mouse_click[2]{false,false};
    if(node.get_child_count() > 0) //>0 children = Tree widget
    {
        ImGuiTreeNodeFlags flags = TREE_FLAGS;
        if(node.selected) flags = TREE_FLAGS | ImGuiTreeNodeFlags_Selected;
        if(ImGui::TreeNodeEx(node.name.c_str(),flags))
        {      
            node_drag_source(node);

            if (this->drag_check_list.find(node.shared_from_this()) == this->drag_check_list.end())
                drop_target(node);

            get_mouse_click(mouse_click);


            for(size_t i=0;i < node.get_child_count();i++)
                render_node(node.get_child(i));

            ImGui::TreePop();
        }
    }
    else //0 children = Leaf Widget
    { 
        ImGuiTreeNodeFlags flags = LEAF_FLAGS;
        if(node.selected ) flags |= ImGuiTreeNodeFlags_Selected;
        if(ImGui::TreeNodeEx(node.name.c_str(),flags))
        {
            node_drag_source(node);
            
            if (this->drag_check_list.find(node.shared_from_this()) == this->drag_check_list.end())
                drop_target(node);

            get_mouse_click(mouse_click);

        }

    }

    
    this->drag_check_list.insert(node.shared_from_this());
    handle_input(node,mouse_click);
}



NodeMenu::NodeMenu(const std::string _id): id{std::move(_id)} {}

void NodeMenu::open(std::shared_ptr<Node> node)
{
    this->node = std::move(node);
    ImGui::OpenPopup(this->id.c_str());
}

void NodeMenu::close()
{
    ImGui::CloseCurrentPopup();
    this->node = std::nullopt;
}

const std::optional<std::shared_ptr<Node>>& NodeMenu::get_current_node()
{
    return this->node;
}

void NodeMenu::render()
{
    if (this->node.has_value() == false)
        return;

    auto node = this->node.value();

    if(ImGui::BeginPopup(this->id.c_str()))
    {   
        // Add child option
        if(ImGui::Button("Add Child"))
        {
            add_child_input.open("");
        }

        if (auto result = add_child_input.run(); result.runned() && result.has_value())
        {
            node->add_child(result.value().c_str());
            this->close();
        }

        //TODO: Error message if the new name was already used
        //Rename option
        if(ImGui::Button("Rename"))
        {
            rename_input.open("");
        }

        if (auto result = rename_input.run(); result.runned() && result.has_value())
        {
            node->rename(result.value());
            this->close();
        }

        //Non-root exclusive options
        if (node->is_rootless() == false)
        {
            //Duplicate
            if(ImGui::Button("Duplicate"))
            {
                node->get_parent()->duplicate_child(node->get_idx());
                this->close();
            }

            //Reorder
            if(node->get_idx() < (node->get_parent()->get_child_count() - 1) && ImGui::Button("Move down"))
            {
                node->get_parent()->reorder_child(node->get_idx(), node->get_idx() + 1);
                this->close();
            }
            if(node->get_idx() != 0 && ImGui::Button("Move up"))
            {
                node->get_parent()->reorder_child(node->get_idx(), node->get_idx()-1);
                this->close();
            }

            //Delete option
            if(ImGui::Button("Delete"))
            {
                node->get_parent()->remove_children(node->get_idx());
                this->close();
            }
        }

        ImGui::EndPopup();
    }
    else
    {
        this->node = std::nullopt;
    }
}
