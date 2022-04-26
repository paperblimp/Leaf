// header
#include "system.hpp"

// local
#include "utils/log.hpp"



std::filesystem::path get_system_config_directory()
{
    #if defined(_WIN32)
    auto var_name = "APPDATA";
    #elif defined(__linux__)
    auto var_name = "HOME";
    #endif

    auto output = getenv(var_name);

    if (output == nullptr)
        panic("path environment variable not found");
    
    return output;
}
