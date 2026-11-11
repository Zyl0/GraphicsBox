#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"

namespace FrameGraph
{
    struct DrawGLTFSettings
    {
        uint8_t TextureUnitOffset = 0;
        const Sampler* MaterialSampler = nullptr;
        
        GLint GLTFMaterialBufferBinding = -1;
        GLint CurrentCameraBufferBinding = -1;
        GLint PreviousCameraBufferBinding = -1;
        
        GLTF::Material::EFlags MaterialFlagsRequireFilter = GLTF::Material::EFlags::None;
        GLTF::Material::EFlags MaterialFlagsExcludeFilter = GLTF::Material::EFlags::None;
        
        bool BindMaterial = true;
        bool FrustumCulling = true;
    };
    void DrawGLTFScene(const CommandContext& Resources, const Pipeline& Pipeline, FrameGraph::Location Camera, const DrawGLTFSettings& Settings);
}
