#include "Pipelines.h"

#include <array>
#include <stack>
#include <vector>

#include "Memory/Functions.h"
#include "Shared/Annotations.h"

static
const char* GetShaderTypeDefineName(const GLenum type)
{
    switch (type)
    {
    case GL_VERTEX_SHADER: return "VERTEX_SHADER";
    case GL_FRAGMENT_SHADER: return "FRAGMENT_SHADER";
    case GL_GEOMETRY_SHADER: return "GEOMETRY_SHADER";
#ifdef GL_VERSION_4_0
    case GL_TESS_CONTROL_SHADER: return "TESSELATION_CONTROL";
    case GL_TESS_EVALUATION_SHADER: return "EVALUATION_CONTROL";
#endif
#ifdef GL_VERSION_4_3
    case GL_COMPUTE_SHADER: return "COMPUTE_SHADER";
#endif

    case Shader::__Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
    }
}

static
const char* GetShaderTypeDefineName(Shader::Type type)
{
    switch (type)
    {
    case Shader::VERTEX_SHADER:
        return "VERTEX_SHADER";
    case Shader::FRAGMENT_SHADER:
        return "FRAGMENT_SHADER";
    case Shader::GEOMETRY_SHADER:
        return "GEOMETRY_SHADER";

#ifdef GL_VERSION_4_0
    case Shader::TESSELATION_CONTROL_SHADER:
        return "TESSELATION_CONTROL";
    case Shader::TESSELATION_EVALUATION_SHADER:
        return "EVALUATION_CONTROL";
#endif

#ifdef GL_VERSION_4_3
    case Shader::COMPUTE_SHADER:
        return "COMPUTE_SHADER";
#endif

    case Shader::__Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
    }
}

Pipeline::Pipeline(std::span<const ShaderPair> Shaders, std::string_view Label):
    m_Shaders(None), m_Type(_Count)
{
    GLCall(m_Pipeline = glCreateProgram())

    if(!Label.empty())
    {
        glObjectLabel(GL_PROGRAM, m_Pipeline, static_cast<GLsizei>(Label.size()), Label.data());
    }

    Data(Shaders);
}

Pipeline::~Pipeline()
{
    if (m_Pipeline == 0) return;

    GLCall(glDeleteProgram(m_Pipeline))
}

void Pipeline::Data(std::span<const ShaderPair> Shaders)
{
    // Cleanup previously used shaders
    if (m_Type != _Count)
    {
        int shaders_max= 0;
        glGetProgramiv(m_Pipeline, GL_ATTACHED_SHADERS, &shaders_max);

        if(shaders_max > 0)
        {
            std::vector<GLuint> shaders(shaders_max, 0);
            glGetAttachedShaders(m_Pipeline, shaders_max, NULL, &shaders.front());
            for(int i= 0; i < shaders_max; i++)
            {
                glDetachShader(m_Pipeline, shaders[i]);
            }
        }

        m_Type = _Count;
        m_Shaders = None;
    }

    // Attach new shaders
    for (const auto & shader : Shaders)
    {
#ifdef CONFIG_DEBUG
        AssertOrErrorCall(glIsShader(shader.second.Handle()), continue;, "Given shader handle does not refer to a valid shader")
#endif // CONFIG_DEBUG
        
        if (m_Type == _Count)
        {
            if (shader.first < Shader::__RasterEnd)
            {
                m_Type = Raster;
            }
            else if (shader.first < Shader::__ComputeEnd)
            {
                m_Type = Compute;
            }
            else
            {
                ENUM_OUT_OF_RANGE("Unsupported shader type")
            }
        }
        else
        {
            if (m_Type == Raster && shader.first < Shader::__RasterEnd)
            {}
            else if (m_Type == Compute && shader.first < Shader::__ComputeEnd)
            {}
            else
            {
                ENUM_OUT_OF_RANGE("Shader type miss match with pipeline type, described by previous shaders subscribed to this pipeline")
            }
        }

        switch (shader.first)
        {
        case Shader::VERTEX_SHADER:                 m_Shaders = m_Shaders | Pipeline::VERTEX_SHADER; break;
        case Shader::FRAGMENT_SHADER:               m_Shaders = m_Shaders | Pipeline::FRAGMENT_SHADER; break;
        case Shader::GEOMETRY_SHADER:               m_Shaders = m_Shaders | Pipeline::GEOMETRY_SHADER; break;
        case Shader::TESSELATION_CONTROL_SHADER:    m_Shaders = m_Shaders | Pipeline::TESSELATION_CONTROL_SHADER; break;
        case Shader::TESSELATION_EVALUATION_SHADER: m_Shaders = m_Shaders | Pipeline::TESSELATION_EVALUATION_SHADER; break;
        case Shader::COMPUTE_SHADER:                m_Shaders = m_Shaders | Pipeline::COMPUTE_SHADER; break;
            
        case Shader::__Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
        }

        GLCall(glAttachShader(m_Pipeline, shader.second.Handle()))
    }
#ifdef CONFIG_DEBUG
    AssertOrErrorCall(glIsProgram(m_Pipeline), return;, "Current program object is not a program")
#endif // CONFIG_DEBUG
    GLCall(glLinkProgram(m_Pipeline))
    GLCall(glValidateProgram(m_Pipeline))

    // Verify
    GLint status;
    GLCall(glGetProgramiv(m_Pipeline, GL_LINK_STATUS, &status))
    AssertOrErrorCall(status == GL_TRUE,, "Pipeline linkage failed.")

    GLint len = 0;
    glGetProgramiv(m_Pipeline, GL_INFO_LOG_LENGTH, &len);

    if (status == true && len > 1)
    {
        std::string log(len, '\0');
        glGetProgramInfoLog(m_Pipeline, len, nullptr, log.data());
        EngineLoggerWarnF("Pipeline linked with warning(s): %s", log.c_str());
    }

    if(status == GL_TRUE)
        return;
    
    int shaders_max= 0;
    GLCall(glGetProgramiv(m_Pipeline, GL_ATTACHED_SHADERS, &shaders_max))
    AssertOrErrorCall(shaders_max > 0, return, "No shader in Pipeline.")

    GLint value= 0;
    GLCall(glGetProgramiv(m_Pipeline, GL_INFO_LOG_LENGTH, &value))

    if (value != 0)
    {
        std::vector<char>log(value + 1, 0);
        glGetProgramInfoLog(m_Pipeline, static_cast<GLsizei>(log.size()), nullptr, &log.front());
        EngineLoggerErrorF("Failed to link Pipeline because %s\n", log.data());
        printf("%s\n", log.data());

        return;
    }
    
    char log[1024];
    GLsizei length;
    glGetProgramInfoLog(m_Pipeline, 1024, &length, log);

    if (length > 0) {
        printf("Link log:\n%s\n", log);
    }
}

bool Pipeline::IsComplete() const
{
    GLint status= GL_FALSE;
    glGetProgramiv(m_Pipeline, GL_LINK_STATUS, &status);
    if(status == GL_TRUE)
        return true;

#ifdef GL_VERSION_4_3
    char label[1024];
    glGetObjectLabel(GL_PROGRAM, m_Pipeline, sizeof(label), nullptr, label);
        
    if(status == GL_FALSE)
        EngineLoggerWarnF("Program %u \"%s\" is not ready",m_Pipeline, label);
#else
    EngineLoggerWarnF("Program %u is not ready", m_Pipeline);
#endif
    
    return false;
}

void Bind(const Pipeline& pipeline)
{
    GLCall(glUseProgram(pipeline.Handle()))
}

void UnBind(const Pipeline& pipeline)
{
    GLCall(glUseProgram(0))
}

Pipeline PipelineFromString(std::string_view label, Pipeline::Shaders shaders, std::string_view source, Shader::DefinesView Defines)
{
    std::vector<Shader> shaderObjects;
    std::vector<Pipeline::ShaderPair> shaderRefs;
    
    shaderObjects.reserve(2);
    shaderRefs.reserve(2);
    
    if (shaders & Pipeline::Shaders::VERTEX_SHADER)
    {
        shaderObjects.emplace_back(Shader::VERTEX_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::VERTEX_SHADER, shaderObjects.back());
    }
    if (shaders & Pipeline::Shaders::FRAGMENT_SHADER)
    {
        shaderObjects.emplace_back(Shader::FRAGMENT_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::FRAGMENT_SHADER, shaderObjects.back());
    }
    if (shaders & Pipeline::Shaders::GEOMETRY_SHADER)
    {
        shaderObjects.emplace_back(Shader::GEOMETRY_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::GEOMETRY_SHADER, shaderObjects.back());
    }
    if (shaders & Pipeline::Shaders::TESSELATION_CONTROL_SHADER)
    {
        shaderObjects.emplace_back(Shader::TESSELATION_CONTROL_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::TESSELATION_CONTROL_SHADER, shaderObjects.back());
    }
    if (shaders & Pipeline::Shaders::TESSELATION_EVALUATION_SHADER)
    {
        shaderObjects.emplace_back(Shader::TESSELATION_EVALUATION_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::TESSELATION_EVALUATION_SHADER, shaderObjects.back());
    }
    if (shaders & Pipeline::Shaders::COMPUTE_SHADER)
    {
        shaderObjects.emplace_back(Shader::COMPUTE_SHADER, label, source, Defines);
        shaderRefs.emplace_back(Shader::COMPUTE_SHADER, shaderObjects.back());
    }
    
    return Pipeline (shaderRefs, label);
}

bool PipelineUpdateFromString(Pipeline& pipeline, std::string_view source, Shader::DefinesView Defines)
{
    std::vector<Shader> shaderObjects;
    std::vector<Pipeline::ShaderPair> shaderRefs;
    
    GLint maxLength = 0;
    glGetIntegerv(GL_MAX_LABEL_LENGTH, &maxLength);
    std::string label(maxLength, '\0');
    GLsizei length = 0;
    
    glGetObjectLabel(
        GL_PROGRAM,
        pipeline.Handle(),
        maxLength,
        &length,
        label.data()
    );
    
    shaderObjects.reserve(2);
    shaderRefs.reserve(2);
    
    if (pipeline.MemberShaders() & Pipeline::Shaders::VERTEX_SHADER)
    {
        shaderObjects.emplace_back(Shader::VERTEX_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::VERTEX_SHADER, shaderObjects.back());
    }
    if (pipeline.MemberShaders() & Pipeline::Shaders::FRAGMENT_SHADER)
    {
        shaderObjects.emplace_back(Shader::FRAGMENT_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::FRAGMENT_SHADER, shaderObjects.back());
    }
    if (pipeline.MemberShaders() & Pipeline::Shaders::GEOMETRY_SHADER)
    {
        shaderObjects.emplace_back(Shader::GEOMETRY_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::GEOMETRY_SHADER, shaderObjects.back());
    }
    if (pipeline.MemberShaders() & Pipeline::Shaders::TESSELATION_CONTROL_SHADER)
    {
        shaderObjects.emplace_back(Shader::TESSELATION_CONTROL_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::TESSELATION_CONTROL_SHADER, shaderObjects.back());
    }
    if (pipeline.MemberShaders() & Pipeline::Shaders::TESSELATION_EVALUATION_SHADER)
    {
        shaderObjects.emplace_back(Shader::TESSELATION_EVALUATION_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::TESSELATION_EVALUATION_SHADER, shaderObjects.back());
    }
    if (pipeline.MemberShaders() & Pipeline::Shaders::COMPUTE_SHADER)
    {
        shaderObjects.emplace_back(Shader::COMPUTE_SHADER, label, source, Defines);
        if (!shaderObjects.back().IsComplete()) return false;
        
        shaderRefs.emplace_back(Shader::COMPUTE_SHADER, shaderObjects.back());
    }
    
    pipeline.Data(shaderRefs);
    
    return pipeline.IsComplete();
}

PipelineMatrix::Name PipelineMatrix::MatrixDesc::AddParameterName(std::string_view defineName)
{
    m_Parameters.emplace_back(std::pair<std::string_view, std::vector<std::string_view>>(defineName, {})); return m_Parameters.size() - 1;
}

PipelineMatrix::Name PipelineMatrix::MatrixDesc::AddParameterValue(Name parameterName, std::string_view defineValue)
{
    AssertOrError(parameterName < m_Parameters.size(), "No such parapmeter")

    m_Parameters[parameterName].second.emplace_back(defineValue);
    return m_Parameters[parameterName].second.size() - 1;
}

PipelineMatrix::Name PipelineMatrix::MatrixDesc::FindParameterName(std::string_view defineName) const
{
    for (Name i = 0; i < m_Parameters.size(); i++)
    {
        if (m_Parameters[i].first == defineName) return i;
    }

    return -1;
}

PipelineMatrix::Name PipelineMatrix::MatrixDesc::FindParameterValue(Name parameterName, std::string_view defineValue) const
{
    AssertOrError(parameterName < m_Parameters.size(), "No such parapmeter")

    for (Name i = 0; i < m_Parameters[parameterName].second.size(); i++)
    {
        if (m_Parameters[parameterName].second[i] == defineValue) return i;
    }

    return -1;
}

std::string_view PipelineMatrix::MatrixDesc::GetParameterName(Name parameterName) const
{
    AssertOrError(parameterName < m_Parameters.size(), "No such parapmeter")

    return m_Parameters[parameterName].first;
}

std::string_view PipelineMatrix::MatrixDesc::GetParameterValue(Name parameterName, uint64_t defineValue) const
{
    AssertOrError(parameterName < m_Parameters.size(), "No such parapmeter")
    AssertOrError(defineValue < m_Parameters[parameterName].second.size(), "No such parameter value")

    return m_Parameters[parameterName].second[defineValue];
}

void PipelineMatrix::VariantDesc::Set(const MatrixDesc& Ref)
{
    m_Parameters.resize(Ref.m_Parameters.size());

    for (size_t i = 0; i < Ref.m_Parameters.size(); ++i)
    {
        m_Parameters[i] = 0;
    }
}

void PipelineMatrix::VariantDesc::SetParameter(Name param, Name Value)
{
    AssertOrErrorCall(param < m_Parameters.size(),  return;, "No such parapmeter")

    m_Parameters[param] = Value;
}

void PipelineMatrix::VariantDesc::UnsetParameter(Name param)
{
    AssertOrErrorCall(param < m_Parameters.size(),  return;, "No such parapmeter")

    m_Parameters[param] = 0;
}

void PipelineMatrix::VariantDesc::Reset()
{
    for (size_t i = 0; i < m_Parameters.size(); ++i)
        m_Parameters[i] = 0;
}

void PipelineMatrix::VariantDesc::Clear()
{
    m_Parameters.clear();
}

const Pipeline& PipelineMatrix::GetVariant(const VariantDesc& variant)
{
    uint32_t elt = m_Head;
    size_t param = 0;
    while ((elt & (1 << 31)) == 0)
    {
        elt = m_PipelineTree[elt + variant.m_Parameters[param++]];
    }

    return m_Pipelines[SetBoolAt(elt, 31, false)];
}

PipelineMatrix PipelineMatrixFromString(std::string_view label, Pipeline::Shaders shaders, std::string_view source,
                                        const PipelineMatrix::MatrixDesc& paramMatrix, Shader::DefinesView Defines)
{
    std::vector<size_t> paramValuesCountStack(paramMatrix.m_Parameters.size());
    size_t totalParamValuesCount = 1;

    for (size_t i = 0; i < paramMatrix.m_Parameters.size(); i++)
    {
        const auto & parameter = paramMatrix.m_Parameters[i];
        paramValuesCountStack[i] = parameter.second.size();
        totalParamValuesCount *= parameter.second.size();
    }

    PipelineMatrix pipelines;
    pipelines.m_ParameterMatrix = paramMatrix;

    pipelines.m_Pipelines.reserve(totalParamValuesCount);
    pipelines.m_PipelineTree.resize(totalParamValuesCount);

    for (size_t i = 0; i < totalParamValuesCount; i++)
    {
        pipelines.m_PipelineTree[i] = SetBoolAt(static_cast<uint32_t>(i), 31, true);
    }

    // Construct tree by building through stack
    // Recursively group parameter in reverse order of the paramValuesCountStack list so that
    // when going through each layer of the parameter tree, we iterate on parameters in sequential order
    // for easier iteration
    size_t paramLayerSize = totalParamValuesCount;
    size_t paramLayerOffset = 0;
    pipelines.m_Head = 0;
    if (!paramValuesCountStack.empty())
    for (size_t i = paramValuesCountStack.size() - 1; i-- > 0;)
    {
        size_t paramCount = paramValuesCountStack[i];
        size_t clusterCount = paramLayerSize / paramCount;
        
        paramLayerOffset += paramLayerSize;
        // paramLayerSize = clusterCount;
        pipelines.m_PipelineTree.resize(pipelines.m_PipelineTree.size() + clusterCount);

        size_t currentCluster = paramLayerOffset - paramLayerSize;
        for (size_t j = paramLayerOffset; j < paramLayerOffset + clusterCount; j++)
        {
            pipelines.m_PipelineTree[j] = currentCluster;
            currentCluster += paramCount;
        }

        if (i == 0)
        {
            pipelines.m_Head = paramLayerOffset;
        }
    }

    Shader::DefineDynArray variantDefines; variantDefines.resize(paramMatrix.m_Parameters.size() + Defines.size());
    PipelineMatrix::VariantDesc variant; variant.Set(paramMatrix);
    
    struct LayerStack
    {
        size_t Start, End, At, Param;
    };

    std::stack<LayerStack> iterationStack;
    iterationStack.push({.Start = pipelines.m_Head, .End = pipelines.m_PipelineTree.size(), .At = pipelines.m_Head, .Param = 0});

    // Compile all variants using the tree to explore parameters
    EngineLoggerLogF("Compiling pipeline matrix \"%.*s\" with \"%llu\" variant(s)", (int)(label.size()), label.data(), totalParamValuesCount);
    while (iterationStack.empty() == false)
    {
continue_tree_iteration:
        LayerStack& currentLayer = iterationStack.top();

        if (pipelines.m_PipelineTree[currentLayer.Start] & (1 << 31))
        {
            // Leafs
            // Compile the pipelines
            for (size_t i = currentLayer.Start; i < currentLayer.End; i++)
            {
                uint32_t paramNode = pipelines.m_PipelineTree[i];
                variant.SetParameter(currentLayer.Param, i - currentLayer.Start);

                // Resolve defines
                variantDefines.clear();
                for (const auto& define : Defines) variantDefines.push_back(define);
                for (size_t p = 0; p < variant.m_Parameters.size(); p++)
                {
                    variantDefines.emplace_back(paramMatrix.m_Parameters[p].first, paramMatrix.m_Parameters[p].second[variant.m_Parameters[p]]);
                }

                // Create pipeline
                AssertOrError(pipelines.m_Pipelines.size() == SetBoolAt(paramNode, 31, false), "Out of order pipeline iteration")
                pipelines.m_Pipelines.emplace_back(PipelineFromString(label, shaders, source, variantDefines));
            }
        }
        else
        {
            // Node
            // queue up next layers
            if (currentLayer.At < currentLayer.End)
            {
                uint32_t paramNode = pipelines.m_PipelineTree[currentLayer.At];
                variant.SetParameter(currentLayer.Param, currentLayer.At - currentLayer.Start);
                ++currentLayer.At;

                iterationStack.push({.Start = paramNode, .End = paramNode + paramValuesCountStack[currentLayer.Param + 1], .At = paramNode, .Param = currentLayer.Param + 1});
                goto continue_tree_iteration;
            }
        }
        
        iterationStack.pop();
    }

    return pipelines;
}

bool PipelineMatrixUpdateFromString(PipelineMatrix& pipelines, std::string_view source, Shader::DefinesView Defines)
{
    std::vector<size_t> paramValuesCountStack(pipelines.m_ParameterMatrix.m_Parameters.size());
    size_t totalParamValuesCount = 1;

    for (size_t i = 0; i < pipelines.m_ParameterMatrix.m_Parameters.size(); i++)
    {
        const auto & parameter = pipelines.m_ParameterMatrix.m_Parameters[i];
        paramValuesCountStack[i] = parameter.second.size();
        totalParamValuesCount *= parameter.second.size();
    }
    
    Shader::DefineDynArray variantDefines; variantDefines.resize(pipelines.m_ParameterMatrix.m_Parameters.size() + Defines.size());
    PipelineMatrix::VariantDesc variant; variant.Set(pipelines.m_ParameterMatrix);
    
    struct LayerStack
    {
        size_t Start, End, At, Param;
    };

    std::stack<LayerStack> iterationStack;
    iterationStack.push({.Start = pipelines.m_Head, .End = pipelines.m_PipelineTree.size(), .At = pipelines.m_Head, .Param = 0});

    // Compile all variants using the tree to explore parameters
    bool status = true;
    EngineLoggerLogF("Recompiling pipeline matrix with \"%llu\" variant(s)",  totalParamValuesCount);
    while (iterationStack.empty() == false)
    {
continue_tree_iteration:
        LayerStack& currentLayer = iterationStack.top();

        if (pipelines.m_PipelineTree[currentLayer.Start] & (1 << 31))
        {
            // Leafs
            // Compile the pipelines
            for (size_t i = currentLayer.Start; i < currentLayer.End; i++)
            {
                uint32_t paramNode = pipelines.m_PipelineTree[i];
                variant.SetParameter(currentLayer.Param, i - currentLayer.Start);

                // Resolve defines
                variantDefines.clear();
                for (const auto& define : Defines) variantDefines.push_back(define);
                for (size_t i = 0; i < variant.m_Parameters.size(); i++)
                {
                    variantDefines.emplace_back(pipelines.m_ParameterMatrix.m_Parameters[i].first, pipelines.m_ParameterMatrix.m_Parameters[i].second[variant.m_Parameters[i]]);
                }

                // Create pipeline
                status &= PipelineUpdateFromString(pipelines.m_Pipelines[SetBoolAt(paramNode, 31, false)], source, variantDefines);
            }
        }
        else
        {
            // Node
            // queue up next layers
            if (currentLayer.At < currentLayer.End)
            {
                uint32_t paramNode = pipelines.m_PipelineTree[currentLayer.At];
                variant.SetParameter(currentLayer.Param, currentLayer.At - currentLayer.Start);
                ++currentLayer.At;

                iterationStack.push({.Start = paramNode, .End = paramValuesCountStack[currentLayer.Param + 1], .At = paramNode, .Param = currentLayer.Param + 1});
                goto continue_tree_iteration;
            }
        }
        
        iterationStack.pop();
    }

    return status;
}
