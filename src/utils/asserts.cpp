// header
#include "asserts.hpp"

// local
#include "utils/log.hpp"



void leaf_assert(bool b, std::string message, SourceLocation location)
{
    #ifndef NDEBUG

        if (b)
            return;
            
        if (message.empty())
            message = "assert failed";

        panic(message, location);
    
    #endif
}

void leaf_runtime_assert(bool b, std::string message, SourceLocation location)
{
    if (b)
        return;
 
    if (message.empty())
        message = "assert failed";

    panic(message, location);
}
