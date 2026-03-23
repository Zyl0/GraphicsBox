#include "Files.h"

#include <fstream>
#include <sstream>
#include <set>

#include "Shared/Logger.h"

static std::set<std::filesystem::path> SearchPaths{};

std::string FileToString(const std::filesystem::path& filename, bool binary)
{
    std::string source;
    bool exists = false;
    {
        std::ifstream in;
        if (binary)
        {
            in.open(filename, std::ios::binary);
        }
        else
        {
            in.open(filename);
        }

        if (in.good() == true)
        {
            if (binary)
            {
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                in.seekg(0, std::ios::beg);

                source.resize(size);
                in.read(source.data(), size);
            }
            else
            {
                std::stringbuf buf;
                in.get(buf, 0);
                source = buf.str();
            }

            exists = true;
        }
    }

    if (!exists)
    {
        EngineLoggerErrorF("Impossible to load file %s. No such file or directory", filename.string().c_str());
    }
    return source;
}

void AddSearchPath(const std::filesystem::path& path)
{
    SearchPaths.insert(path);
}

bool GetAbsoluteFilePath(const std::filesystem::path& RelativePath, std::filesystem::path& absolutePath)
{
    for (const auto & search_path : SearchPaths)
    {
        std::filesystem::path path = search_path / RelativePath;
        if (exists(path))
        {
            absolutePath = path;
            return true;
        }
    }
    return false;
}
