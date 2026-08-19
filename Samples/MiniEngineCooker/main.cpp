#include "App.h"
#include "Core/Spec.h"

// Modules
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"

#include "Files/Files.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Rendering/Rendering.h"

using namespace Math;

class AppModule : public Engine::IModule
{
public:
    AppModule() {}

    ~AppModule() override = default;
    
    void RegisterDependencies(Engine::Spec& spec) override
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
    }

    void Initialize() override
    {
        using path = std::filesystem::path;
        
        std::array ScenesToCook = {
            path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb",
            path("RTXDI-Assets") / "bistro" / "bistro.gltf",
        };
        
        EngineLoggerLogF("Cooking %llu scene(s)", ScenesToCook.size());
        
        for (size_t i = 0; i < ScenesToCook.size(); i++)
        {
            const path& scene = ScenesToCook[i];
            EngineLoggerLogF("Cooking Scene %llu/%llu", i + 1, ScenesToCook.size());
            
            path AbsolutePath;
            AssertOrErrorCallF(GetAbsoluteFilePath(scene, AbsolutePath), continue;, "Could not find scene %s", scene.generic_string().c_str())
            
            CookScene(AbsolutePath, scene);
        }
    }

    void Tick(double deltaTime) override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        Window->RequestClose();
    }

    void Shutdown() override
    {
        
    }
    
private:
    static void CookScene(const std::filesystem::path& SceneAbsolutePath, const std::filesystem::path& RelativePath)
    {
        GLTF::GPUScene scene;
        
        // Load Scene
        EngineLoggerLogF("Loading Scene %s", SceneAbsolutePath.generic_string().c_str());
        std::filesystem::path extension = SceneAbsolutePath.extension();
        if (extension.compare(".glb") == 0 || extension.compare(".gltf") == 0)
        {
            AssertOrError(GLTF::LoadGPUScene(SceneAbsolutePath, scene), "Failed to load scene")
        }
        else if (extension.compare(".gbs") == 0)
        {
            AssertOrError(GBS::LoadGPUScene(SceneAbsolutePath, scene), "Failed to load scene")
        }
        else
        {
            EngineLoggerErrorF("Unsupported file extension (%s) for gltf file", extension.string().c_str());
            return;
        }
        
        // Perform Cooker actions
        EngineLoggerLog("Performing cooking operations");
        EnableMaterialAsBuffers(scene);
        EnableMaterialAsUnifiedBuffer(scene);
        EnableTexturesAsBindlessArrays(scene);
        // TODO when spirv, bake material shaders maybe?
        // TODO bake lighting data
        // TODO bake BVH data
        
        // Store as .gbs files
        std::filesystem::path ExportPath = std::filesystem::path(TEMP_BAKED_SCENES) / RelativePath;
        EngineLoggerLogF("Exporting Cooked Scene to %s", ExportPath.generic_string().c_str());
        ExportPath.replace_extension(".gbs");
        GBS::SaveGPUScene(ExportPath, scene, GBS::ExportSettings{
            .flags = GBS::ExportSettings::None
        });
    }
};


int main(int argc, char* argv[])
{
    // Search paths
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);
    AddSearchPath(RESOURCES_SAMPLE_SCENES);
    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<AppModule>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
