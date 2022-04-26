
// header
#include "animation/animation.hpp"

// local
#include "animation/keyframe.hpp"
#include "utils/math_utils.hpp"
#include "sections/keyframe_widget.hpp"

// extern
#include <glm/ext/vector_float2.hpp>

// builtin
#include <array>



namespace Animation
{

    void vector2_transformation(glm::vec2& property, const Vector2Instant& instant1, const Vector2Instant& instant2, double current_time)
    {
        if(instant1.time == instant2.time)
        {
            property = instant2.vector;
            return;
        }

        auto normalized_time = normalize(current_time, instant1.time, instant2.time);
        auto new_position = glm::vec2 {
            (float)interpolate(
                normalized_time,
                (double)instant1.vector.x,
                (double)instant2.vector.x,
                instant1.easing
            ),
            (float)interpolate(
                normalized_time,
                (double)instant1.vector.y,
                (double)instant2.vector.y,
                instant1.easing
            ),
        };

        property = new_position;
    }

    void double_transformation(double& property, const DoubleInstant& instant1, const DoubleInstant& instant2, double current_time)
    {
        if(instant1.time == instant2.time)
        {
            property = instant2.value;
            return;
        }

        auto normalized_time = normalize(current_time, instant1.time, instant2.time);
        auto new_rotation = interpolate(normalized_time, instant1.value, instant2.value, instant1.easing);

        property = new_rotation;
    }
}



template <typename T>
void assert_keyframe_ordering(const std::vector<T>& keyframe)
{
    double last = -1;
    for (auto& instant: keyframe)
    {
        auto instant_time = instant.time;
        if (!(instant_time >= last))
            panic("invalid keyframe ordering");

        last = instant_time;
    }
}


// TODO: utilizar busca binária ao invés de checar cada instante de forma linear
template <typename T>
std::optional<std::pair<const T*, const T*>> get_current_instant_pair(double current_time, const std::vector<T>& keyframe)
{
    size_t idx = 0;

    for (; idx < keyframe.size(); ++idx)
        if (keyframe[idx].time > current_time)
            break;

    
    if (idx == 0)
        return std::nullopt;
    else if (idx == keyframe.size()) 
        return std::pair{&keyframe[idx-1], &keyframe[idx-1]};
    else
        return std::pair{&keyframe[idx - 1], &keyframe[idx]};
}


template <typename T, typename P>
using Transformation = void(*)(P&, const T&, const T&, double);

template <typename T, typename P>
void transform(P& property, const std::vector<T>& keyframe, double current_time, Transformation<T, P> transformation)
{
    // assert instants ordering
    #ifndef NDEBUG
    assert_keyframe_ordering(keyframe);
    #endif

    auto instant_pair = get_current_instant_pair(current_time, keyframe);

    if (!instant_pair.has_value())
        return;

    const T& instant1 = *instant_pair.value().first;
    const T& instant2 = *instant_pair.value().second;

    transformation(property, instant1, instant2, current_time);
}


void animate(Node& node, double time)
{
    // position transformation
    transform(node.position, node.keyframe.get_track<Track::POSITION>(), time, Animation::vector2_transformation);

    // rotation transformation
    transform(node.rotation, node.keyframe.get_track<Track::ROTATION>(), time, Animation::double_transformation);

    // scale transformation
    transform(node.scale, node.keyframe.get_track<Track::SCALE>(), time, Animation::vector2_transformation);

    //Pivot transformation
    transform(node.rotation_pivot, node.keyframe.get_track<Track::PIVOT>(), time, Animation::vector2_transformation);
}



void AnimationData::set_time(const double new_time)
{
    if(new_time >= 0 && new_time <= this->length)
    {
        this->preview_time = new_time;
    }
    this->call_animate();
}

double AnimationData::get_time()
{
    return this->preview_time;
}


void AnimationData::move_time(const double delta_time)
{   //Move animation delta_time in the timeline
    this->preview_time += delta_time;

    if(this->preview_time > this->length)
    {
        if(this->loop) //Reset time on loop
        {
            this->preview_time -= this->length;
        }
        else     //Stop outside loop
        {
            this->paused = true;
            this->preview_time = this->length;
        }
    }
    else if(this->preview_time < 0 )
    {
        if(this->loop)
        {
            this->preview_time += this->length;
        }
        else
        {
            this->paused = true;
            this->preview_time = 0;
        }
    }

    this->call_animate();
}

void AnimationData::update(const double delta_time)
{   
    if(!this->paused) this->move_time(delta_time);
}

void AnimationData::call_animate()
{
    node_tree->run_on_nodes([](Node& node){ animate(node, anim_data.get_time() );});
}