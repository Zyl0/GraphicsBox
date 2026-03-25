#include "Pipelines.h"

#include <vector>

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

        glAttachShader(m_Pipeline, shader.second.Handle());
    }

    glLinkProgram(m_Pipeline);
    glValidateProgram(m_Pipeline);

    // Verify
    GLint status;
    glGetProgramiv(m_Pipeline, GL_LINK_STATUS, &status);
    if(status == GL_TRUE)
        return;
    EngineLoggerError("Pipeline linkage failed.");
    
    int shaders_max= 0;
    glGetProgramiv(m_Pipeline, GL_ATTACHED_SHADERS, &shaders_max);
    AssertOrErrorCall(shaders_max > 0, return, "No shader in Pipeline.")

    GLint value= 0;
    glGetProgramiv(m_Pipeline, GL_INFO_LOG_LENGTH, &value);

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
