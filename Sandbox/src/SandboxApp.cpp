#include <Ant.h>
#include <iostream>


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

				out vec3 v_Position;
				out vec4 v_Color;
				
				void main()
				{
					gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

		m_Shader.reset(new Ant::Shader(vertexSrc, fragmentSrc));


		tmpVA.reset(Ant::VertexArray::Create());

		float verticesTmp[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 -0.75f, 0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			 0.75f,  -0.75f, 0.0f
		};

		Ant::Ref<Ant::VertexBuffer> tmpVB;
		tmpVB.reset(Ant::VertexBuffer::Create(verticesTmp, sizeof(verticesTmp)));
		tmpVB->SetLayout({ { Ant::ShaderDataType::Float3, "a_Position" } });
		tmpVA->AddVertexBuffer(tmpVB);
		unsigned int indicesTmp[6] = { 0, 1, 2, 0, 2, 3 };
		Ant::Ref<Ant::IndexBuffer> tmpIB;
		tmpIB.reset(Ant::IndexBuffer::Create(indicesTmp, std::size(indicesTmp)));
		tmpVA->SetIndexBuffer(tmpIB);


		std::string tmpVertexSrc = R"(
				#version 330 core

				layout(location = 0) in vec3 a_Position;

				uniform mat4 u_ViewProjection;
				
				void main()
				{
					gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
				}
		)";

		std::string tmpFragmentSrc = R"(
				#version 330 core

				layout(location = 0) out vec4 color;


				void main()
				{
					color = vec4(0.8, 0.3, 0.2, 1.0);
				}
		)";

		tmpShader.reset(new Ant::Shader(tmpVertexSrc, tmpFragmentSrc));
	}

	void OnUpdate()
	{
		if (Ant::Input::IsKeyPressed(Ant::Key::Left))
			m_CameraPosition.x -= m_CameraSpeed;
		if (Ant::Input::IsKeyPressed(Ant::Key::Right))
			m_CameraPosition.x += m_CameraSpeed;
		if (Ant::Input::IsKeyPressed(Ant::Key::Up))
			m_CameraPosition.y += m_CameraSpeed;
		if (Ant::Input::IsKeyPressed(Ant::Key::Down))
			m_CameraPosition.y -= m_CameraSpeed;
		if (Ant::Input::IsKeyPressed(Ant::Key::A))
			m_CameraRotation -= m_CameraSpeed;
		if (Ant::Input::IsKeyPressed(Ant::Key::D))
			m_CameraRotation += m_CameraSpeed;


		Ant::RendererCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Ant::RendererCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Ant::Renderer::BeginScene(m_Camera);

		Ant::Renderer::Submit(tmpShader, tmpVA);
		Ant::Renderer::Submit(m_Shader, m_VertexArray);

		Ant::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
	}

	void OnEvent(Ant::Event& event)
	{
	}


private:
	Ant::Ref<Ant::Shader> m_Shader;
	Ant::Ref<Ant::VertexArray> m_VertexArray;
	Ant::Ref<Ant::VertexBuffer> m_VertexBuffer;
	Ant::Ref<Ant::IndexBuffer> m_IndexBuffer;

	Ant::Ref<Ant::Shader> tmpShader;
	Ant::Ref<Ant::VertexArray> tmpVA;

	Ant::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;
	float m_CameraSpeed = 0.1f;
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
