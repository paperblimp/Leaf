#pragma once

// local
#include "result.hpp"
#include "graphical/graphics.hpp"

// builtin
#include <optional>
#include <tuple>


class Choise
{
    private:

        std::string name;
        std::string text;
        std::vector<std::string> alternatives;

    public:

        Choise(const std::string _name, const std::string _text, const std::vector<std::string> _alternatives);

        void open();
        DialogResult<std::string> run();

    private:

        std::pair<bool, std::optional<std::string>> render();

};