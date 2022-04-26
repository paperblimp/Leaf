#pragma once

// local
#include "node_tree.hpp"
#include "animation/keyframe.hpp"



namespace Animation
{
    void vector2_transformation(Node& node, const Vector2Instant& instant1, const Vector2Instant& instant2, double current_time);
    void double_transformation (Node& node, const DoubleInstant&  instant1, const DoubleInstant&  instant2, double current_time);
}


void animate(Node& node, double time);

class AnimationData
{
    private:
        double preview_time = 0;

    public: 
        double length = 20;
        bool paused = true;
        bool loop   = false;
    
    public:
        void set_time(const double new_anim_time);
        double get_time();

        void move_time(const double delta_time);
        void update(const double delta_time);
        void call_animate();

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & archive, const unsigned int version)
        {
            archive & preview_time;
            archive & length;
            archive & loop;
        }
};

inline AnimationData anim_data;
