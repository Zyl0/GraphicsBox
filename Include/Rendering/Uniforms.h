#pragma once

#include "Math/RMath.h"

#include "GLHelper.h"

#include "Pipelines.h"
#include "Sampler.h"
#include "StorageBuffer.h"
#include "Textures.h"
#include "UniformBuffer.h"

GLint GetUniformLocation(const Pipeline& Pipeline, std::string_view name);

void SetUniform(const Pipeline& Pipeline, std::string_view name, bool value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, int value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, unsigned int value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, float value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, double value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2i value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector2d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3i value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector3d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3i value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Point3d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4i value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Vector4d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix3f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix3d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix4f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Matrix4d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Transform4f value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, Math::Transform4d value);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture2D& texture);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture3D& texture);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const TextureCube& texture);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture2D& texture, const Sampler& sampler);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const Texture3D& texture, const Sampler& sampler);
void SetUniform(const Pipeline& Pipeline, std::string_view name, uint8_t TextureUnit, const TextureCube& texture, const Sampler& sampler);

void SetUniform(GLint Location, bool value);
void SetUniform(GLint Location, int value);
void SetUniform(GLint Location, unsigned int value);
void SetUniform(GLint Location, float value);
void SetUniform(GLint Location, double value);
void SetUniform(GLint Location, Math::Vector2i value);
void SetUniform(GLint Location, Math::Vector2f value);
void SetUniform(GLint Location, Math::Vector2d value);
void SetUniform(GLint Location, Math::Vector3i value);
void SetUniform(GLint Location, Math::Vector3f value);
void SetUniform(GLint Location, Math::Vector3d value);
void SetUniform(GLint Location, Math::Point3i value);
void SetUniform(GLint Location, Math::Point3f value);
void SetUniform(GLint Location, Math::Point3d value);
void SetUniform(GLint Location, Math::Vector4i value);
void SetUniform(GLint Location, Math::Vector4f value);
void SetUniform(GLint Location, Math::Vector4d value);
void SetUniform(GLint Location, Math::Matrix3f value);
void SetUniform(GLint Location, Math::Matrix3d value);
void SetUniform(GLint Location, Math::Matrix4f value);
void SetUniform(GLint Location, Math::Matrix4d value);
void SetUniform(GLint Location, Math::Transform4f value);
void SetUniform(GLint Location, Math::Transform4d value);
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture2D& texture);
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture3D& texture);
void SetUniform(GLint Location, uint8_t TextureUnit, const TextureCube& texture);
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture2D& texture, const Sampler& sampler);
void SetUniform(GLint Location, uint8_t TextureUnit, const Texture3D& texture, const Sampler& sampler);
void SetUniform(GLint Location, uint8_t TextureUnit, const TextureCube& texture, const Sampler& sampler);
void SetUniform(GLint Binding, const UniformBuffer& buffer);
void SetUniform(GLint Binding, const StorageBuffer& buffer);

