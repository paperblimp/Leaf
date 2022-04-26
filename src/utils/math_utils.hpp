#pragma once

// extern
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <fmt/core.h>

// builtin
#include <limits>
#include <stdint.h>
#include <vector>
#include <array>



template <typename number_T,size_t point_count>
void rotate_figure(
    std::array<glm::tvec3<number_T>,point_count> &local_points,const number_T angle, 
    const glm::tvec3<number_T> &pivot = glm::tvec3<number_T>{0,0,0})
{   
    //Receive a vector of points representing a figure and rotate then in the given angle and around the given pivot
    //The points MUST be in local coordinates, as it'll rotate around the origin(0,0)
    
    const glm::tvec3<number_T> axis{0,0,1};

    for(size_t i=0;i<point_count;i++)
    {
        //Translate the bound in the opposite direction of the pivot in order to rotate around it
        //The translation is undone after the rotation
        local_points[i] -= pivot;

        local_points[i] = glm::rotate(local_points[i],angle,axis);

        local_points[i] += pivot;
    }
}

template <typename number_T,size_t point_count>
void scale_figure(
    std::array<glm::tvec3<number_T>,point_count> &local_points,const glm::tvec3<number_T> &factor)
{
    //Scale the bounds of the figure with the given factor
    //The points MUST be in local coordinates
    for(size_t i=0;i<point_count;i++)
    {
        local_points[i] = glm::tvec3<number_T>
        {
            local_points[i].x *= factor.x,
            local_points[i].y *= factor.y,
            0
        };
    }
}

template <typename number_T>
number_T rad2deg(const number_T rad)
{
    return rad / (glm::pi<number_T>()/180.0);
}

template <typename number_T>
number_T deg2rad(const number_T deg)
{
    return deg * (glm::pi<number_T>()/180.0);
}


template <typename number_T>
number_T normalize(number_T value, number_T min, number_T max)
{
    return (value-min) / (max-min);
}

template <typename number_T>
number_T interpolate(number_T normalized_value, number_T start, number_T target, number_T (*ease_function)(number_T))
{   
    const number_T eased_value = ease_function(normalized_value);
    return start + ((target-start) * eased_value);
}

template <typename number_T>
number_T linear_interpolation(number_T normalized_value, number_T start, number_T target)
{   
    return start + ((target-start) * normalized_value);
}


template <typename number_T>
glm::tvec3<number_T> linear_interpolation_vector(number_T normalized_value, glm::tvec3<number_T> start , glm::tvec3<number_T> target, number_T (*ease_function)(number_T))
{   
    return glm::tvec3<number_T> 
    {
        interpolate(normalized_value, start.x, target.x, ease_function),
        interpolate(normalized_value, start.y, target.y, ease_function),
        interpolate(normalized_value, start.z, target.z, ease_function),
    };
}


template <typename vector2_T>
bool is_inside_rect(const vector2_T& point,const vector2_T& top_left_corner,const vector2_T& bottom_right_corner)
{

    if(   point.x > top_left_corner.x && point.x < bottom_right_corner.x 
       && point.y > top_left_corner.y && point.y < bottom_right_corner.y ) return true;

    else return false;
}

template <typename vector2_T>
bool is_inside_rect_by_size(const vector2_T& point,const vector2_T& top_left_corner,const vector2_T& rect_size)
{
    //Same as is_inside_rect, but receives the size of the rect as third argument instead of the absolute points
    const vector2_T bottom_right_corner
    {
        top_left_corner.x + rect_size.x,
        top_left_corner.y + rect_size.y
    };

    if(   point.x > top_left_corner.x && point.x < bottom_right_corner.x 
       && point.y > top_left_corner.y && point.y < bottom_right_corner.y ) return true;

    else return false;
}

template <typename vector2_T,typename length_T>
bool is_inside_rhombus(const vector2_T& point, const vector2_T& center, const length_T& half_size_x,const length_T& half_size_y)
{
    const vector2_T local_point 
    {
        point.x - center.x,
        point.y - center.y
    };

    if( abs(local_point.x)/half_size_x + abs(local_point.y)/half_size_y <= 1) return true;
    else return false;
}