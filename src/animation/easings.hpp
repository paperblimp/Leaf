#pragma once

#include <cmath>
#include <glm/ext/scalar_constants.hpp>
#include <map>
#include <string>


namespace Easings 
{

    template <typename number_T>
    number_T linear(number_T value)
    {
        return value;
    }

    template <typename number_T>
    number_T quad(number_T value)
    {
        return value*value;
    }

    template <typename number_T>
    number_T cubic(number_T value)
    {
        return value*value*value;
    }

    template <typename number_T>
    number_T quart(number_T value)
    {
        return pow(value,4);
    }

    template <typename number_T>
    number_T quint(number_T value)
    {
        return pow(value,5);
    }

    template <typename number_T>
    number_T sine(number_T value)
    {
        return 1.0 - cos((value * glm::pi<number_T>()) / 2.0);
    }

    template <typename number_T>
    number_T circ(number_T value)
    {
        return 1 - sqrt(1 - pow(value, 2));
    }

}

const inline std::map<std::string, double (*)(double)> easings
{
    {"Linear" ,&Easings::linear},
    {"Quad"   ,&Easings::quad  },
    {"Cubic"  ,&Easings::cubic },
    {"Quart"  ,&Easings::quart },
    {"Quint"  ,&Easings::quint },
    {"Sine"   ,&Easings::sine  },
    {"Circ"   ,&Easings::circ  },
};

const inline std::map<double (*)(double), std::string> easings_ref
{
    {&Easings::linear,"Linear"},
    {&Easings::quad  ,"Quad"  },
    {&Easings::cubic ,"Cuboc" },
    {&Easings::quart ,"Quart" },
    {&Easings::quint ,"Quint" },
    {&Easings::sine  ,"Sine"  },
    {&Easings::circ  ,"Circ"  },
};