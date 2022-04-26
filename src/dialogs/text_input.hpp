#pragma once


// local
#include "graphical/graphics.hpp"
#include "../utils/asserts.hpp"
#include "result.hpp"

// builtin
#include <optional>
#include <tuple>



class TextInput
{
    private:

        std::array<char, 6666>* state = nullptr;
        std::string name;
        std::string label;

    public:

        TextInput(const std::string _name, const std::string _label);

        void open(std::string state = "");
        DialogResult<std::string> run();

    private:

        std::pair<bool, std::optional<std::string>> render();
};
