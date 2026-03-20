#pragma once

#include <span>
#include <vector>

#include "Shared/Annotations.h"
#include "Math/Math.h"
#include "GLHelper.h"
#include "VertexBuffer.h"

class VertexArrayObject
{
public:
	class Layout
	{
	public:
		struct Element
		{
			unsigned int count;
			unsigned int type;
			unsigned char normalized;

			static unsigned int GetSizeOfType(GLenum type);
		};

		/**
		 * @brief Add an vertex buffer layout element. Unsuported type
		 * @tparam T 
		 * @param count per vertex element count
		 */
		template<typename T>
		void Push(unsigned int count)
		{
			static_assert(sizeof(T) == 0, "Unsupported member type for VertexBufferLayout.");
		}

		/**
		 * @brief Add a float vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push<float>(unsigned int count);

		/**
		 * @brief Add an unsigned int vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push< unsigned int>(unsigned int count);

		/**
		 * @brief Add an unsigned byte vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push< unsigned char>(unsigned int count);

		/**
		 * @brief Add a vector 2 float vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push<Math::Vector2f>(unsigned int count);

		/**
		 * @brief Add a point 3 float vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push<Math::Point3f>(unsigned int count);

		/**
		 * @brief Add a vector 3 float vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push<Math::Vector3f>(unsigned int count);

		/**
		 * @brief Add a vector 4 float vertex buffer layout element.
		 * @param count per vertex element count
		 */
		template<>
		void Push<Math::Vector4f>(unsigned int count);

		INLINE const std::vector<Element>& GetElements() const { return m_elements; }
		INLINE unsigned int GetStride() const { return m_stride; }
		
	private:
		std::vector<Element> m_elements;
		unsigned int m_stride;
	};
	
	// TODO complete implementation for geometry
	// For now only supports dummy VAO

	VertexArrayObject();
	~VertexArrayObject();

	INLINE GLuint Handle() const
	{
		return m_VAO;
	}

	/**
	 * @brief Add vertex buffer to the Vertex Array Object.
	 * @param vertex_buffer vertex buffer to link
	 * @param layout vertex array layout
	 */
	void BufferData(const VertexBuffer& vertex_buffer, const Layout& layout);

	/**
	* @brief Add vertex buffer to the Vertex Array Object.
	* @param vertex_buffer vertex buffer to link
	*/
	void BufferData(std::span<const VertexBuffer> vertex_buffers, const Layout& layout);

private:
	GLuint m_VAO;
};

void Bind(const VertexArrayObject& shader);
void UnBind(const VertexArrayObject& shader);