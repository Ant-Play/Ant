//
// Note:	this file is to be included in client applications ONLY
//			NEVER include this file anywhere in the engine codebase
//
#pragma once

// For use by Ant application
#include "Ant/Core/Base.h"

#include "Ant/Core/Application.h"
#include "Ant/Core/Layer.h"
#include "Ant/Core/Log.h"

#include "Ant/Core/Timestep.h"
#include "Ant/Core/Inputs.h"
#include "Ant/Core/KeyCodes.h"
#include "Ant/Core/MouseCodes.h"

#include "Ant/ImGui/ImGuiLayer.h"

#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Scene/Components.h"
#include "Ant/Scene/ScriptableEntity.h"

// ----Renderer-----------------------------------
#include "Ant/Renderer/Renderer.h"
#include "Ant/Renderer/Renderer2D.h"
#include "Ant/Renderer/RenderCommand.h"

#include "Ant/Renderer/Buffer.h"
#include "Ant/Renderer/Shader.h"
#include "Ant/Renderer/FrameBuffer.h"
#include "Ant/Renderer/Texture.h"
#include "Ant/Renderer/SubTexture.h"
#include "Ant/Renderer/VertexArray.h"

#include "Ant/Renderer/OrthographicCamera.h"
#include "Ant/Renderer/OrthographicCameraController.h"
#include "Ant/Renderer/EditorCamera.h"
// ---------------------------------------------------