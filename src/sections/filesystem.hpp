#pragma once


// local
#include "graphical/graphics.hpp"
#include "graphical/sprite.hpp"
#include "dialogs/file_browser.hpp"
#include "section.hpp"

// builtin
#include <filesystem>
#include <unordered_set>
#include <chrono>



// TODO: tela de carregamento dos sprites
class Filesystem : public Section
{
    private:

        static const std::unordered_set<std::string> supported_image_formats;
        inline static const std::chrono::system_clock::duration search_paths_update_time = std::chrono::seconds(2);

        std::filesystem::path current_path;
        std::array<char, 6666>* input_buffer = nullptr;
        std::chrono::system_clock::time_point last_search_path_update = std::chrono::system_clock::now();
        const uint64_t max_icon_width = 96;
        std::vector<std::filesystem::path> search_paths;
        FileBrowser file_browser;

    public:

        Filesystem(const std::string& path);
        ~Filesystem();
        
        void render();

    private:

        void reload_search_paths();
        const Sprite& get_sprite(const std::string& filename);
        ImVec2 get_image_render_size(uint64_t max, const Sprite& image);
};