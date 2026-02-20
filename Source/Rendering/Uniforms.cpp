#include "Uniforms.h"

GLint GetUniformLocation(const Pipeline& Pipeline, std::string_view name)
{
    GLint location = glGetUniformLocation(Pipeline.Handle(), name.data());
    if(location < 0)
    {
#ifdef GL_VERSION_4_3
        char label[1024];
        glGetObjectLabel(GL_PROGRAM, Pipeline.Handle(), sizeof(label), nullptr, label);
        AssertOrWarnCallF(location >= 0,, "Cannot find Uniform %s %u \"%s\"", label, Pipeline.Handle(), name.data())
#else
        AssertOrWarnCallF(location >= 0,, "Uniform %s %u \"%s\"", "program", Pipeline.Handle(), name.data());
#endif

        return -1;
    }

    return location;
}

void SetUniform(const Pipeline& Pipeline, std::string_view name, bool value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, int value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, unsigned int value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, float value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, double value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2i value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3i value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3i value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4i value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix3f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix3d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix4f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix4d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Transform4f value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Transform4d value)
{
    SetUniform(GetUniformLocation(Pipeline, name), value);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture2D& texture)
{
    SetUniform(GetUniformLocation(Pipeline, name), TextureUnit, texture);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture3D& texture)
{
    SetUniform(GetUniformLocation(Pipeline, name), TextureUnit, texture);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture2D& texture, const Sampler& sampler)
{
    SetUniform(GetUniformLocation(Pipeline, name), TextureUnit, texture, sampler);
}
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture3D& texture, const Sampler& sampler)
{
    SetUniform(GetUniformLocation(Pipeline, name), TextureUnit, texture, sampler);
}

void SetUniform(GLint Location, bool value)
{
    if(Location >= 0) glUniform1ui(Location, value ? 1 : 0);
}
void SetUniform(GLint Location, int value)
{
    if(Location >= 0) glUniform1i(Location, value);
}
void SetUniform(GLint Location, unsigned int value)
{
    if(Location >= 0) glUniform1ui(Location, value);
}
void SetUniform(GLint Location, float value)
{
    if(Location >= 0) glUniform1f(Location, value);
}
void SetUniform(GLint Location, double value)
{
    if(Location >= 0) glUniform1d(Location, value);
}
void SetUniform(GLint Location, Math::Vector2i value)
{
    if(Location >= 0) glUniform2iv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector2f value)
{
    if(Location >= 0) glUniform2fv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector2d value)
{
    if(Location >= 0) glUniform2dv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector3i value)
{
    if(Location >= 0) glUniform3iv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector3f value)
{
    if(Location >= 0) glUniform3fv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector3d value)
{
    if(Location >= 0) glUniform3dv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Point3i value)
{
    if(Location >= 0) glUniform3iv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Point3f value)
{
    if(Location >= 0) glUniform3fv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Point3d value)
{
    if(Location >= 0) glUniform3dv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector4i value)
{
    if(Location >= 0) glUniform4iv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector4f value)
{
    if(Location >= 0) glUniform4fv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Vector4d value)
{
    if(Location >= 0) glUniform4dv(Location, 1, value.data());
}
void SetUniform(GLint Location, Math::Matrix3f value)
{
    if(Location >= 0) glUniformMatrix3fv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, Math::Matrix3d value)
{
    if(Location >= 0) glUniformMatrix3dv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, Math::Matrix4f value)
{
    if(Location >= 0) glUniformMatrix4fv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, Math::Matrix4d value)
{
    if(Location >= 0) glUniformMatrix4dv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, Math::Transform4f value)
{
    if(Location >= 0) glUniformMatrix4fv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, Math::Transform4d value)
{
    if(Location >= 0) glUniformMatrix4dv(Location, 1, GL_FALSE, value.data());
}
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture2D& texture)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + TextureUnit))
    Bind(texture);

    GLCall(glUniform1i(Location, TextureUnit))
}
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture3D& texture)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + TextureUnit))
    Bind(texture);

    GLCall(glUniform1i(Location, TextureUnit))
}
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture2D& texture, const Sampler& sampler)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + TextureUnit))
    Bind(texture);
    GLCall(glBindSampler(TextureUnit, sampler.Handle()))

    GLCall(glUniform1i(Location, TextureUnit))
}
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture3D& texture, const Sampler& sampler)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + TextureUnit))
    Bind(texture);
    GLCall(glBindSampler(TextureUnit, sampler.Handle()))

    GLCall(glUniform1i(Location, TextureUnit))
}

void SetUniform(GLint Binding, const UniformBuffer& buffer)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, Binding, buffer.Handle());
}
