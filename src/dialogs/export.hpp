
#pragma once


// local
#include "dialogs/file_browser.hpp"
#include "graphical/graphics.hpp"
#include "utils/asserts.hpp"
#include "node_tree.hpp"

// builtin
#include <array>
#include <cstdint>
#include <atomic>



class ExportProcess
{

    private: 
    
        std::shared_ptr<std::atomic<uint8_t>> progress_counter;
        std::shared_ptr<std::atomic_bool> _stop;

    public:

        ExportProcess(std::string path, uint64_t fps, double animation_length);

        std::optional<uint8_t> get_export_progress();
        void stop();

};


class ExportDialog
{

    private:

        std::optional<ExportProcess> export_process = std::nullopt;
        std::optional<FileBrowser> file_browser = std::nullopt;
        double animation_length;
        std::array<char, 6666> path;
        int32_t fps;

    public:

        ExportDialog(double _animation_length);
        bool run();

};


void export_animation(std::string path, uint64_t fps, double length, std::shared_ptr<std::atomic_uint8_t> progress_counter, std::shared_ptr<std::atomic_bool> stop);