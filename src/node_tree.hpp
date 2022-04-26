#pragma once


// builtin
#include <functional>
#include <algorithm>
#include <optional>
#include <queue>
#include <string>
#include <vector>
#include <string>
#include <ctype.h>
#include <stdlib.h>

// extern
#include "imgui.h"
#include <boost/optional/optional.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/optional.hpp>

// local
#include "dialogs/text_input.hpp"
#include "sections/property_editor.hpp"
#include "animation/keyframe.hpp"
#include "history.hpp"



class Node;
class NodeTree;
class PropertyEditor;

namespace boost::serialization
{

    template<class Archive>
    void serialize(Archive& archive,NodeTree& node_tree ,const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive,Node& node ,const unsigned int _version);
}



struct AddNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node = nullptr;
        std::shared_ptr<Node> destination = nullptr;

    public:

        AddNode(std::shared_ptr<Node> node, std::shared_ptr<Node> destination);
        
        virtual void apply() const override;
        virtual void revert() const override;
};

struct RemoveNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node = nullptr;
        std::shared_ptr<Node> destination = nullptr;

    public:

        RemoveNode(std::shared_ptr<Node> node, std::shared_ptr<Node> destination);
        
        virtual void apply() const override;
        virtual void revert() const override;
};

struct ReaparentNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node = nullptr;
        std::shared_ptr<Node> new_parent = nullptr;
        std::shared_ptr<Node> old_parent = nullptr;

    public:

        ReaparentNode(std::shared_ptr<Node> node, std::shared_ptr<Node> new_parent, std::shared_ptr<Node> old_parent);

        virtual void apply() const override;
        virtual void revert() const override;
};

struct ReorderNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node = nullptr;
        size_t new_idx;
        size_t old_idx;

    public:

        ReorderNode(std::shared_ptr<Node> node, size_t new_idx, size_t old_idx);

        virtual void apply() const override;
        virtual void revert() const override;
};

struct RenameNode final: public Action
{
    private:

        mutable std::shared_ptr<Node> node = nullptr;
        std::string new_name;
        std::string old_name;

    public:

        RenameNode(std::shared_ptr<Node> node, std::string new_name, std::string old_name);

        virtual void apply() const override;
        virtual void revert() const override;
};


inline uint64_t inside_tree_walk = 0;

// TODO: adicionar retorno nas funções que podem falhar informando se ocorreu ou não um erro
class Node: public std::enable_shared_from_this<Node> {

    friend class boost::serialization::access;
    friend NodeTree;
    template<class Archive> friend void boost::serialization::serialize(Archive&, Node&, const unsigned int);

    friend AddNode;
    friend RemoveNode;
    friend ReaparentNode;
    friend ReorderNode;

    private:

        //Node tree related
        std::weak_ptr<Node> parent;
        size_t index;
        std::vector<std::shared_ptr<Node>> children; 

    public:
        
        //General properties
        std::string name;
        bool visible = true;
        bool selected = false;

        //Transform properties
        glm::vec2 position = {0,0};
        glm::vec2 scale = {1, 1};
        double rotation = 0.0;
        glm::vec2 rotation_pivot = {0,0};

        //Graphic properties
        size_t layer = 0;
        boost::optional<std::string> texture_path;

        // keyframe
        KeyFrame keyframe;


    public:
        
        Node(std::string name);
        Node(std::string name, size_t index, std::shared_ptr<Node> parent);
        Node(Node&) = default;
        Node() = default;
        
        size_t get_idx();
        size_t get_child_count();
        Node& get_child(size_t idx);
        std::shared_ptr<Node> get_parent();
        bool is_rootless();

        void add_child(std::string child_name, bool record_action = true);
        void duplicate_child(size_t child_idx, bool record_action = true);
        void remove_children(size_t child_idx, bool record_action = true);
        void reaparent(std::shared_ptr<Node> new_parent, bool record_action = true);
        void reorder_child(size_t old_pos, size_t new_pos, bool record_action = true);
        void rename(std::string new_name, bool record_action = true);
        
        bool child_name_available(const std::string& name);

        void save_all_properties(const double time);
        
        std::string get_next_name(std::string name);

        //TODO: Property loading
        //void load_properties();

        //TODO: Texture load/unload
        //void load_texture  (std::string texture_path);
        //void unload_texture(std::string texture_path);
        
    private:

        void add_child(std::shared_ptr<Node> node, bool record_action = true);
        void update_children_index();

        bool is_child(std::shared_ptr<Node> possible_child);
        glm::vec2 get_pivot_position();

};


class NodeTree
{   

    template<class Archive> friend void boost::serialization::serialize(Archive&, NodeTree&, const unsigned int);

    private:

        std::shared_ptr<Node> root_node;

    public:

        std::vector<std::shared_ptr<Node>> selected_nodes;
        std::optional<PropertyEditor*> property_editor;

    public:

        NodeTree();

        void reset_selection();
        void invert_selection(std::shared_ptr<Node> node);
        void new_selection(std::shared_ptr<Node> node);
        void sub_selection(std::shared_ptr<Node> node);
        void add_selection(std::shared_ptr<Node> node);
        
        Node& get_root_node();

        void run_on_nodes(std::function<void(Node&)> function);
        void run_on_nodes_ordered(std::function<void(Node&)> function);
        void run_on_nodes_ordered_reverse(std::function<void(Node&)> function);
        
        ~NodeTree();

    private:

        void run_on_nodes_pre_order(std::shared_ptr<Node> root, std::function<void(Node&)> function);
        void run_on_nodes_post_order(std::shared_ptr<Node> root, std::function<void(Node&)> function);
};



inline NodeTree* node_tree = new NodeTree{};
