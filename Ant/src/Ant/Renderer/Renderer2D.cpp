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
		Ref<Shader> FlatColorShader;
	};

	struct Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new struct Renderer2DStorage();

		s_Data->QuadVertexArray = VertexArray::Create();

		float verticesTmp[5 * 4] = {
			 -0.5f, -0.5f, 0.0f,
			 -0.5f, 0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			 0.5f,  -0.5f, 0.0f
		};

		Ant::Ref<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(verticesTmp, sizeof(verticesTmp)));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
			});
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);
		unsigned int indicesTmp[6] = { 0, 1, 2, 0, 2, 3 };
		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(indicesTmp, std::size(indicesTmp)));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

		s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColorShader.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}


	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_Transform", glm::mat4(1.0f));
	}


	void Renderer2D::EndScene()
	{

	}


	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);

		s_Data->QuadVertexArray->Bind();
		RendererCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}
