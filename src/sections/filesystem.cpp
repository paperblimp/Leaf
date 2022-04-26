// header
#include "filesystem.hpp"

// builtin
#include <chrono>
#include <string_view>
#include <system_error>

// local
#include "graphical/custom_widgets.hpp"


const std::unordered_set<std::string> Filesystem::supported_image_formats
{
    ".png", ".jpeg", ".bmp", ".ppm", ".pgm"
};

Filesystem::Filesystem(const std::string& path): current_path{path}, file_browser{path, FileBrowser::Type::Folder}
{
    this->input_buffer = new std::array<char, 6666>{};
    strcpy(input_buffer->data(), "");

    this->reload_search_paths();
}

Filesystem::~Filesystem()
{
    delete this->input_buffer;
}


void Filesystem::render()
{
    ImGui::Begin("##Filesystem",NULL,SECTION_FLAGS);
    CustomImGui::Title("Assets");

    if ((std::chrono::system_clock::now() - this->last_search_path_update) > this->search_paths_update_time)
        this->reload_search_paths();

    if (ImGui::InputText("sprite name", this->input_buffer->data(), this->input_buffer->size()))
        this->reload_search_paths();

    if (ImGui::Button("change folder"))
    {
        auto state = FileBrowser::State::get_default();
        state.current_path = this->current_path;
        this->file_browser.open(state);
    }

    if (auto output = this->file_browser.run(); output.has_value())
    {
        this->current_path = output.value();
        this->reload_search_paths();
    }

    uint64_t indent = 6;
    uint64_t window_width = ImGui::GetWindowWidth();
    uint64_t min_images_per_row = (window_width - indent) / (this->max_icon_width + indent);
    
    uint64_t max_image_width;
    if (min_images_per_row != 0)
        max_image_width = this->max_icon_width;
    else
    {
        min_images_per_row = 1;
        max_image_width = ImGui::GetWindowWidth() - indent;
    }


    ImGui::BeginGroup();

    for (size_t idx = 0; idx < this->search_paths.size(); ++idx)
    {
        
        auto& sprite = this->get_sprite(this->search_paths[idx].string());
        auto size = this->get_image_render_size(max_image_width, sprite);

        ImGui::SameLine();
        if (ImGui::GetWindowWidth() - ImGui::GetCursorPos().x < size.x || idx == 0)
            ImGui::NewLine();


        ImGui::BeginGroup();

        ImGui::Image((void*)(uintptr_t)sprite.id.value(), size);
        
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("sprite", sprite.path.data(), sprite.path.size());

            ImGui::Image((void*)(uintptr_t)sprite.id.value(), size);
            ImGui::EndDragDropSource();
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + size.x);
        ImGui::TextWrapped("%s", this->search_paths[idx].filename().string().c_str());


        ImGui::EndGroup();

    }
    ImGui::EndGroup();

    ImGui::End();
}


void Filesystem::reload_search_paths()
{
    this->search_paths.clear();

    if (std::string_view{this->input_buffer->data()} == "")
    {
        for (auto path: std::filesystem::directory_iterator{this->current_path})
            if (path.is_regular_file() && this->supported_image_formats.find(path.path().extension().string()) != this->supported_image_formats.end())
                this->search_paths.push_back(path.path());
    }
    else
    {
        for (auto path: std::filesystem::directory_iterator{this->current_path})
        {
            if (!path.is_regular_file() || !(this->supported_image_formats.find(path.path().extension().string()) != this->supported_image_formats.end()))
                continue;
                
            auto name = path.path().filename().string();
            if (name.find(this->input_buffer->data()) != name.npos)
                this->search_paths.push_back(path.path());
        }
    }

    this->last_search_path_update = std::chrono::system_clock::now();
}

const Sprite& Filesystem::get_sprite(const std::string& filename)
{
    if (!sprite_manager.sprite_exists(filename))
        sprite_manager.load_sprite(filename);

    return sprite_manager.get_sprite(filename);
}



ImVec2 Filesystem::get_image_render_size(uint64_t max, const Sprite& image)
{
    if (image.size.x > image.size.y)
    {
        auto proportion = (float)image.size.y / (float)image.size.x;
        return {(float)max, (float)max * proportion};
    }
    else
    {
        auto proportion = (float)image.size.x / (float)image.size.y;
        return {(float)max * proportion, (float)max};
    }
}