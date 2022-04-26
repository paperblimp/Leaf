#pragma once


// builtin
#include <string>

// local
#include "source_location.hpp"



void leaf_assert(bool b, std::string message = "", SourceLocation location = SourceLocation::get_source_location());
void leaf_runtime_assert(bool b, std::string message = "", SourceLocation location = SourceLocation::get_source_location());
