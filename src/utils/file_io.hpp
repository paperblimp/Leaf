#pragma once


// builtin
#include <string>



std::string read_file(const std::string& path);
void write_file(const std::string& path, void* data, size_t size);
