#include "antpch.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Ant {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Ant::ShaderDataType::Float:		return GL_FLOAT;
			case Ant::ShaderDataType::Float2:		return GL_FLOAT;
			case Ant::ShaderDataType::Float3:		return GL_FLOAT;
			case Ant::ShaderDataType::Float4:		return GL_FLOAT;
			case Ant::ShaderDataType::Mat3:			return GL_FLOAT;
			case Ant::ShaderDataType::Mat4:			return GL_FLOAT;
			case Ant::ShaderDataType::Int:			return GL_INT;
			case Ant::ShaderDataType::Int2:			return GL_INT;
			case Ant::ShaderDataType::Int3:			return GL_INT;
			case Ant::ShaderDataType::Int4:			return GL_INT;
			case Ant::ShaderDataType::Bool:			return GL_BOOL;
		}

		ANT_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return GL_FLOAT;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
		glBindVertexArray(m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		m_VertexBuffers.clear();
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}


	void OpenGLVertexArray::UnBind() const
	{
		glBindVertexArray(0);
	}


	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		ANT_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffers has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.emplace_back(vertexBuffer);
	}


	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}
