#pragma once

// local
#include "result.hpp"
#include "config.hpp"
#include "utils/serialization.hpp"

// builtin
#include <array>
#include <optional>

// extern
#include <imgui.h>
#include <imgui_internal.h>



class PreferencesDialog
{

    std::optional<ApplicationConfig::Project::Preferences> temp_preferences{};


    public:
        
        void open();
        EmptyDialogResult run();
};