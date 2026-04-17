#include "GLTF/SceneLoader.h"

#ifdef USE_TINY_GLTF_3
#include <tiny_gltf_v3.h>

namespace GLTF
{
    bool LoadCPUScene(const std::filesystem::path& path, CPUScene& scene)
    {
        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        
        tinygltf3::Model model;
        tinygltf3::ErrorStack errors;
        tg3_parse_options options;
        tg3_error_code rc = tinygltf3::parse_file(model, errors, path.generic_string().c_str());
    }

    bool LoadGPUScene(const std::filesystem::path& path, GPUScene& scene)
    {
    }
}
#endif // USE_TINY_GLTF_3