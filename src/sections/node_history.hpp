#pragma once

// local
#include "node_tree.hpp"



template <typename V>
class NodeMemberEdit final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        V& node_member;
        
        const V old_value;
        const V new_value;

    public:

        NodeMemberEdit(std::shared_ptr<Node> _node, V& _node_member, const V _old_value, const V _new_value)
        : node{_node}, node_member{_node_member}, old_value{_old_value}, new_value{_new_value}
        {
            leaf_assert((void*)&this->node_member >= (void*)this->node.get());
            leaf_assert((void*)&this->node_member < (void*)(this->node.get() + 1));
        }

        virtual void apply() const override
        {
            this->node_member = this->new_value;
        }

        virtual void revert() const override
        {
            this->node_member = this->old_value;
        }
};


template <Track track, typename instant_t = get_track_type_t<track>>
class NodeKeyframeInsert final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        instant_t instant;

    public:

        NodeKeyframeInsert(std::shared_ptr<Node> _node, instant_t _instant): node{_node}, instant{_instant} {}

        virtual void apply() const override
        {
            this->node->keyframe.insert_instant<track>(this->instant);
        }

        virtual void revert() const override
        {
            this->node->keyframe.remove_key<track>(this->instant.time);
        }
};

template <Track track, typename instant_t = get_track_type_t<track>>
class NodeKeyframeRemove final: public Action
{
    private:

        mutable std::shared_ptr<Node> node;
        instant_t instant;

    public:

        NodeKeyframeRemove(std::shared_ptr<Node> _node, instant_t _instant): node{_node}, instant{_instant} {}

        virtual void apply() const override
        {
            this->node->keyframe.remove_key<track>(this->instant.time);
        }

        virtual void revert() const override
        {
            this->node->keyframe.insert_instant<track>(this->instant);
        }
};
