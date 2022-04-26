
// local
#include "application.hpp"



int main()
{
    notice(fmt::format("system config dir: {}", get_system_config_directory().string()));
    Application{}.run();
}
