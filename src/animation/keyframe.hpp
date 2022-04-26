#pragma once


// builtin
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <optional>

// local
#include "animation/easings.hpp"
#include "utils/log.hpp"

// extern
#include <glm/vec2.hpp>
#include <fmt/core.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>



enum class Track
{
    POSITION = 0,
    SCALE,
    ROTATION,
    PIVOT
};


using Easing = double(*)(double);


struct Vector2Instant
{
    Vector2Instant();
    Vector2Instant(const double time, const glm::vec2 vector);
    Vector2Instant(const double time,const glm::vec2& vector,Easing easing);
    Vector2Instant(const double time,const float   vector[2],const std::string& easing);
    Vector2Instant(const double time,const glm::vec2& vector,const std::string& easing);

    double time;
    glm::vec2 vector;
    Easing easing;
};


struct DoubleInstant
{
    DoubleInstant();
    DoubleInstant(const double time,const double value,Easing easing);
    DoubleInstant(const double time,const float* value,const std::string& easing);
    DoubleInstant(const double time,const double value,const std::string& easing);

    double time;
    double value;
    Easing easing;

};


template <Track track>
struct get_track_type;

template <Track track>
using get_track_type_t = typename get_track_type<track>::type;

template <>
struct get_track_type<Track::POSITION> { using type = Vector2Instant; inline static const char* name = "position"; };

template <>
struct get_track_type<Track::SCALE> { using type = Vector2Instant; inline static const char* name = "scale"; };

template <>
struct get_track_type<Track::ROTATION> { using type = DoubleInstant; inline static const char* name = "rotation"; };

template <>
struct get_track_type<Track::PIVOT> { using type = Vector2Instant; inline static const char* name = "pivot"; };



class KeyFrame;

namespace boost::serialization
{

    template<class Archive>
    void serialize(Archive& archive, KeyFrame& node_tree, const unsigned int);
}


// TODO: impedir a presença de mais de um instante no mesmo momento
class KeyFrame
{   
    template<class Archive> friend void boost::serialization::serialize(Archive& archive, KeyFrame& node_tree, const unsigned int);
    friend class boost::serialization::access;

    private:

        std::vector<Vector2Instant> position;
        std::vector<Vector2Instant> rot_pivot;
        std::vector<Vector2Instant> scale;
        std::vector<DoubleInstant> rotation;
        
    private:
        
        KeyFrame(const Vector2Instant& position, const Vector2Instant& rot_pivot,
                 const Vector2Instant& scale   , const DoubleInstant&  rotation );

        template <Track track>
        std::vector<get_track_type_t<track>>& _get_track()
        {
            if constexpr (track == Track::POSITION)
                return this->position;

            else if constexpr (track == Track::SCALE)
                return this->scale;

            else if constexpr (track == Track::ROTATION)
                return this->rotation;
            
            else if constexpr (track == Track::PIVOT)
                return this->rot_pivot;
            
            else
                panic("invalid track");
        }

    public:

        KeyFrame();

        template <Track track, typename track_type = get_track_type_t<track>>
        std::optional<track_type> remove_key(double time)
        {
            const auto predicate = [time](const auto& instant) { return instant.time == time; };
            auto& keyframe = this->_get_track<track>();

            if (auto idx = std::find_if(keyframe.begin(), keyframe.end(), predicate); idx != keyframe.end())
            {
                auto instant = *idx;
                keyframe.erase(idx);
                return instant;
            }
            else
                return std::nullopt;
        }

        template <Track track, typename track_type = get_track_type_t<track>>
        void insert_instant(track_type instant)
        {
            auto& keyframe = this->_get_track<track>();
            auto position = std::find_if(keyframe.begin(), keyframe.end(), [time = instant.time](const track_type& instant){ return instant.time > time; });
            keyframe.insert(position, instant);
        }

        template <Track track>
        const std::vector<get_track_type_t<track>>& get_track()
        {
            return this->_get_track<track>();
        }

        template <Track track, typename instant_t = get_track_type_t<track>>
        std::optional<instant_t*> get_instant(double time)
        {
            const auto predicate = [time](const auto& instant) { return instant.time == time; };
            auto& keyframe = this->_get_track<track>();

            if (auto idx = std::find_if(keyframe.begin(), keyframe.end(), predicate); idx != keyframe.end())
                return &*idx;
            else
                return std::nullopt;
        }
};
