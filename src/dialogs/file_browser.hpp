#pragma once

// local
#include "graphical/graphics.hpp"
#include "text_input.hpp"
#include "alert.hpp"
#include "choise.hpp"
#include "../utils/asserts.hpp"

// builtin
#include <optional>
#include <tuple>
#include <filesystem>
#include <functional>



class FileBrowser
{
    public:
    
        struct State
        {
            std::filesystem::path current_path;
            std::optional<uint32_t> selected_directory;
            std::optional<uint32_t> selected_file;
            std::array<char, 6666>* new_node_name = nullptr;
            std::array<char, 6666>* file_name_indicator = nullptr;
            
            TextInput new_folder_input;
            TextInput new_file_input;
            Alert folder_already_exists_alert;
            
            Choise file_overwrite_choise;
            std::optional<std::string> overwrite_path;
            std::optional<std::filesystem::path> selected_path;
            std::optional<Alert> invalid_file_confirmation;

            static State get_default();
            static State default_with_current_path();
        };

        enum class Type
        {
            File,
            Folder
        };

        static inline const std::function<std::optional<std::string>(std::filesystem::path)> default_filter = [](const std::filesystem::path) -> std::optional<std::string>
        {
            return std::nullopt;
        };
    
    private:

        std::optional<State> state;
        std::string name;
        Type select_type;
        std::function<std::optional<std::string>(std::filesystem::path)> filter;

    public:

        FileBrowser(std::string _name, Type _select_type = Type::File, decltype(filter) _filter = default_filter);

        void open(FileBrowser::State state = State::get_default());
        DialogResult<std::string> run();

    private:

        std::pair<bool, std::optional<std::string>> render();

};