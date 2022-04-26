
// local
#include "node_tree.hpp"
#include "animation/keyframe.hpp"
#include "utils/log.hpp"
#include "utils/asserts.hpp"
#include "utils/serialization.hpp"

// extern
#include <fmt/core.h>
#include <glm/ext/vector_float2.hpp>
#include <imgui.h>

// builtin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>



Node::Node(std::string _name): name{_name} {}

Node::Node(std::string name, size_t index, std::shared_ptr<Node> parent)
 : parent{parent}, index{index}, name{std::move(name)}
{

}


void Node::add_child(std::string child_name, bool record_action)
{
    leaf_assert(inside_tree_walk == 0);

    if (this->child_name_available(child_name) == false)
        child_name = this->get_next_name(child_name);
    
    auto new_child = std::make_shared<Node>(child_name, children.size(), this->shared_from_this());
    
    if (record_action == true)
    {
        auto action = std::make_unique<AddNode>(new_child, this->shared_from_this());
        history->push_action(std::move(action));
    }
    
    children.push_back(new_child);
}

void Node::add_child(std::shared_ptr<Node> node, bool record_action)
{
    leaf_assert(inside_tree_walk == 0);

    if (this->child_name_available(node->name) == false)
        node->name = this->get_next_name(node->name);


    if (record_action == true)
    {
        auto action = std::make_unique<AddNode>(node, this->shared_from_this());
        history->push_action(std::move(action));
    }

    children.push_back(node);
    node->parent = this->shared_from_this();
    node->index = children.size()-1;
}

void Node::duplicate_child(size_t child_idx, bool record_action)
{
    leaf_assert(inside_tree_walk == 0);

    auto new_child = std::make_shared<Node>(*this->children.at(child_idx));
    new_child->selected = false;

    // update name
    new_child->name = this->get_next_name(new_child->name);


    if (record_action == true)
    {
        auto action = std::make_unique<AddNode>(new_child, this->shared_from_this());
        history->push_action(std::move(action));
    }

    this->add_child(new_child, false);
    this->update_children_index();
}


void Node::remove_children(size_t child_idx, bool record_action)
{
    leaf_assert(inside_tree_walk == 0);

    auto& child = this->children[child_idx];

    if (record_action == true)
    {
        auto action = std::make_unique<RemoveNode>(child, this->shared_from_this());
        history->push_action(std::move(action));
    }

    this->children.erase(this->children.begin() + child_idx);
    update_children_index();
}

void Node::reaparent(std::shared_ptr<Node> new_parent, bool record_action)
{   
    leaf_assert(inside_tree_walk == 0);
    
    if (this->parent.lock() == new_parent || this->is_child(new_parent) || this->shared_from_this() == new_parent)
        return;

    if (new_parent->child_name_available(this->name) == false)
        this->name = new_parent->get_next_name(this->name);

    if (record_action == true)
    {
        auto action = std::make_unique<ReaparentNode>(this->shared_from_this(), new_parent, this->parent.lock());
        history->push_action(std::move(action));
    }

    this->parent.lock()->children.erase(this->parent.lock()->children.begin() + this->index);
    this->update_children_index();
    new_parent->add_child(this->shared_from_this(), false);
}

void Node::reorder_child(size_t old_pos, size_t new_pos, bool record_action)
{
    leaf_assert(inside_tree_walk == 0);
    
    leaf_assert(old_pos < this->children.size());
    leaf_assert(new_pos < this->children.size());

    if (record_action == true)
    {
        auto action = std::make_unique<ReorderNode>(this->shared_from_this(), new_pos, old_pos);
        history->push_action(std::move(action));
    }

    std::swap(this->children[old_pos], this->children[new_pos]);
    this->update_children_index();
}

void Node::rename(std::string new_name, bool record_action)
{
    if (record_action == true)
    {
        auto action = std::make_unique<RenameNode>(this->shared_from_this(), new_name, this->name);
        history->push_action(std::move(action));
    }

    this->name = new_name;
}

void Node::update_children_index()
{
    for(size_t i= 0; i < children.size(); i++)
        children[i]->index = i;
}

bool Node::is_child(std::shared_ptr<Node> possible_child)
{
    for (auto child: children)
    {
        if(possible_child == child)
            return true;

        if(child->is_child(possible_child) == true)
            return true; 
    }

    return false;
}

size_t Node::get_child_count()
{
    return this->children.size();
}

Node& Node::get_child(size_t idx)
{
    return *this->children.at(idx);
}

size_t Node::get_idx()
{
    return this->index;
}

std::shared_ptr<Node> Node::get_parent()
{
    return this->parent.lock();
}

bool Node::is_rootless()
{
    return this->parent.expired();
}

bool Node::child_name_available(const std::string& name)
{
    for(size_t i = 0; i < children.size();i++)
        if(name == children[i]->name)
            return false;

    return true;
}

glm::vec2 Node::get_pivot_position()
{
    return {this->position.x + this->rotation_pivot[0], this->position.y + this->rotation_pivot[1]};
}


void Node::save_all_properties(const double time)
{
    //Save all properities as keys in the Keyframe

    auto position_instant = Vector2Instant{time, this->position, Easings::linear};
    this->keyframe.insert_instant<Track::POSITION>(position_instant);
    
    auto scale_instant = Vector2Instant{time, this->scale, Easings::linear};
    this->keyframe.insert_instant<Track::SCALE>(scale_instant);
    
    auto rotation_instant = DoubleInstant{time, this->rotation, Easings::linear};
    this->keyframe.insert_instant<Track::ROTATION>(rotation_instant);
    
    auto pivot_instant = Vector2Instant{time, this->rotation_pivot, Easings::linear};
    this->keyframe.insert_instant<Track::PIVOT>(pivot_instant);
}

std::string Node::get_next_name(std::string name)
{
    do {
        size_t first_tail_number = name.size();
        while (std::isdigit(name[first_tail_number - 1]))
            first_tail_number -= 1;

        if (first_tail_number == name.size())
            return name + "1";

        auto name_without_number = std::string{name.begin(), name.begin() + first_tail_number};
        auto number = std::stoi(std::string{name.begin() + first_tail_number, name.end()});

        number += 1;
        name = name_without_number + std::to_string(number);
    }
    while (this->child_name_available(name) == false);

    return name;
}










NodeTree::NodeTree()
{
    this->root_node = std::make_shared<Node>("Root");
}

NodeTree::~NodeTree() = default;


void NodeTree::reset_selection()
{
    for (auto node : selected_nodes)
        node->selected = false;

    selected_nodes.clear();
}

void NodeTree::new_selection(std::shared_ptr<Node> node)
{
    reset_selection();
    add_selection(node);
}

void NodeTree::add_selection(std::shared_ptr<Node> node)
{
    node->selected = true;
    selected_nodes.push_back(node);
}

void NodeTree::invert_selection(std::shared_ptr<Node> node)
{
    if (node->selected)
        sub_selection(node);
    else
        add_selection(node);
}

void NodeTree::sub_selection(std::shared_ptr<Node> node)
{   
    node->selected = false;
    for(size_t i = 0; i < selected_nodes.size(); i++)
    {
        if(selected_nodes[i] == node)
        {
            selected_nodes.erase(selected_nodes.begin() + i);
            break;
        }
    }
}

void NodeTree::run_on_nodes(std::function<void(Node&)> function)
{
    inside_tree_walk += 1;
    this->run_on_nodes_pre_order(this->root_node, std::move(function));
    inside_tree_walk -= 1;
}

void NodeTree::run_on_nodes_pre_order(std::shared_ptr<Node> node, std::function<void(Node&)> function)
{
    function(*node);

    if (node->children.empty())
        return;

    for (auto& node: node->children)
    {
        function(*node);
        this->run_on_nodes_pre_order(node, function);
    }
}

void NodeTree::run_on_nodes_post_order(std::shared_ptr<Node> node, std::function<void(Node&)> function)
{
    function(*node);

    if (node->children.empty())
        return;

    for (auto& node: node->children)
    {
        this->run_on_nodes_post_order(node, function);
        function(*node);
    }
}

void NodeTree::run_on_nodes_ordered(std::function<void(Node&)> function)
{
    uint64_t node_count = 0;
    uint64_t deepest_layer = 0;

    this->run_on_nodes([&](Node& node)
    {   
        ++node_count;

        if (node.layer > deepest_layer)
            deepest_layer = node.layer;
    });

    size_t layer = 0;
    
    while (node_count != 0)
    {
        node_tree->run_on_nodes_pre_order(this->root_node, [&](Node& node)
        {
            if (node.layer != layer)
                return;
            else
                --node_count;

            function(node);
        });
        
        ++layer;
    }
}

void NodeTree::run_on_nodes_ordered_reverse(std::function<void(Node&)> function)
{
    uint64_t node_count = 0;
    uint64_t deepest_layer = 0;

    this->run_on_nodes([&](Node& node)
    {   
        ++node_count;

        if (node.layer > deepest_layer)
            deepest_layer = node.layer;
    });

    size_t layer = deepest_layer;
    
    while (node_count != 0)
    {
        node_tree->run_on_nodes_post_order(this->root_node, [&](Node& node)
        {
            if (node.layer != layer)
                return;
            else
                --node_count;

            function(node);
        });
        
        --layer;
    }
}

Node& NodeTree::get_root_node()
{
    return *this->root_node;
}



AddNode::AddNode(std::shared_ptr<Node> _node, std::shared_ptr<Node> _destination): node{_node}, destination{_destination} {}

void AddNode::apply() const
{
    this->destination->add_child(this->node, false);
}

void AddNode::revert() const
{
    this->destination->remove_children(this->node->get_idx(), false);
}

RemoveNode::RemoveNode(std::shared_ptr<Node> _node, std::shared_ptr<Node> _destination): node{_node}, destination{_destination} {}


void RemoveNode::apply() const
{
    this->destination->remove_children(this->node->get_idx(), false);
}

void RemoveNode::revert() const
{
    this->destination->add_child(this->node, false);
}


ReaparentNode::ReaparentNode(std::shared_ptr<Node> _node, std::shared_ptr<Node> _new_parent, std::shared_ptr<Node> _old_parent)
: node{_node}, new_parent{_new_parent}, old_parent{_old_parent} {}

void ReaparentNode::apply() const
{
    this->old_parent->remove_children(this->node->index, false);
    this->new_parent->add_child(this->node, false);
}

void ReaparentNode::revert() const
{
    this->new_parent->remove_children(this->node->index, false);
    this->old_parent->add_child(this->node, false);
}


ReorderNode::ReorderNode(std::shared_ptr<Node> _node, size_t _new_idx, size_t _old_idx): node{_node}, new_idx{_new_idx}, old_idx{_old_idx} {}

void ReorderNode::apply() const
{
    this->node->reorder_child(this->old_idx, this->new_idx, false);
}

void ReorderNode::revert() const
{
    this->node->reorder_child(this->old_idx, this->new_idx, false);
}


RenameNode::RenameNode(std::shared_ptr<Node> _node, std::string _new_name, std::string _old_name): node{_node}, new_name{_new_name}, old_name{_old_name} {}

void RenameNode::apply() const
{
    this->node->rename(this->new_name, false);
}

void RenameNode::revert() const
{
    this->node->rename(this->old_name, false);
}
