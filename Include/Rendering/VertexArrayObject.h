#pragma once

#include "Shared/Annotations.h"
#include "GLHelper.h"

class VertexArrayObject
{
public:
	// TODO complete implementation for geometry
	// For now only supports dummy VAO

	VertexArrayObject();
	~VertexArrayObject();

	INLINE GLuint Handle() const
	{
		return m_VAO;
	}

private:
	GLuint m_VAO;
};

void Bind(const VertexArrayObject& shader);
void UnBind(const VertexArrayObject& shader);