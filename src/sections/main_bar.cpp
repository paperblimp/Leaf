#include "main_bar.hpp"

//local
#include "node_tree.hpp"
#include "utils/serialization.hpp"

#include <imgui.h>


bool MainBar::render()
{
    //Return true if the user chooses to quit

    bool shall_quit = false;
    bool shall_open_quit = false;
    bool shall_open_preferences = false;

    if(ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Preferences"))
            {
                shall_open_preferences = true;
            }
            if(ImGui::MenuItem("Save"))
            {
                serialize_project(config.current_project.header.path);
            }
            if(ImGui::MenuItem("Quit"))
            {
                shall_open_quit = true;
            }
            
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }


    if(shall_open_quit)  save_on_quit.open();
    if(auto quit_input = save_on_quit.run();quit_input.has_value())
    {
        if(quit_input.value() == "Save & Exit") 
        {
            serialize_project(config.current_project.header.path);
            unload_project();
            shall_quit = true;
        }
        else if(quit_input.value() == "Don't Save")
        {
            unload_project();
            shall_quit = true;
        }
    }

    if(shall_open_preferences)   preferences_dialog.open();
    if(preferences_dialog.run().ended()) serialize_project(config.current_project.header.path);

    return shall_quit;
}