#pragma once

#include <cstdint>
#include <string_view>
#include <span>
#include <filesystem>

#include "GLHelper.h"
#include "Shared/Annotations.h"

std::string ShaderFileToString(const std::filesystem::path& filename);

class Shader
{
public:
    using DefinesView = std::span < const std::pair<std::string_view, std::string_view> >;

    enum Type : uint8_t
    {
        VERTEX_SHADER = 0,
        FRAGMENT_SHADER,
        GEOMETRY_SHADER,
        TESSELATION_CONTROL_SHADER,
        TESSELATION_EVALUATION_SHADER,
        COMPUTE_SHADER,

        __Count,
        __RasterBegin = VERTEX_SHADER,
        __RasterEnd = COMPUTE_SHADER,
        __ComputeBegin = COMPUTE_SHADER,
        __ComputeEnd = __Count,
    };

    Shader(Type type, std::string_view SourceCode, DefinesView Defines = {});
    ~Shader();

    Shader& operator=(Shader other)
    {
        using std::swap;

        swap(m_Shader, other.m_Shader);

        return *this;
    }

    INLINE GLuint Handle() const {return m_Shader;}

    bool IsComplete() const;
    
private:
    GLuint m_Shader;
};

Shader ShaderFromFile(Shader::Type type, const std::filesystem::path& filename, Shader::DefinesView Defines = {});