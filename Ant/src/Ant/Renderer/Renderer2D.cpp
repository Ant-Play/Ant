#include "antpch.h"
#include "Ant/Renderer/Renderer2D.h"

#include "Ant/Renderer/VertexArray.h"
#include "Ant/Renderer/Shader.h"
#include "Ant/Renderer/RendererCommand.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Ant{
	
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	struct Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		ANT_PROFILE_FUNCTION();

		s_Data = new struct Renderer2DStorage();

		s_Data->QuadVertexArray = VertexArray::Create();

		float verticesTmp[5 * 4] = {
			 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f
		};

		Ant::Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(verticesTmp, sizeof(verticesTmp));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ Ant::ShaderDataType::Float2, "a_Texcoord" }
			});
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);
		unsigned int indicesTmp[6] = { 0, 1, 2, 0, 2, 3 };
		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(indicesTmp, std::size(indicesTmp));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		ANT_PROFILE_FUNCTION();

		delete s_Data;
	}


	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		ANT_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}


	void Renderer2D::EndScene()
	{
		ANT_PROFILE_FUNCTION();
	}


	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		ANT_PROFILE_FUNCTION();

		s_Data->TextureShader->SetFloat4("u_Color", color);
		s_Data->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RendererCommand::DrawIndexed(s_Data->QuadVertexArray);
	}


	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		ANT_PROFILE_FUNCTION();

		s_Data->TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));
		texture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RendererCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}
