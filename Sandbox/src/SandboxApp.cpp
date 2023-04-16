#include <Ant.h>
#include <iostream>

#include <imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Ant::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_VertexArray.reset(Ant::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};


		m_VertexBuffer.reset(Ant::VertexBuffer::Create(vertices, sizeof(vertices)));

		Ant::BufferLayout layout = {
						{ Ant::ShaderDataType::Float3, "a_Position" },
						{ Ant::ShaderDataType::Float4, "a_Color" }
		};

		m_VertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		m_IndexBuffer.reset(Ant::IndexBuffer::Create(indices, std::size(indices)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::string vertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;
				layout(location = 1) in vec4 a_Color;
				
				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				out vec3 v_Position;
				out vec4 v_Color;
				
				void main()
				{
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
					v_Position = a_Position;
					v_Color = a_Color;
				}
		)";

		std::string fragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;

				in vec3 v_Position;
				in vec4 v_Color;

				void main()
				{
					color = v_Color;
				}
		)";

		m_Shader.reset(Ant::Shader::Create(vertexSrc, fragmentSrc));


		tmpVA.reset(Ant::VertexArray::Create());

		float verticesTmp[5 * 4] = {
			 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f
		};

		Ant::Ref<Ant::VertexBuffer> tmpVB;
		tmpVB.reset(Ant::VertexBuffer::Create(verticesTmp, sizeof(verticesTmp)));
		tmpVB->SetLayout({ 
			{ Ant::ShaderDataType::Float3, "a_Position" },
			{ Ant::ShaderDataType::Float2, "a_Texcoord" }
			});
		tmpVA->AddVertexBuffer(tmpVB);
		unsigned int indicesTmp[6] = { 0, 1, 2, 0, 2, 3 };
		Ant::Ref<Ant::IndexBuffer> tmpIB;
		tmpIB.reset(Ant::IndexBuffer::Create(indicesTmp, std::size(indicesTmp)));
		tmpVA->SetIndexBuffer(tmpIB);

		std::string tmpVertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;
				
				out vec3 v_Position;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;
				
				void main()
				{
					gl_Position = u_ViewProjection * u_Transform *  vec4(a_Position, 1.0);
				}
		)";

		std::string tmpFragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;

				uniform vec3 u_Color;

				void main()
				{
					//color = vec4(0.8, 0.3, 0.2,1.0);
					color = vec4(u_Color, 1.0);
				}
		)";

		tmpShader.reset(Ant::Shader::Create(tmpVertexSrc, tmpFragmentSrc));


		std::string texVertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;
				layout(location = 1) in vec2 a_TexCoord;
				
				out vec3 v_Position;
				out vec2 v_TexCoord;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;
				
				void main()
				{
					v_TexCoord = a_TexCoord;
					gl_Position = u_ViewProjection * u_Transform *  vec4(a_Position, 1.0);
				}
		)";

		std::string texFragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;
	
				in vec2 v_TexCoord;

				uniform sampler2D u_Texture;

				void main()
				{
					//color = vec4(0.8, 0.3, 0.2,1.0);
					color = texture(u_Texture, v_TexCoord);
				}
		)";
		texShader.reset(Ant::Shader::Create(texVertexSrc, texFragmentSrc));

		m_Texture = Ant::Texture2D::Create("assets/textures/Checkerboard.png");
		m_AnotherTexture = Ant::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Ant::OpenGLShader>(texShader)->Bind();
		std::dynamic_pointer_cast<Ant::OpenGLShader>(texShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Ant::Timestep ts)
	{
		if (Ant::Input::IsKeyPressed(Ant::Key::Left))
			m_CameraPosition.x -= m_CameraTranslateSpeed * ts;
		if (Ant::Input::IsKeyPressed(Ant::Key::Right))
			m_CameraPosition.x += m_CameraTranslateSpeed * ts;
		if (Ant::Input::IsKeyPressed(Ant::Key::Up))
			m_CameraPosition.y += m_CameraTranslateSpeed * ts;
		if (Ant::Input::IsKeyPressed(Ant::Key::Down))
			m_CameraPosition.y -= m_CameraTranslateSpeed * ts;
		if (Ant::Input::IsKeyPressed(Ant::Key::A))
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		if (Ant::Input::IsKeyPressed(Ant::Key::D))
			m_CameraRotation += m_CameraRotationSpeed * ts;


		Ant::RendererCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Ant::RendererCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Ant::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Ant::OpenGLShader>(tmpShader)->Bind();
		std::dynamic_pointer_cast<Ant::OpenGLShader>(tmpShader)->UploadUniformFloat3("u_Color", m_TmpColor);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Ant::Renderer::Submit(tmpShader, tmpVA, transform);
			}
		}
		//Ant::Renderer::Submit(tmpShader, tmpVA);
		//Ant::Renderer::Submit(m_Shader, m_VertexArray);
		m_Texture->Bind();
		Ant::Renderer::Submit(texShader, tmpVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));
		m_AnotherTexture->Bind();
		Ant::Renderer::Submit(texShader, tmpVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));

		Ant::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_TmpColor));
		ImGui::End();
	}

	void OnEvent(Ant::Event& event)
	{
	}


private:
	Ant::Ref<Ant::Texture2D> m_Texture, m_AnotherTexture;
	Ant::Ref<Ant::Shader> m_Shader, tmpShader, texShader;
	Ant::Ref<Ant::VertexArray> m_VertexArray, tmpVA;
	Ant::Ref<Ant::VertexBuffer> m_VertexBuffer;
	Ant::Ref<Ant::IndexBuffer> m_IndexBuffer;

	Ant::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;
	float m_CameraTranslateSpeed = 10.0f;

	glm::vec3 m_TmpColor = { 0.8, 0.3, 0.2 };
};

class Sandbox : public Ant::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer);
	}

	~Sandbox()
	{

	}


};

Ant::Application* Ant::CreateApplication()
{
	return new Sandbox();
}
