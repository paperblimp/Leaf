// header
#include "file_browser.hpp"

// builtin
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <imgui.h>



FileBrowser::FileBrowser(const std::string _name, Type _select_type, decltype(filter) _filter)
: state{std::nullopt}, name{std::move(_name)}, select_type(_select_type), filter{_filter}
{

}

FileBrowser::State FileBrowser::State::get_default()
{
    return {
        "/",
        std::nullopt,
        std::nullopt,
        nullptr,
        new std::array<char, 6666>{""},
        {"new folder", "new folder name"},
        {"new file", "new file name"},
        {"alert##folder", "folder already exists"},
        {"file overwrite", "this file already exists, want to overwrite?", {"no", "yes"}},
        std::nullopt,
        std::nullopt,
        std::nullopt
    };
}

FileBrowser::State FileBrowser::State::default_with_current_path()
{
    auto state = FileBrowser::State::get_default();
    state.current_path = std::filesystem::current_path();
    
    return state;
}

void FileBrowser::open(FileBrowser::State state)
{
    leaf_assert(this->state.has_value() == false);
    this->state = state;
    leaf_assert(this->state->current_path.is_absolute());
    ImGui::OpenPopup(this->name.c_str());
}

DialogResult<std::string> FileBrowser::run()
{
    
    DialogResult<std::string> return_value = DialogResult<std::string>::make_closed();

    if (ImGui::BeginPopupModal(this->name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return_value = DialogResult<std::string>::make_runned();

        leaf_assert(this->state.has_value());
        if (auto output = this->render(); output.first == false)
        {
            this->state = std::nullopt;
            ImGui::CloseCurrentPopup();
            if (output.second.has_value())
                return_value = {true, true, output.second.value()};
        }

        ImGui::EndPopup();
    }

    return return_value; 
}  

std::pair<bool, std::optional<std::string>> FileBrowser::render()
{
    std::vector<std::filesystem::path> directories;
    std::vector<std::filesystem::path> files;

    // load current path content
    for (auto node: std::filesystem::directory_iterator{this->state->current_path})
    {
        if (node.is_directory())
            directories.push_back(node);
        else
            files.push_back(node);
    }

    // add parent directory
    directories.insert(directories.begin(), this->state->current_path.parent_path());


    const auto is_selected_dir = [this](uint32_t idx) -> bool
    {
        return (this->state->selected_directory.has_value()) ? idx == this->state->selected_directory.value() : false;
    };

    const auto is_selected_file = [this](uint32_t idx) -> bool
    {
        return (this->state->selected_file.has_value()) ? idx == this->state->selected_file.value() : false;
    };


    // current path indicator
    ImGui::TextWrapped("%s", this->state->current_path.string().c_str());
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::BeginGroup();
    ImGui::Text("folders");
    ImGui::BeginChild("directories", {200, 300}, true, ImGuiWindowFlags_HorizontalScrollbar);
    

    // return to parend dir
    if (this->state->current_path.has_parent_path() && ImGui::Selectable("..", is_selected_dir(0), ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            this->state->current_path = this->state->current_path.parent_path();
            this->state->selected_directory = {};
            this->state->selected_file = {};
            ImGui::SetScrollHereY(0.0f);
        }
        else
            this->state->selected_directory = 0;
    }

    // current path folders
    for (size_t idx = 1; idx < directories.size(); ++idx)
    {
        if (ImGui::Selectable(directories[idx].stem().string().c_str(), is_selected_dir(idx), ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                this->state->current_path = directories[idx];
                this->state->selected_directory = {};
                this->state->selected_file = {};
                ImGui::SetScrollHereY(0.0f);
            }
            else
            {
                this->state->selected_directory = idx;

                if (this->select_type == Type::Folder)
                    strcpy(this->state->file_name_indicator->data(), directories[idx].filename().string().data());
            }
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();


    ImGui::SameLine();


    ImGui::BeginGroup();
    ImGui::Text("files");
    ImGui::BeginChild("files", {400, 300}, true, ImGuiWindowFlags_HorizontalScrollbar);


    // current path files
    for (size_t idx = 0; idx < files.size(); ++idx)
    {
        if (ImGui::Selectable(files[idx].filename().string().c_str(), is_selected_file(idx), 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
        {
            this->state->selected_file = idx;

            if (this->select_type == Type::File)
                strcpy(this->state->file_name_indicator->data(), files[idx].filename().string().data());
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();


    // selected file indicator
    ImGui::PushItemWidth(608);

    ImGui::PushID("input text foo");
    ImGui::InputText("", this->state->file_name_indicator->data(), this->state->file_name_indicator->size());
    ImGui::PopID();

    // 'new file' button
    if (ImGui::Button("new file"))
        this->state->new_file_input.open();

    if (auto output = this->state->new_file_input.run(); output.has_value())
    {
        auto path = this->state->current_path / output.value();

        if (std::filesystem::exists(path))
        {
            this->state->file_overwrite_choise.open();
            this->state->overwrite_path = path.string();
        }
        else
            std::ofstream{path};
    }

    if (auto output = this->state->file_overwrite_choise.run(); output.has_value() && output.value() == "ok")
    {
        std::ofstream(this->state->overwrite_path.value());
        this->state->overwrite_path = std::nullopt;
    }


    ImGui::SameLine();

    // 'new folder' button
    if (ImGui::Button("new folder"))
        this->state->new_folder_input.open();

    if (auto output = this->state->new_folder_input.run(); output.has_value())
    {
        auto path = this->state->current_path / output.value();

        if (std::filesystem::exists(path))
            this->state->folder_already_exists_alert.open();
        else
            std::filesystem::create_directory(path);
    }

    this->state->folder_already_exists_alert.run();


    ImGui::SameLine();

    if (ImGui::Button("cancel"))
        return {false, std::nullopt};
    
    ImGui::SameLine();

    if (ImGui::Button("ok"))
    {
        if (strlen(this->state->file_name_indicator->data()) > 0)
            this->state->selected_path = this->state->current_path / this->state->file_name_indicator->data();
        else
            this->state->selected_path = this->state->current_path;
    }

    if (this->state->selected_path.has_value())
    {
        if (auto output = this->filter(this->state->selected_path.value()); output.has_value())
        {
            this->state->invalid_file_confirmation = Alert{"alert##invalid_file", output.value()};
            this->state->invalid_file_confirmation.value().open();

            this->state->selected_path = std::nullopt;
        }
        else
            return {false, this->state->selected_path.value().string()};
    }

    if (this->state->invalid_file_confirmation.has_value())
        if (this->state->invalid_file_confirmation->run().ended())
            this->state->invalid_file_confirmation = std::nullopt;


    return {true, {}};
}
