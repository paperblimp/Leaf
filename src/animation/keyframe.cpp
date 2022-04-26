
// builtin
#include <optional>

// local
#include "keyframe.hpp"
#include "animation/easings.hpp"



Vector2Instant::Vector2Instant()
{}

Vector2Instant::Vector2Instant(const double _time, const glm::vec2 _vector): time{_time}, vector{_vector} {}

Vector2Instant::Vector2Instant(const double _time, const glm::vec2& _vector, const std::string& _easing)
: time{_time},vector{_vector},easing{easings.at(_easing)}
{}

 Vector2Instant::Vector2Instant(const double _time, const float _vector[2], const std::string& _easing)
: time{_time},vector{_vector[0],_vector[1]},easing{easings.at(_easing)}
 {}

Vector2Instant::Vector2Instant(const double _time, const glm::vec2& _vector, Easing _easing)
: time{_time},vector{_vector},easing{_easing}
{}


DoubleInstant::DoubleInstant()
{}

DoubleInstant::DoubleInstant(const double _time, const double _value, Easing _easing)
:time{_time},value{_value},easing{_easing}
{}

DoubleInstant::DoubleInstant(const double _time, const double _value, const std::string& _easing)
:time{_time},value{_value},easing{easings.at(_easing)}
{}

DoubleInstant::DoubleInstant(const double _time, const float* _value, const std::string& _easing)
:time{_time},value{*_value},easing{easings.at(_easing)}
{}


KeyFrame::KeyFrame()
{}

KeyFrame::KeyFrame(const Vector2Instant& position, const Vector2Instant& rot_pivot,
                   const Vector2Instant& scale   , const DoubleInstant&  rotation )
:position{position},rot_pivot{rot_pivot},scale{scale},rotation{rotation}
{}
