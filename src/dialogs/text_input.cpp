// header
#include "text_input.hpp"
#include <imgui.h>



TextInput::TextInput(const std::string _name, const std::string _label): name{std::move(_name)}, label{std::move(_label)} {}

void TextInput::open(std::string state)
{
    leaf_assert(this->state == nullptr);
    this->state = new std::array<char, 6666>{};

    if (!state.empty())
        std::strcpy(this->state->data(), state.data());

    ImGui::OpenPopup(this->name.c_str());
}

DialogResult<std::string> TextInput::run()
{
    DialogResult<std::string> return_value = DialogResult<std::string>::make_closed();

    if (ImGui::BeginPopupModal(this->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        leaf_assert(this->state != nullptr);
        
        if (auto output = this->render(); output.first == false)
        {
            this->state = nullptr;
            ImGui::CloseCurrentPopup();
            if (output.second.has_value())
            {
                return_value = DialogResult<std::string>{true, true, output.second.value()};
                delete this->state;
            }
        }
        else
        {
            return_value = DialogResult<std::string>::make_runned();
        }

        ImGui::EndPopup();
    }

    return return_value;
}

std::pair<bool, std::optional<std::string>> TextInput::render()
{
    leaf_assert(this->state != nullptr);

    ImGui::InputText(this->label.c_str(), (char*)this->state->data(), this->state->size());
    //Add focus to this input on start
    if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        return {false, {}};

    ImGui::SameLine();

    if (ImGui::Button("Ok") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
        return {false, std::string{this->state->data()}};

    return {true, std::nullopt};
}