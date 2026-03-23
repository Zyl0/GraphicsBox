#pragma once

#include <string>
#include <filesystem>

std::string FileToString(const std::filesystem::path& filename, bool binary = false);

void AddSearchPath(const std::filesystem::path& path);

bool GetAbsoluteFilePath(const std::filesystem::path& RelativePath, std::filesystem::path& absolutePath);