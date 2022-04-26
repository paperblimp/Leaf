#pragma once

// builtin
#include <string>

// local
#include "file_browser.hpp"



class FrameExportDialog
{
    private:

        static inline const std::string dialog_name{"Print Export"};

    private:

        FileBrowser file_browser{"select output path", FileBrowser::Type::File, FrameExportDialog::browser_filter};
        std::array<char, 6666> output_path = {""};

    public:

        void open();
        void run();

    private:

        void export_frame(const std::string& path);
        static std::optional<std::string> browser_filter(const std::filesystem::path path);

};
