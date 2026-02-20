#include "VertexArrayObject.h"

VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &m_VAO);
}

VertexArrayObject::~VertexArrayObject()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void Bind(const VertexArrayObject& shader)
{
	glBindVertexArray(shader.Handle());
}

void UnBind(const VertexArrayObject& shader)
{
	glBindVertexArray(0);
}
