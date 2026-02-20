#pragma once

#include <span>

#include "Shaders.h"

class Pipeline
{
public:
    using ShaderPair = std::pair<Shader::Type, const Shader&>;

    enum Shaders : uint8_t
    {
        None = 0,
        VERTEX_SHADER =                     1,
        FRAGMENT_SHADER =                   1 << 1,
        GEOMETRY_SHADER =                   1 << 2,
        TESSELATION_CONTROL_SHADER =        1 << 3,
        TESSELATION_EVALUATION_SHADER =     1 << 4,
        
        COMPUTE_SHADER =                    1 << 5            
    };

    enum PipelineType : uint8_t
    {
        Raster,
        Compute,

        _Count
    };

    Pipeline(std::span<const ShaderPair> Shaders, std::string_view Label);
    ~Pipeline();

    Pipeline(Pipeline&& other) : m_Pipeline(other.m_Pipeline), m_Shaders(other.m_Shaders), m_Type(other.m_Type)
    { 
        other.m_Pipeline = 0;
        other.m_Shaders = None;
        other.m_Type = _Count;
    }

    Pipeline& operator=(Pipeline other)
    {
        using std::swap;
        swap(m_Pipeline, other.m_Pipeline);
        swap(m_Shaders, other.m_Shaders);
        swap(m_Type, other.m_Type);
        return *this;
    }

    void Data(std::span<const ShaderPair> Shaders);

    bool IsComplete() const;

    INLINE Shaders MemberShaders() const {return m_Shaders;}
    INLINE PipelineType Type() const {return m_Type;}
    INLINE GLuint Handle() const {return m_Pipeline;}
private:
    GLuint m_Pipeline;
    Shaders m_Shaders;
    PipelineType m_Type;
};

INLINE Pipeline::Shaders operator|(Pipeline::Shaders a, Pipeline::Shaders b)
{
    return static_cast<Pipeline::Shaders>(static_cast<int>(a) | static_cast<int>(b));
}
INLINE Pipeline::Shaders operator&(Pipeline::Shaders a, Pipeline::Shaders b)
{
    return static_cast<Pipeline::Shaders>(static_cast<int>(a) & static_cast<int>(b));
}
INLINE Pipeline::Shaders operator^(Pipeline::Shaders a, Pipeline::Shaders b)
{
    return static_cast<Pipeline::Shaders>(static_cast<int>(a) ^ static_cast<int>(b));
}

void Bind(const Pipeline& shader);
void UnBind(const Pipeline& shader);
