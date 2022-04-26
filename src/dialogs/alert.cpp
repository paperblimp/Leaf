// header
#include "alert.hpp"
#include <imgui.h>


Alert::Alert(const std::string _name, const std::string _message): name{std::move(_name)}, message{std::move(_message)}
{

};


void Alert::open()
{
    ImGui::OpenPopup(this->name.c_str());
}

EmptyDialogResult Alert::run()
{
    auto return_value = EmptyDialogResult::make_closed();

    if (ImGui::BeginPopupModal(this->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (this->render() == false)
            return_value = EmptyDialogResult::make_ended();
        else
            return_value = EmptyDialogResult::make_runned();

        ImGui::EndPopup();
    }

    return return_value;
}

bool Alert::render()
{
    ImGui::Text("%s", this->message.c_str());
    
    if (ImGui::Button("Ok") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
    {
        ImGui::CloseCurrentPopup();
        return false;
    }

    return true;
}
