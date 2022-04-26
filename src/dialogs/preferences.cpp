#include "preferences.hpp"
#include "config.hpp"
#include "utils/serialization.hpp"
#include <fmt/core.h>
#include <imgui.h>

void PreferencesDialog::open()
{
    temp_preferences = ApplicationConfig::Project::Preferences{config.current_project.preferences};
    ImGui::OpenPopup("Project Preferences");
}

EmptyDialogResult PreferencesDialog::run()
{
    auto return_value = EmptyDialogResult::make_closed(); //Return true when user clicks on save


    if(ImGui::BeginPopupModal("Project Preferences",nullptr,ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto& prefs = temp_preferences.value();
        ImGui::Text("Resolution:");ImGui::SameLine();
        ImGui::InputInt2("##VideoResolution", prefs.video_resolution);
        if(prefs.video_resolution[0] < 1)prefs.video_resolution[0] = 1;
        if(prefs.video_resolution[1] < 1)prefs.video_resolution[1] = 1;


        if(ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if(ImGui::SameLine();ImGui::Button("Save"))
        {
            config.current_project.preferences = prefs;
            return_value = EmptyDialogResult::make_ended();

            ImGui::CloseCurrentPopup();
        }
        else
        {
            return_value = EmptyDialogResult::make_runned();
        }

        ImGui::EndPopup();
    }

    return return_value;

}