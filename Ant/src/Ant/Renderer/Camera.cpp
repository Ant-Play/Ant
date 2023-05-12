#include "antpch.h"
#include "Ant/Renderer/Camera.h"

namespace Ant{

	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}
}