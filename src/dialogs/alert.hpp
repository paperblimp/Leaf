#pragma once


// local
#include "result.hpp"
#include "graphical/graphics.hpp"

// builtin
#include <optional>
#include <tuple>



class Alert
{
    private:

        std::string name;
        std::string message;

    public:

        Alert(const std::string _name, const std::string _message);

        void open();
        EmptyDialogResult run();

    private:

        bool render();
};
