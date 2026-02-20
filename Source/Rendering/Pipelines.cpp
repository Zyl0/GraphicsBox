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

    return;

    std::vector<GLuint> shaders(shaders_max, 0);
    glGetAttachedShaders(m_Pipeline, shaders_max, nullptr, &shaders.front());
    for (int i = 0; i < shaders_max; i++)
    {
        GLint value;
        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &value);
        if (value == GL_FALSE)
        {
            GLuint Shader = shaders[i];

            // Gathering shader compiler errors
            glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &value);
            std::vector<char>log(value + 1, 0);
            glGetShaderInfoLog(Shader, static_cast<GLsizei>(log.size()), nullptr, &log.front());

            // Gathering source code
            glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &value);
            std::vector<char> source(value + 1, 0);
            glGetShaderSource(Shader, static_cast<GLsizei>(source.size()), nullptr, &source.front());

            glGetShaderiv(Shader, GL_SHADER_TYPE, &value);
            EngineLoggerErrorF("Failed to compile shader of type %s.", GetShaderTypeDefineName(value));

            std::vector<char> errorLogBuffer;
            unsigned int line = 0, srcIndex = 0;
            for (unsigned int i = 0; log[i] != '\0'; i++)
            {
                unsigned int string_id = 0, line_id = 0, position = 0;
#ifdef PLATFORM_WINDOWS
                if (sscanf_s(&log[i], "%u ( %u ) : %n", &string_id, &line_id, &position) == 2  // nvidia syntax
                    || sscanf_s(&log[i], "%u : %u (%*u) : %n", &string_id, &line_id, &position) == 2  // mesa syntax
                    || sscanf_s(&log[i], "ERROR : %u : %u : %n", &string_id, &line_id, &position) == 2  // ati syntax
                    || sscanf_s(&log[i], "WARNING : %u : %u : %n", &string_id, &line_id, &position) == 2)  // ati syntax
#else // PLATFORM_WINDOWS
                if (sscanf(&log[i], "%u ( %u ) : %n", &string_id, &line_id, &position) == 2  // nvidia syntax
                    || sscanf(&log[i], "%u : %u (%*u) : %n", &string_id, &line_id, &position) == 2  // mesa syntax
                    || sscanf(&log[i], "ERROR : %u : %u : %n", &string_id, &line_id, &position) == 2  // ati syntax
                    || sscanf(&log[i], "WARNING : %u : %u : %n", &string_id, &line_id, &position) == 2)  // ati syntax
#endif // !PLATFORM_WINDOWS
                {
                    if (line_id > line)
                        for (; source[srcIndex] != '\0'; srcIndex++)
                        {
                            errorLogBuffer.push_back(source[srcIndex]);
                            if (source[srcIndex] == '\n')
                                line++;

                            if (line_id == line)
                            {
                                srcIndex++;
                                break;
                            }
                        }

                    errorLogBuffer.push_back('-');
                    errorLogBuffer.push_back('>');
                    for (; log[i] != '\n' && log[i] != '\0'; i++)
                    {
                        errorLogBuffer.push_back(log[i]);
                    }
                    errorLogBuffer.push_back('\n');
                }
            }
            for (; source[srcIndex] != '\0'; srcIndex++)
            {
                errorLogBuffer.push_back(source[srcIndex]);
                if (source[srcIndex] == '\n')
                    line++;
            }
            errorLogBuffer.push_back('\0');
            printf("%s\n", errorLogBuffer.data());
        }
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
