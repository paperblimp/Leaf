#pragma once

// builtin
#include <fstream>
#include <imgui.h>
#include <array>

// extern

#include <imgui.h>
#include <fmt/core.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

//local
#include "node_tree.hpp"
#include "dialogs/preferences.hpp"
#include "dialogs/choise.hpp"



class MainBar
{
    Choise save_on_quit{"Exit to Project Screen","Would you like to save before exiting?\n",
        std::vector<std::string> {
            "Save & Exit",
            "Cancel",
            "Don't Save"}
        };
    PreferencesDialog preferences_dialog{};

    public:
        bool render();
};