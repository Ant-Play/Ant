//
// Note:	this file is to be included in client applications ONLY
//			NEVER include this file anywhere in the engine codebase
//
#pragma once

#include "Ant/Core/Application.h"
#include "Ant/Core/Log.h"
#include "Ant/Core/Inputs.h"
#include "Ant/Core/Timestep.h"
#include "Ant/Core/Timer.h"

#include "Ant/Core/Events/Event.h"
#include "Ant/Core/Events/ApplicationEvent.h"
#include "Ant/Core/Events/KeyEvent.h"
#include "Ant/Core/Events/MouseEvent.h"

#include "Ant/Core/Math/AABB.h"
#include "Ant/Core/Math/Ray.h"

#include "imgui/imgui.h"

// --- Ant Render API ------------------------------
#include "Ant/Renderer/Renderer.h"
#include "Ant/Renderer/SceneRenderer.h"
#include "Ant/Renderer/RenderPass.h"
#include "Ant/Renderer/Framebuffer.h"
#include "Ant/Renderer/VertexBuffer.h"
#include "Ant/Renderer/IndexBuffer.h"
#include "Ant/Renderer/Pipeline.h"
#include "Ant/Renderer/Texture.h"
#include "Ant/Renderer/Shader.h"
#include "Ant/Renderer/Mesh.h"
#include "Ant/Renderer/Camera.h"
#include "Ant/Renderer/Material.h"
// ---------------------------------------------------

// Scenes
#include "Ant/Scene/Entity.h"
#include "Ant/Scene/Scene.h"
#include "Ant/Scene/SceneCamera.h"
#include "Ant/Scene/SceneSerializer.h"
#include "Ant/Scene/Components.h"
