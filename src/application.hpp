#pragma once


// builtin
#include <imgui.h>
#include <optional>
#include <filesystem>

// extern
#include <nlohmann/json.hpp>

// local
#include "config.hpp"
#include "graphical/graphics.hpp"
#include "screens/main_screen.hpp"
#include "screens/projects_screen.hpp"
#include "graphical/sprite.hpp"
#include "utils/system.hpp"
#include "utils/asserts.hpp"
#include "utils/file_io.hpp"
#include "history.hpp"
#include "graphical/theme.hpp"
#include "key_map.hpp"



class Application
{

    public:

        Application()
        {

            config = this->load_config();
            graphic_context.init();
            history = new History{config.max_history_length};
            
            leaf_assert(graphic_context.initialized = true);
        }


        ~Application()
        {
            sprite_manager.clear();
            graphic_context.destroy();

            auto json = nlohmann::json(config).dump();
            write_file((get_system_config_directory() / ".leaf").string(), (void*)json.data(), json.size());
        }

        void run()
        {
            std::tuple<bool, void*, nlohmann::json>(*init_window)(nlohmann::json) = projects_screen;
            nlohmann::json input = nlohmann::json::object();
            style_manager.apply_main_style();
            style_manager.load_fonts();


            while (true)
            {
                // reset key map
                for (auto& k: key_map)
                    k = false;

                auto result = init_window(input);

                if (std::get<0>(result) == false)
                    break;

                init_window = (decltype(init_window))std::get<1>(result);
                input = std::get<2>(result);
            }
        }


    private:

        nlohmann::json load_config()
        {
            auto path = get_system_config_directory() / ".leaf";
            if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
                return nlohmann::json::parse(read_file(path.string()));
            else
                return nlohmann::json::parse(read_file("default_config.json"));
        }
};