#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"

namespace FrameGraph
{
    struct DrawGLTFFilters
    {
        GLTF::Material::EFlags MaterialFlagsRequire = GLTF::Material::EFlags::None;
        GLTF::Material::EFlags MaterialFlagsExclude = GLTF::Material::EFlags::None;
        
        bool FrustumCulling = true;
    };
    
    enum DrawGLTFSortingProfile
    {
        SP_SortForRenderingShadedElements = 0,
        SP_SortForRenderingDepthBuffer
    };
    
    struct DrawGLTFSettings
    {
        uint8_t TextureUnitOffset = 0;
        const Sampler* MaterialSampler = nullptr;
        
        GLint GLTFMaterialBufferBinding = -1;
        GLint CurrentCameraBufferBinding = -1;
        GLint PreviousCameraBufferBinding = -1;
        
        DrawGLTFFilters Filters;
        
        bool BindMaterial = true;
        
        std::span<uint32_t> ElementListToDraw = {};
    };
    void DrawGLTFScene(const CommandContext& Resources, const Pipeline& Pipeline, FrameGraph::Location Camera, const DrawGLTFSettings& Settings);
    
    // Draw Calls list
    size_t DrawGLTFElementListSize(const CommandContext& Resources, FrameGraph::Location Camera, const DrawGLTFFilters& Filters);
    void DrawGLTFElementListCollect(const CommandContext& Resources, FrameGraph::Location Camera, std::span<uint32_t> ElementList, const DrawGLTFFilters& Filters);
    void DrawGLTFElementListSort(const CommandContext& Resources, FrameGraph::Location Camera, std::span<uint32_t> ElementList, DrawGLTFSortingProfile SortProfile);
}
