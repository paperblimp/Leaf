// header
#include "choise.hpp"




Choise::Choise(const std::string _name, const std::string _text, const std::vector<std::string> _alternatives)
: name{std::move(_name)}, text{std::move(_text)}, alternatives{std::move(_alternatives)}
{

}

void Choise::open()
{
    ImGui::OpenPopup(this->name.c_str());
}

DialogResult<std::string> Choise::run()
{
    DialogResult<std::string> return_value = DialogResult<std::string>::make_closed();

    if (ImGui::BeginPopupModal(this->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (auto output = this->render(); output.first == false)
        {
            ImGui::CloseCurrentPopup();
            if (output.second.has_value())
                return_value = {true, true, output.second.value()};
        }
        else
        {
            return_value = DialogResult<std::string>::make_runned();
        }

        ImGui::EndPopup();
    }

    return return_value;
}

std::pair<bool, std::optional<std::string>> Choise::render()
{
    ImGui::Text("%s", this->text.c_str());

    if (ImGui::Button(this->alternatives[0].c_str()))
        return {false, this->alternatives[0]};

    for (size_t idx = 1; idx < this->alternatives.size(); ++idx)
    {
        ImGui::SameLine();
        if (ImGui::Button(this->alternatives[idx].c_str()))
            return {false, this->alternatives[idx]};
    }

    return {true, std::nullopt};
}
