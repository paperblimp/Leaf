// header
#include "file_io.hpp"

// builtin
#include <filesystem>
#include <fstream>

// extern
#include <fmt/core.h>

// local
#include "utils/log.hpp"



std::string read_file(const std::string& path)
{

    if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
        panic(fmt::format("file '{}' does not exist or is not a regular file", path));

    std::ifstream file (path);

    if (file.bad())
        panic(fmt::format("could not open file '{}'", path));
    if(!file.is_open())
        panic(fmt::format("file '{}' was not opened", path));

    auto file_size = std::filesystem::file_size(path);
    
    std::string output;
    output.resize(file_size);

    file.read(output.data(), file_size);
    return output;
}

void write_file(const std::string& path, void* data, size_t size)
{
    std::ofstream file {path};

    if (file.bad() || !file.is_open())
        panic(fmt::format("could not open file '{}'", path));

    file.write((char*)data, size);
}
