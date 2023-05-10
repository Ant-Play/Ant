#include "antpch.h"
#include "Ant/Platform/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

#include "Ant/Renderer/Renderer.h"

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
		Renderer::Submit([this]() {
			glCreateVertexArrays(1, &m_RendererID);
			});
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		Renderer::Submit([this]() {
			glDeleteVertexArrays(1, &m_RendererID);
			});
	}

	void OpenGLVertexArray::Bind() const
	{
		Renderer::Submit([this]() {
			glBindVertexArray(m_RendererID);
			});
	}

	void OpenGLVertexArray::Unbind() const
	{
		Renderer::Submit([this]() {
			glBindVertexArray(0);
			});
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		ANT_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		Bind();
		vertexBuffer->Bind();

		Renderer::Submit([this, vertexBuffer]() {
			const auto& layout = vertexBuffer->GetLayout();
			for (const auto& element : layout)
			{
				auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
				glEnableVertexAttribArray(m_VertexBufferIndex);
				if (glBaseType == GL_INT)
				{
					glVertexAttribIPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						glBaseType,
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset);
				}
				else
				{
					glVertexAttribPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						glBaseType,
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset);
				}
				m_VertexBufferIndex++;
			}
			});
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		Bind();
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}
