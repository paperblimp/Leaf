// header
#include "utils/log.hpp"



void Log::print_log(const std::string& message, SourceLocation location, const std::string& log_type, std::ostream&(*color)(std::ostream&))
{
    std::cout << '[' << color << log_type << termcolor::reset << ']';
    std::cout << '(' << location.to_string() << ')';
    std::cout << ": " << message;
    std::cout << std::endl;
}

[[noreturn]]
void panic(const std::string& message, SourceLocation location)
{
    Log::print_log(message, location, "PANIC", termcolor::red);
    std::terminate();
}

void warn(const std::string& message, SourceLocation location)
{
    Log::print_log(message, location, "WARNING", termcolor::yellow);
}

void notice(const std::string& message, SourceLocation location)
{
    Log::print_log(message, location, "NOTICE", termcolor::white);
}
