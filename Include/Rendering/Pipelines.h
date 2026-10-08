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

    Pipeline(Pipeline&& other) noexcept : m_Pipeline(other.m_Pipeline), m_Shaders(other.m_Shaders), m_Type(other.m_Type)
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

Pipeline PipelineFromString(std::string_view label, Pipeline::Shaders shaders, std::string_view source, Shader::DefinesView Defines = {});

INLINE Pipeline PipelineFromFile(std::string_view label,  Pipeline::Shaders shaders, const std::filesystem::path& filename, Shader::DefinesView Defines = {})
{
    std::string Source = ShaderFileToString(filename);
    
    return PipelineFromString(label, shaders, Source, Defines);
}

bool PipelineUpdateFromString(Pipeline& pipeline, std::string_view source, Shader::DefinesView Defines = {});

INLINE bool PipelineUpdateFromFile(Pipeline& pipeline, const std::filesystem::path& filename, Shader::DefinesView Defines = {})
{
    std::string Source = ShaderFileToString(filename);
    
    return PipelineUpdateFromString(pipeline, Source, Defines);
}

class PipelineMatrix
{
public:
    using Name = uint64_t;
    
    struct MatrixDesc
    {
        using Literal = std::vector<std::pair<std::string_view, std::vector<std::string_view>>>;

        MatrixDesc() = default;
        MatrixDesc(const Literal& params): m_Parameters(params) {}
        MatrixDesc(Literal&& params): m_Parameters(std::move(params)) {}
        
        Name AddParameterName(std::string_view defineName);
        Name AddParameterValue(Name parameterName, std::string_view defineValue);

        Name FindParameterName(std::string_view defineName) const;
        Name FindParameterValue(Name parameterName, std::string_view defineValue) const;

        std::string_view GetParameterName(Name parameterName) const;
        std::string_view GetParameterValue(Name parameterName, uint64_t defineValue) const;

        MatrixDesc& operator = (const Literal& params)
        {
            m_Parameters = params;
            return *this;
        }

        MatrixDesc& operator = (Literal&& params)
        {
            m_Parameters = std::move(params);
            return *this;
        }

        Literal m_Parameters;
    };

    struct VariantDesc
    {
        VariantDesc() = default;
        VariantDesc(const MatrixDesc& Ref) { Set(Ref); }
        
        void Set(const MatrixDesc& Ref);
        void SetParameter(Name param, Name Value);
        void UnsetParameter(Name param);
        void Reset();
        void Clear();

        std::vector<Name> m_Parameters;
    };

    PipelineMatrix() = default;

    PipelineMatrix(const PipelineMatrix& other) = delete;

    PipelineMatrix(PipelineMatrix&& other) noexcept
        : m_ParameterMatrix(std::move(other.m_ParameterMatrix)),
          m_Pipelines(std::move(other.m_Pipelines)),
          m_PipelineTree(std::move(other.m_PipelineTree)),
          m_Head(other.m_Head)
    {        
    }

    PipelineMatrix& operator=(PipelineMatrix other)
    {
        using std::swap;
        swap(m_ParameterMatrix, other.m_ParameterMatrix);
        swap(m_Pipelines, other.m_Pipelines);
        swap(m_PipelineTree, other.m_PipelineTree);
        swap(m_Head, other.m_Head);
        return *this;
    }

    friend PipelineMatrix PipelineMatrixFromString(std::string_view label, Pipeline::Shaders shaders, std::string_view source, const PipelineMatrix::MatrixDesc& paramMatrix, Shader::DefinesView Defines);
    friend bool PipelineMatrixUpdateFromString(PipelineMatrix& pipeline, std::string_view source, Shader::DefinesView Defines);
    
    const Pipeline& GetVariant(const VariantDesc& variant);
    
private:
    MatrixDesc m_ParameterMatrix;
    std::vector<Pipeline> m_Pipelines;
    std::vector<uint32_t> m_PipelineTree;
    size_t m_Head = 0;
};

PipelineMatrix PipelineMatrixFromString(std::string_view label, Pipeline::Shaders shaders, std::string_view source, const PipelineMatrix::MatrixDesc& paramMatrix = {}, Shader::DefinesView Defines = {});

INLINE PipelineMatrix PipelineMatrixFromFile(std::string_view label,  Pipeline::Shaders shaders, const std::filesystem::path& filename, const PipelineMatrix::MatrixDesc& paramMatrix = {}, Shader::DefinesView Defines = {})
{
    std::string Source = ShaderFileToString(filename);
    
    return PipelineMatrixFromString(label, shaders, Source, paramMatrix, Defines);
}

bool PipelineMatrixUpdateFromString(PipelineMatrix& pipeline, std::string_view source, Shader::DefinesView Defines = {});

INLINE bool PipelineMatrixUpdateFromFile(PipelineMatrix& pipeline, const std::filesystem::path& filename, Shader::DefinesView Defines = {})
{
    std::string Source = ShaderFileToString(filename);
    
    return PipelineMatrixUpdateFromString(pipeline, Source, Defines);
}
