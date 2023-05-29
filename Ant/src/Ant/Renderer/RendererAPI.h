#pragma once

#include "Ant/Renderer/Mesh.h"
#include "Ant/Scene/Scene.h"

#include "RendererCapabilities.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"
#include "UniformBufferSet.h"
#include "PipelineCompute.h"

namespace Ant{

	class SceneRenderer;

	enum class RendererAPIType
	{
		None,
		Vulkan,
		OpenGL
	};

	enum class PrimitiveType
	{
		None = 0, Triangles, Lines
	};

	class RendererAPI
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void RT_InsertGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer, const std::string& label, const glm::vec4& color) = 0;
		virtual void RT_BeginGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer, const std::string& label, const glm::vec4& markerColor) = 0;
		virtual void RT_EndGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer) = 0;

		virtual void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, const Ref<RenderPass>& renderPass, bool explicitClear = false) = 0;
		virtual void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer) = 0;
		virtual void SubmitFullscreenQuad(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material) = 0;
		virtual void SubmitFullscreenQuadWithOverrides(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material, Buffer vertexShaderOverrides, Buffer fragmentShaderOverrides) = 0;

		virtual void SetSceneEnvironment(Ref<SceneRenderer> sceneRenderer, Ref<Environment> environment, Ref<Image2D> shadow, Ref<Image2D> spotShadow) = 0;
		virtual std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath) = 0;
		virtual Ref<TextureCube> CreatePreethamSky(float turbidity, float azimuth, float inclination) = 0;

		virtual void RenderStaticMesh(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<StaticMesh> mesh, uint32_t submeshIndex, Ref<MaterialTable> materialTable, Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, uint32_t instanceCount) = 0;
		//virtual void RenderSubmesh(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Mesh> mesh, uint32_t index, Ref<MaterialTable> materialTable, const glm::mat4& transform) = 0;
		virtual void RenderSubmeshInstanced(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Mesh> mesh, uint32_t index, Ref<MaterialTable> materialTable, Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, const std::vector<Ref<StorageBuffer>>& boneTransformUBs, uint32_t boneTransformsOffset, uint32_t instanceCount) = 0;
		virtual void RenderMeshWithMaterial(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material, Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, const std::vector<Ref<StorageBuffer>>& boneTransformUBs, uint32_t boneTransformsOffset, uint32_t instanceCount, Buffer additionalUniforms = Buffer()) = 0;
		virtual void RenderStaticMeshWithMaterial(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<StaticMesh> staticMesh, uint32_t submeshIndex, Ref<Material> material, Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, uint32_t instanceCount, Buffer additionalUniforms = Buffer()) = 0;
		virtual void RenderQuad(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Material> material, const glm::mat4& transform) = 0;
		virtual void LightCulling(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<PipelineCompute> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Material> material, const glm::uvec3& workGroups) = 0;
		virtual void SubmitFullscreenQuad(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref< StorageBufferSet> storageBufferSet, Ref<Material> material) = 0;
		virtual void ClearImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Image2D> image) = 0;
		virtual void CopyImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Image2D> sourceImage, Ref<Image2D> destinationImage) = 0;
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBuffer, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform, uint32_t indexCount = 0) = 0;
		virtual void DispatchComputeShader(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<PipelineCompute> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Material> material, const glm::uvec3& workGroups, Buffer additionalUniforms = {}) = 0;

		virtual RendererCapabilities& GetCapabilities() = 0;

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
		static void SetAPI(RendererAPIType api);
	private:
		inline static RendererAPIType s_CurrentRendererAPI = RendererAPIType::Vulkan;
	};
}
