#include "VertexArrayObject.h"

unsigned int VertexArrayObject::Layout::Element::GetSizeOfType(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT:
	case GL_UNSIGNED_INT:
		return 4;
	case GL_UNSIGNED_BYTE:
		return 1;

	SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported buffer data type for vertex array object")
	}
}

template <>
void VertexArrayObject::Layout::Push<float>(unsigned int count)
{
	m_elements.push_back({count, GL_FLOAT, GL_FALSE});
	m_stride += count * Element::GetSizeOfType(GL_FLOAT);
}

template <>
void VertexArrayObject::Layout::Push<unsigned>(unsigned int count)
{
	m_elements.push_back({count, GL_UNSIGNED_INT, GL_FALSE});
	m_stride += count * Element::GetSizeOfType(GL_UNSIGNED_INT);
}

template <>
void VertexArrayObject::Layout::Push<unsigned char>(unsigned int count)
{
	m_elements.push_back({count, GL_UNSIGNED_BYTE, GL_TRUE});
	m_stride += count * Element::GetSizeOfType(GL_UNSIGNED_BYTE);
}

template <>
void VertexArrayObject::Layout::Push<Math::Vector2t<float>>(unsigned int count)
{
	m_elements.push_back({count * 2, GL_FLOAT, GL_FALSE});
	m_stride += count * 2 * Element::GetSizeOfType(GL_FLOAT);
}

template <>
void VertexArrayObject::Layout::Push<Math::Vector2t<double>>(unsigned int count)
{
	m_elements.push_back({count * 2, GL_DOUBLE, GL_FALSE});
	m_stride += count * 2 * Element::GetSizeOfType(GL_DOUBLE);
}

template <>
void VertexArrayObject::Layout::Push<Math::Point3t<float>>(unsigned int count)
{
	m_elements.push_back({count * 3, GL_FLOAT, GL_FALSE});
	m_stride += count * 3 * Element::GetSizeOfType(GL_FLOAT);
}

template <>
void VertexArrayObject::Layout::Push<Math::Point3t<double>>(unsigned int count)
{
	m_elements.push_back({count * 3, GL_DOUBLE, GL_FALSE});
	m_stride += count * 3 * Element::GetSizeOfType(GL_DOUBLE);
}

template <>
void VertexArrayObject::Layout::Push<Math::Vector3t<float>>(unsigned int count)
{
	m_elements.push_back({count * 3, GL_FLOAT, GL_FALSE});
	m_stride += count * 3 * Element::GetSizeOfType(GL_FLOAT);
}

template <>
void VertexArrayObject::Layout::Push<Math::Vector3t<double>>(unsigned int count)
{
	m_elements.push_back({count * 3, GL_DOUBLE, GL_FALSE});
	m_stride += count * 3 * Element::GetSizeOfType(GL_DOUBLE);
}

template <>
void VertexArrayObject::Layout::Push<Math::Vector4t<float>>(unsigned int count)
{
	m_elements.push_back({count * 4, GL_FLOAT, GL_FALSE});
	m_stride += count * 4 * Element::GetSizeOfType(GL_FLOAT);
}

VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &m_VAO);
}

VertexArrayObject::~VertexArrayObject()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void VertexArrayObject::BufferData(const VertexBuffer& vertex_buffer, const Layout& layout)
{
	Bind(*this);
	Bind(vertex_buffer);
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto &element = elements[i];
		GLCall(glEnableVertexAttribArray(i))
		GLCall(glVertexAttribPointer(i,element.count,element.type, element.normalized, layout.GetStride(), reinterpret_cast<const void*>(offset)))
        
		offset += element.count * Layout::Element::GetSizeOfType(element.type);
	}
	UnBind(vertex_buffer);
	UnBind(*this);
}

void VertexArrayObject::BufferData(std::span<const VertexBuffer> vertex_buffers, const Layout& layout)
{
	Bind(*this);

	for (unsigned int i = 0; i < vertex_buffers.size(); i++)
	{
		const auto &buffer = vertex_buffers[i];
		const auto &element = layout.GetElements()[i];

		Bind(buffer);
		GLCall(glVertexAttribPointer(i, 
			element.count, element.type,
			element.normalized,
			0,
			nullptr
		))
		GLCall(glEnableVertexAttribArray(i))
		UnBind(buffer);
	}
	UnBind(*this);
}

void VertexArrayObject::BufferData(std::span<const VertexBuffer> vertex_buffers, std::span<const Layout> layouts)
{
	AssertOrErrorCall(vertex_buffers.size() == layouts.size(), return;, "VertexBuffers count and layout count missmatch")
	
	Bind(*this);

	for (unsigned int i = 0, index = 0; i < vertex_buffers.size(); i++)
	{
		const auto &buffer = vertex_buffers[i];
		const auto &layout = layouts[i];

		Bind(buffer);
		const auto& elements = layout.GetElements();
		unsigned int offset = 0;
		for (unsigned int j = 0; j < elements.size(); j++)
		{
			const auto &element = elements[j];
			GLCall(glEnableVertexAttribArray(index))
			GLCall(glVertexAttribPointer(index++,element.count,element.type, element.normalized, layout.GetStride(), reinterpret_cast<const void*>(offset)))
        
			offset += element.count * Layout::Element::GetSizeOfType(element.type);
		}
		UnBind(buffer);
	}
	UnBind(*this);
}

void Bind(const VertexArrayObject& shader)
{
	glBindVertexArray(shader.Handle());
}

void UnBind(const VertexArrayObject& shader)
{
	glBindVertexArray(0);
}
