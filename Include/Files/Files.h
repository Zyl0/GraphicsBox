#pragma once

#include <string>
#include <filesystem>

std::string FileToString(const std::filesystem::path& filename, bool binary = false);