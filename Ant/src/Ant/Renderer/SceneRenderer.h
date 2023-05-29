#pragma once

#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Components.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "ShaderDefs.h"

#include "UniformBufferSet.h"
#include "RenderCommandBuffer.h"
#include "PipelineCompute.h"

#include "StorageBufferSet.h"

#include "Ant/Project/TieringSettings.h"

#include "DebugRenderer.h"

namespace Ant{

	struct SceneRendererOptions
	{
		bool ShowGrid = true;
		bool ShowSelectedInWireframe = false;

		enum class PhysicsColliderView
		{
			SelectedEntity = 0, All = 1
		};

		bool ShowPhysicsColliders = false;
		PhysicsColliderView PhysicsColliderMode = PhysicsColliderView::SelectedEntity;
		bool ShowPhysicsCollidersOnTop = false;
		glm::vec4 SimplePhysicsCollidersColor = glm::vec4{ 0.2f, 1.0f, 0.2f, 1.0f };
		glm::vec4 ComplexPhysicsCollidersColor = glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f };

		// General AO
		float AOShadowTolerance = 0.15f;

		// HBAO
		bool EnableHBAO = false;
		float HBAOIntensity = 1.5f;
		float HBAORadius = 1.0f;
		float HBAOBias = 0.35f;
		float HBAOBlurSharpness = 1.0f;

		// GTAO
		bool EnableGTAO = true;
		bool GTAOBentNormals = false;
		int GTAODenoisePasses = 4;

		// SSR
		bool EnableSSR = false;
		ShaderDef::AOMethod ReflectionOcclusionMethod = ShaderDef::AOMethod::None;
	};

	struct SSROptionsUB
	{
		//SSR
		glm::vec2 HZBUvFactor;
		glm::vec2 FadeIn = { 0.1f, 0.15f };
		float Brightness = 0.7f;
		float DepthTolerance = 0.8f;
		float FacingReflectionsFading = 0.1f;
		int MaxSteps = 70;
		uint32_t NumDepthMips;
		float RoughnessDepthTolerance = 1.0f;
		bool HalfRes = true;
		char Padding[3]{ 0, 0, 0 };
		bool EnableConeTracing = true;
		char Padding1[3]{ 0, 0, 0 };
		float LuminanceFactor = 1.0f;
	};

	struct SceneRendererCamera
	{
		Ant::Camera Camera;
		glm::mat4 ViewMatrix;
		float Near, Far; //Non-reversed
		float FOV;
	};

	struct BloomSettings
	{
		bool Enabled = true;
		float Threshold = 1.0f;
		float Knee = 0.1f;
		float UpsampleScale = 1.0f;
		float Intensity = 1.0f;
		float DirtIntensity = 1.0f;
	};

	struct DOFSettings
	{
		bool Enabled = false;
		float FocusDistance = 0.0f;
		float BlurSize = 1.0f;
	};

	struct SceneRendererSpecification
	{
		Tiering::Renderer::RendererTieringSettings Tiering;
	};

	class SceneRenderer : public RefCounted
	{
	public:
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t Meshes = 0;
			uint32_t Instances = 0;
			uint32_t SavedDraws = 0;

			float TotalGPUTime = 0.0f;
		};
	public:
		SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification = SceneRendererSpecification());
		virtual ~SceneRenderer();

		void Init();
		void InitMaterials();

		void Shutdown();

		// Should only be called at initialization.
		void InitOptions();

		void SetScene(Ref<Scene> scene);

		void SetViewportSize(uint32_t width, uint32_t height);

		void UpdateHBAOData();
		void UpdateGTAOData();

		void BeginScene(const SceneRendererCamera& camera);
		void EndScene();

		static void InsertGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer, const std::string& label, const glm::vec4& markerColor = {});
		static void BeginGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer, const std::string& label, const glm::vec4& markerColor = {});
		static void EndGPUPerfMarker(Ref<RenderCommandBuffer> renderCommandBuffer);

		void SubmitMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<MaterialTable> materialTable, const glm::mat4& transform = glm::mat4(1.0f), const std::vector<glm::mat4>& boneTransforms = {}, Ref<Material> overrideMaterial = nullptr);
		void SubmitStaticMesh(Ref<StaticMesh> staticMesh, Ref<MaterialTable> materialTable, const glm::mat4& transform = glm::mat4(1.0f), Ref<Material> overrideMaterial = nullptr);

		void SubmitSelectedMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<MaterialTable> materialTable, const glm::mat4& transform = glm::mat4(1.0f), const std::vector<glm::mat4>& boneTransforms = {}, Ref<Material> overrideMaterial = nullptr);
		void SubmitSelectedStaticMesh(Ref<StaticMesh> staticMesh, Ref<MaterialTable> materialTable, const glm::mat4& transform = glm::mat4(1.0f), Ref<Material> overrideMaterial = nullptr);

		void SubmitPhysicsDebugMesh(Ref<Mesh> mesh, uint32_t submeshIndex, const glm::mat4& transform = glm::mat4(1.0f));
		void SubmitPhysicsStaticDebugMesh(Ref<StaticMesh> mesh, const glm::mat4& transform = glm::mat4(1.0f), const bool isPrimitiveCollider = true);

		Ref<Pipeline> GetFinalPipeline();
		Ref<RenderPass> GetFinalRenderPass();
		Ref<RenderPass> GetCompositeRenderPass() { return m_CompositePipeline->GetSpecification().RenderPass; }
		Ref<RenderPass> GetExternalCompositeRenderPass() { return m_ExternalCompositeRenderPass; }
		Ref<Image2D> GetFinalPassImage();

		Ref<Renderer2D> GetRenderer2D() { return m_Renderer2D; }
		Ref<DebugRenderer> GetDebugRenderer() { return m_DebugRenderer; }

		SceneRendererOptions& GetOptions();

		void SetShadowSettings(float nearPlane, float farPlane, float lambda, float scaleShadowToOrigin = 0.0f)
		{
			CascadeNearPlaneOffset = nearPlane;
			CascadeFarPlaneOffset = farPlane;
			CascadeSplitLambda = lambda;
			m_ScaleShadowCascadesToOrigin = scaleShadowToOrigin;
		}

		void SetShadowCascades(float a, float b, float c, float d)
		{
			m_UseManualCascadeSplits = true;
			m_ShadowCascadeSplits[0] = a;
			m_ShadowCascadeSplits[1] = b;
			m_ShadowCascadeSplits[2] = c;
			m_ShadowCascadeSplits[3] = d;
		}

		BloomSettings& GetBloomSettings() { return m_BloomSettings; }
		DOFSettings& GetDOFSettings() { return m_DOFSettings; }

		void SetLineWidth(float width);

		static void WaitForThreads();

		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }

		float GetOpacity() const { return m_Opacity; }
		void SetOpacity(float opacity) { m_Opacity = opacity; }

		const Statistics& GetStatistics() const { return m_Statistics; }
	private:
		void FlushDrawList();

		void PreRender();

		struct MeshKey
		{
			AssetHandle MeshHandle;
			AssetHandle MaterialHandle;
			uint32_t SubmeshIndex;
			bool IsSelected;

			MeshKey(AssetHandle meshHandle, AssetHandle materialHandle, uint32_t submeshIndex, bool isSelected)
				: MeshHandle(meshHandle), MaterialHandle(materialHandle), SubmeshIndex(submeshIndex), IsSelected(isSelected)
			{
			}

			bool operator<(const MeshKey& other) const
			{
				if (MeshHandle < other.MeshHandle)
					return true;

				if (MeshHandle > other.MeshHandle)
					return false;

				if (SubmeshIndex < other.SubmeshIndex)
					return true;

				if (SubmeshIndex > other.SubmeshIndex)
					return false;

				if (MaterialHandle < other.MaterialHandle)
					return true;

				if (MaterialHandle > other.MaterialHandle)
					return false;

				return IsSelected < other.IsSelected;

			}
		};

		void CopyToBoneTransformStorage(const MeshKey& meshKey, const Ref<MeshSource>& meshSource, const std::vector<glm::mat4>& boneTransforms);

		void ClearPass();
		void ClearPass(Ref<RenderPass> renderPass, bool explicitClear = false);
		void DeinterleavingPass();
		void HBAOCompute();
		void GTAOCompute();

		void AOComposite();

		void GTAODenoiseCompute();

		void ReinterleavingPass();
		void HBAOBlurPass();
		void ShadowMapPass();
		void SpotShadowMapPass();
		void PreDepthPass();
		void HZBCompute();
		void PreIntegration();
		void LightCullingPass();
		void GeometryPass();
		void PreConvolutionCompute();
		void JumpFloodPass();

		// Post-processing
		void BloomCompute();
		void SSRCompute();
		void SSRCompositePass();
		void EdgeDetectionPass();
		void CompositePass();

		struct CascadeData
		{
			glm::mat4 ViewProj;
			glm::mat4 View;
			float SplitDepth;
		};
		void CalculateCascades(CascadeData* cascades, const SceneRendererCamera& sceneCamera, const glm::vec3& lightDirection) const;
		void CalculateCascadesManualSplit(CascadeData* cascades, const SceneRendererCamera& sceneCamera, const glm::vec3& lightDirection) const;

		void UpdateStatistics();
	private:
		Ref<Scene> m_Scene;
		SceneRendererSpecification m_Specification;
		Ref<RenderCommandBuffer> m_CommandBuffer;

		Ref<Renderer2D> m_Renderer2D;
		Ref<DebugRenderer> m_DebugRenderer;

		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			// Resources
			Ref<Environment> SceneEnvironment;
			float SkyboxLod = 0.0f;
			float SceneEnvironmentIntensity;
			LightEnvironment SceneLightEnvironment;
			DirLight ActiveLight;
		} m_SceneData;

		struct UBCamera
		{
			// projection with near and far inverted
			glm::mat4 ViewProjection;
			glm::mat4 InverseViewProjection;
			glm::mat4 Projection;
			glm::mat4 InverseProjection;
			glm::mat4 View;
			glm::mat4 InverseView;
			glm::vec2 NDCToViewMul;
			glm::vec2 NDCToViewAdd;
			glm::vec2 DepthUnpackConsts;
			glm::vec2 CameraTanHalfFOV;
		} CameraDataUB;

		struct UBHBAOData
		{
			glm::vec4 PerspectiveInfo;
			glm::vec2 InvQuarterResolution;
			float RadiusToScreen;
			float NegInvR2;

			float NDotVBias;
			float AOMultiplier;
			float PowExponent;
			bool IsOrtho;
			char Padding0[3]{ 0, 0, 0 };

			glm::vec4 Float2Offsets[16];
			glm::vec4 Jitters[16];

			glm::vec3 Padding;
			float ShadowTolerance;
		} HBAODataUB;

		struct CBGTAOData
		{
			glm::vec2 NDCToViewMul_x_PixelSize;
			float EffectRadius = 0.5f;
			float EffectFalloffRange = 0.62f;

			float RadiusMultiplier = 1.46f;
			float FinalValuePower = 2.2f;
			float DenoiseBlurBeta = 1.2f;
			bool HalfRes = false;
			char Padding0[3]{ 0, 0, 0 };

			float SampleDistributionPower = 2.0f;
			float ThinOccluderCompensation = 0.0f;
			float DepthMIPSamplingOffset = 3.3f;
			int NoiseIndex = 0;

			glm::vec2 HZBUVFactor;
			float ShadowTolerance;
			float Padding;
		} GTAODataCB;

		struct UBScreenData
		{
			glm::vec2 InvFullResolution;
			glm::vec2 FullResolution;
			glm::vec2 InvHalfResolution;
			glm::vec2 HalfResolution;
		} ScreenDataUB;

		struct UBShadow
		{
			glm::mat4 ViewProjection[4];
		} ShadowData;

		struct DirLight
		{
			glm::vec3 Direction;
			float ShadowAmount;
			glm::vec3 Radiance;
			float Intensity;
		};

		struct UBPointLights
		{
			uint32_t Count{ 0 };
			glm::vec3 Padding{};
			PointLight PointLights[1024]{};
		} PointLightsUB;

		struct UBSpotLights
		{
			uint32_t Count{ 0 };
			glm::vec3 Padding{};
			SpotLight SpotLights[1000]{};
		} SpotLightUB;

		struct UBSpotShadowData
		{
			glm::mat4 ShadowMatrices[1000]{};
		} SpotShadowDataUB;

		struct UBScene
		{
			DirLight Lights;
			glm::vec3 CameraPosition;
			float EnvironmentMapIntensity = 1.0f;
		} SceneDataUB;

		struct UBRendererData
		{
			glm::vec4 CascadeSplits;
			uint32_t TilesCountX{ 0 };
			bool ShowCascades = false;
			char Padding0[3] = { 0,0,0 }; // Bools are 4-bytes in GLSL
			bool SoftShadows = true;
			char Padding1[3] = { 0,0,0 };
			float Range = 0.5f;
			float MaxShadowDistance = 200.0f;
			float ShadowFade = 1.0f;
			bool CascadeFading = true;
			char Padding2[3] = { 0,0,0 };
			float CascadeTransitionFade = 1.0f;
			bool ShowLightComplexity = false;
			char Padding3[3] = { 0,0,0 };
		} RendererDataUB;

		// GTAO
		Ref<Image2D> m_GTAOOutputImage;
		Ref<Image2D> m_GTAODenoiseImage;
		// Points to m_GTAOOutputImage or m_GTAODenoiseImage!
		Ref<Image2D> m_GTAOFinalImage; //TODO: WeakRef!
		Ref<Image2D> m_GTAOEdgesOutputImage;
		Ref<PipelineCompute> m_GTAOPipeline;
		Ref<Material> m_GTAOMaterial;
		glm::uvec3 m_GTAOWorkGroups{ 1 };
		Ref<PipelineCompute> m_GTAODenoisePipeline;
		Ref<Material> m_GTAODenoiseMaterial[2]; //Ping, Pong
		Ref<Material> m_AOCompositeMaterial;
		glm::uvec3 m_GTAODenoiseWorkGroups{ 1 };
		Ref<Pipeline> m_AOCompositePipeline;
		Ref<RenderPass> m_AOCompositeRenderPass;


		//HBAO
		glm::uvec3 m_HBAOWorkGroups{ 1 };
		Ref<Material> m_DeinterleavingMaterial;
		Ref<Material> m_ReinterleavingMaterial;
		Ref<Material> m_HBAOBlurMaterials[2];
		Ref<Material> m_HBAOMaterial;
		Ref<Pipeline> m_DeinterleavingPipelines[2];
		Ref<Pipeline> m_ReinterleavingPipeline;
		Ref<Pipeline> m_HBAOBlurPipelines[2];

		Ref<PipelineCompute> m_HBAOPipeline;
		Ref<Image2D> m_HBAOOutputImage;

		Ref<Shader> m_CompositeShader;


		// Shadows
		Ref<Pipeline> m_SpotShadowPassPipeline;
		Ref<Pipeline> m_SpotShadowPassAnimPipeline;
		Ref<Material> m_SpotShadowPassMaterial;

		glm::uvec3 m_LightCullingWorkGroups;

		Ref<UniformBufferSet> m_UniformBufferSet;
		Ref<StorageBufferSet> m_StorageBufferSet;

		float LightDistance = 0.1f;
		float CascadeSplitLambda = 0.92f;
		glm::vec4 CascadeSplits;
		float CascadeFarPlaneOffset = 50.0f, CascadeNearPlaneOffset = -50.0f;
		float m_ScaleShadowCascadesToOrigin = 0.0f;
		float m_ShadowCascadeSplits[4];
		bool m_UseManualCascadeSplits = false;

		//SSR
		Ref<Pipeline> m_SSRCompositePipeline;
		Ref<PipelineCompute> m_SSRPipeline;
		Ref<PipelineCompute> m_HierarchicalDepthPipeline;
		Ref<PipelineCompute> m_GaussianBlurPipeline;
		Ref<PipelineCompute> m_PreIntegrationPipeline;
		Ref<PipelineCompute> m_SSRUpscalePipeline;
		Ref<Image2D> m_SSRImage;
		Ref<Texture2D> m_HierarchicalDepthTexture;
		Ref<Texture2D> m_PreConvolutedTexture;
		Ref<Texture2D> m_VisibilityTexture;
		Ref<Material> m_SSRCompositeMaterial;
		Ref<Material> m_SSRMaterial;
		Ref<Material> m_GaussianBlurMaterial;
		Ref<Material> m_HierarchicalDepthMaterial;
		Ref<Material> m_PreIntegrationMaterial;
		glm::uvec3 m_SSRWorkGroups{ 1 };


		glm::vec2 FocusPoint = { 0.5f, 0.5f };

		Ref<Material> m_CompositeMaterial;
		Ref<Material> m_LightCullingMaterial;

		Ref<Pipeline> m_GeometryPipeline;
		Ref<Pipeline> m_TransparentGeometryPipeline;
		Ref<Pipeline> m_GeometryPipelineAnim;

		Ref<Pipeline> m_SelectedGeometryPipeline;
		Ref<Pipeline> m_SelectedGeometryPipelineAnim;
		Ref<Material> m_SelectedGeometryMaterial;
		Ref<Material> m_SelectedGeometryMaterialAnim;

		Ref<Pipeline> m_GeometryWireframePipeline;
		Ref<Pipeline> m_GeometryWireframePipelineAnim;
		Ref<Pipeline> m_GeometryWireframeOnTopPipeline;
		Ref<Pipeline> m_GeometryWireframeOnTopPipelineAnim;
		Ref<Material> m_WireframeMaterial;

		Ref<Pipeline> m_PreDepthPipeline;
		Ref<Pipeline> m_PreDepthTransparentPipeline;
		Ref<Pipeline> m_PreDepthPipelineAnim;
		Ref<Material> m_PreDepthMaterial;

		Ref<Pipeline> m_CompositePipeline;

		Ref<Pipeline> m_ShadowPassPipelines[4];
		Ref<Pipeline> m_ShadowPassPipelinesAnim[4];

		Ref<Pipeline> m_EdgeDetectionPipeline;
		Ref<Material> m_EdgeDetectionMaterial;

		Ref<Material> m_ShadowPassMaterial;

		Ref<Pipeline> m_SkyboxPipeline;
		Ref<Material> m_SkyboxMaterial;

		Ref<Pipeline> m_DOFPipeline;
		Ref<Material> m_DOFMaterial;

		Ref<PipelineCompute> m_LightCullingPipeline;

		// Jump Flood Pass
		Ref<Pipeline> m_JumpFloodInitPipeline;
		Ref<Pipeline> m_JumpFloodPassPipeline[2];
		Ref<Pipeline> m_JumpFloodCompositePipeline;
		Ref<Material> m_JumpFloodInitMaterial, m_JumpFloodPassMaterial[2];
		Ref<Material> m_JumpFloodCompositeMaterial;

		// Bloom compute
		uint32_t m_BloomComputeWorkgroupSize = 4;
		Ref<PipelineCompute> m_BloomComputePipeline;
		std::vector<Ref<Texture2D>> m_BloomComputeTextures{ 3 };
		Ref<Material> m_BloomComputeMaterial;

		struct TransformVertexData
		{
			glm::vec4 MRow[3];
		};

		struct TransformBuffer
		{
			Ref<VertexBuffer> Buffer;
			TransformVertexData* Data = nullptr;
		};

		std::vector<TransformBuffer> m_SubmeshTransformBuffers;
		using BoneTransforms = std::array<glm::mat4, 100>; // Note: 100 == MAX_BONES from the shaders
		std::vector<Ref<StorageBuffer>> m_BoneTransformStorageBuffers;
		BoneTransforms* m_BoneTransformsData = nullptr;

		std::vector<Ref<Framebuffer>> m_TempFramebuffers;

		Ref<RenderPass> m_ExternalCompositeRenderPass;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			uint32_t SubmeshIndex;
			Ref<MaterialTable> MaterialTable;
			Ref<Material> OverrideMaterial;

			uint32_t InstanceCount = 0;
			uint32_t InstanceOffset = 0;
			bool IsRigged = false;
		};

		struct StaticDrawCommand
		{
			Ref<StaticMesh> StaticMesh;
			uint32_t SubmeshIndex;
			Ref<MaterialTable> MaterialTable;
			Ref<Material> OverrideMaterial;

			uint32_t InstanceCount = 0;
			uint32_t InstanceOffset = 0;
		};

		struct TransformMapData
		{
			std::vector<TransformVertexData> Transforms;
			uint32_t TransformOffset = 0;
		};

		struct BoneTransformsMapData
		{
			std::vector<BoneTransforms> BoneTransformsData;
			uint32_t BoneTransformsBaseIndex = 0;
		};

		std::map<MeshKey, TransformMapData> m_MeshTransformMap;
		std::map<MeshKey, BoneTransformsMapData> m_MeshBoneTransformsMap;

		//std::vector<DrawCommand> m_DrawList;
		std::map<MeshKey, DrawCommand> m_DrawList;
		std::map<MeshKey, DrawCommand> m_TransparentDrawList;
		std::map<MeshKey, DrawCommand> m_SelectedMeshDrawList;
		std::map<MeshKey, DrawCommand> m_ShadowPassDrawList;

		std::map<MeshKey, StaticDrawCommand> m_StaticMeshDrawList;
		std::map<MeshKey, StaticDrawCommand> m_TransparentStaticMeshDrawList;
		std::map<MeshKey, StaticDrawCommand> m_SelectedStaticMeshDrawList;
		std::map<MeshKey, StaticDrawCommand> m_StaticMeshShadowPassDrawList;

		// Debug
		std::map<MeshKey, StaticDrawCommand> m_StaticColliderDrawList;
		std::map<MeshKey, DrawCommand> m_ColliderDrawList;

		// Grid
		Ref<Pipeline> m_GridPipeline;
		Ref<Material> m_GridMaterial;

		Ref<Material> m_OutlineMaterial;
		Ref<Material> m_SimpleColliderMaterial;
		Ref<Material> m_ComplexColliderMaterial;

		SceneRendererOptions m_Options;
		SSROptionsUB m_SSROptions;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		float m_InvViewportWidth = 0.f, m_InvViewportHeight = 0.f;
		bool m_NeedsResize = false;
		bool m_Active = false;
		bool m_ResourcesCreatedGPU = false;
		bool m_ResourcesCreated = false;

		float m_LineWidth = 2.0f;

		BloomSettings m_BloomSettings;
		DOFSettings m_DOFSettings;
		Ref<Texture2D> m_BloomDirtTexture;

		Ref<Image2D> m_ReadBackImage;
		glm::vec4* m_ReadBackBuffer = nullptr;

		float m_Opacity = 1.0f;

		struct GPUTimeQueries
		{
			uint32_t DirShadowMapPassQuery = 0;
			uint32_t SpotShadowMapPassQuery = 0;
			uint32_t DepthPrePassQuery = 0;
			uint32_t HierarchicalDepthQuery = 0;
			uint32_t PreIntegrationQuery = 0;
			uint32_t LightCullingPassQuery = 0;
			uint32_t GeometryPassQuery = 0;
			uint32_t PreConvolutionQuery = 0;
			uint32_t HBAOPassQuery = 0;
			uint32_t GTAOPassQuery = 0;
			uint32_t GTAODenoisePassQuery = 0;
			uint32_t AOCompositePassQuery = 0;
			uint32_t SSRQuery = 0;
			uint32_t SSRCompositeQuery = 0;
			uint32_t BloomComputePassQuery = 0;
			uint32_t JumpFloodPassQuery = 0;
			uint32_t CompositePassQuery = 0;
		} m_GPUTimeQueries;

		Statistics m_Statistics;

		friend class SceneRendererPanel;
	};

}